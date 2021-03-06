// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/resource_coordinator/memory_instrumentation/graph_processor.h"

#include "base/memory/shared_memory_tracker.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace memory_instrumentation {

using base::ProcessId;
using base::trace_event::HeapProfilerSerializationState;
using base::trace_event::MemoryAllocatorDump;
using base::trace_event::MemoryAllocatorDumpGuid;
using base::trace_event::MemoryDumpArgs;
using base::trace_event::MemoryDumpLevelOfDetail;
using base::trace_event::ProcessMemoryDump;
using Edge = GlobalDumpGraph::Edge;
using Node = GlobalDumpGraph::Node;
using Process = GlobalDumpGraph::Process;

namespace {

const MemoryAllocatorDumpGuid kEmptyGuid;

}  // namespace

class GraphProcessorTest : public testing::Test {
 public:
  GraphProcessorTest() {}

  void MarkImplicitWeakParentsRecursively(Node* node) {
    GraphProcessor::MarkImplicitWeakParentsRecursively(node);
  }

  void MarkWeakOwnersAndChildrenRecursively(Node* node) {
    std::set<const Node*> visited;
    GraphProcessor::MarkWeakOwnersAndChildrenRecursively(node, &visited);
  }

  void RemoveWeakNodesRecursively(Node* node) {
    GraphProcessor::RemoveWeakNodesRecursively(node);
  }

  void AssignTracingOverhead(base::StringPiece allocator,
                             GlobalDumpGraph* global_graph,
                             GlobalDumpGraph::Process* process) {
    GraphProcessor::AssignTracingOverhead(allocator, global_graph, process);
  }

  GlobalDumpGraph::Node::Entry AggregateNumericWithNameForNode(
      GlobalDumpGraph::Node* node,
      base::StringPiece name) {
    return GraphProcessor::AggregateNumericWithNameForNode(node, name);
  }

  void AggregateNumericsRecursively(GlobalDumpGraph::Node* node) {
    GraphProcessor::AggregateNumericsRecursively(node);
  }

  void PropagateNumericsAndDiagnosticsRecursively(GlobalDumpGraph::Node* node) {
    GraphProcessor::PropagateNumericsAndDiagnosticsRecursively(node);
  }

  base::Optional<uint64_t> AggregateSizeForDescendantNode(Node* root,
                                                          Node* descendant) {
    return GraphProcessor::AggregateSizeForDescendantNode(root, descendant);
  }

  void CalculateSizeForNode(GlobalDumpGraph::Node* node) {
    GraphProcessor::CalculateSizeForNode(node);
  }

  void CalculateDumpSubSizes(Node* node) {
    GraphProcessor::CalculateDumpSubSizes(node);
  }

 protected:
  GlobalDumpGraph graph;
};

TEST_F(GraphProcessorTest, SmokeComputeMemoryGraph) {
  std::map<ProcessId, const ProcessMemoryDump*> process_dumps;

  MemoryDumpArgs dump_args = {MemoryDumpLevelOfDetail::DETAILED};
  ProcessMemoryDump pmd(new HeapProfilerSerializationState, dump_args);

  auto* source = pmd.CreateAllocatorDump("test1/test2/test3");
  source->AddScalar(MemoryAllocatorDump::kNameSize,
                    MemoryAllocatorDump::kUnitsBytes, 10);

  auto* target = pmd.CreateAllocatorDump("target");
  pmd.AddOwnershipEdge(source->guid(), target->guid(), 10);

  auto* weak =
      pmd.CreateWeakSharedGlobalAllocatorDump(MemoryAllocatorDumpGuid(1));

  process_dumps.emplace(1, &pmd);

  auto global_dump = GraphProcessor::CreateMemoryGraph(process_dumps);

  ASSERT_EQ(1u, global_dump->process_dump_graphs().size());

  auto id_to_dump_it = global_dump->process_dump_graphs().find(1);
  auto* first_child = id_to_dump_it->second->FindNode("test1");
  ASSERT_NE(first_child, nullptr);
  ASSERT_EQ(first_child->parent(), id_to_dump_it->second->root());

  auto* second_child = first_child->GetChild("test2");
  ASSERT_NE(second_child, nullptr);
  ASSERT_EQ(second_child->parent(), first_child);

  auto* third_child = second_child->GetChild("test3");
  ASSERT_NE(third_child, nullptr);
  ASSERT_EQ(third_child->parent(), second_child);

  auto* direct = id_to_dump_it->second->FindNode("test1/test2/test3");
  ASSERT_EQ(third_child, direct);

  ASSERT_EQ(third_child->entries()->size(), 1ul);

  auto size = third_child->entries()->find(MemoryAllocatorDump::kNameSize);
  ASSERT_EQ(10ul, size->second.value_uint64);

  ASSERT_TRUE(weak->flags() & MemoryAllocatorDump::Flags::WEAK);

  auto& edges = global_dump->edges();
  auto edge_it = edges.begin();
  ASSERT_EQ(std::distance(edges.begin(), edges.end()), 1l);
  ASSERT_EQ(edge_it->source(), direct);
  ASSERT_EQ(edge_it->target(), id_to_dump_it->second->FindNode("target"));
  ASSERT_EQ(edge_it->priority(), 10);
}

TEST_F(GraphProcessorTest, SmokeComputeSharedFootprint) {
  std::map<ProcessId, const ProcessMemoryDump*> process_dumps;

  MemoryDumpArgs dump_args = {MemoryDumpLevelOfDetail::DETAILED};
  ProcessMemoryDump pmd1(new HeapProfilerSerializationState, dump_args);
  ProcessMemoryDump pmd2(new HeapProfilerSerializationState, dump_args);

  auto* p1_d1 = pmd1.CreateAllocatorDump("process1/dump1");
  auto* p1_d2 = pmd1.CreateAllocatorDump("process1/dump2");
  auto* p2_d1 = pmd2.CreateAllocatorDump("process2/dump1");

  base::UnguessableToken token = base::UnguessableToken::Create();

  // Done by SharedMemoryTracker.
  size_t size = 256;
  auto global_dump_guid =
      base::SharedMemoryTracker::GetGlobalDumpIdForTracing(token);
  auto local_dump_name =
      base::SharedMemoryTracker::GetDumpNameForTracing(token);

  MemoryAllocatorDump* local_dump_1 =
      pmd1.CreateAllocatorDump(local_dump_name, MemoryAllocatorDumpGuid(1));
  local_dump_1->AddScalar("virtual_size", MemoryAllocatorDump::kUnitsBytes,
                          size);
  local_dump_1->AddScalar(MemoryAllocatorDump::kNameSize,
                          MemoryAllocatorDump::kUnitsBytes, size);

  MemoryAllocatorDump* global_dump_1 =
      pmd1.CreateSharedGlobalAllocatorDump(global_dump_guid);
  global_dump_1->AddScalar(MemoryAllocatorDump::kNameSize,
                           MemoryAllocatorDump::kUnitsBytes, size);

  pmd1.AddOverridableOwnershipEdge(local_dump_1->guid(), global_dump_1->guid(),
                                   0 /* importance */);

  MemoryAllocatorDump* local_dump_2 =
      pmd2.CreateAllocatorDump(local_dump_name, MemoryAllocatorDumpGuid(2));
  local_dump_2->AddScalar("virtual_size", MemoryAllocatorDump::kUnitsBytes,
                          size);
  local_dump_2->AddScalar(MemoryAllocatorDump::kNameSize,
                          MemoryAllocatorDump::kUnitsBytes, size);

  MemoryAllocatorDump* global_dump_2 =
      pmd2.CreateSharedGlobalAllocatorDump(global_dump_guid);
  pmd2.AddOverridableOwnershipEdge(local_dump_2->guid(), global_dump_2->guid(),
                                   0 /* importance */);

  // Done by each consumer of the shared memory.
  pmd1.CreateSharedMemoryOwnershipEdge(p1_d1->guid(), token, 2);
  pmd1.CreateSharedMemoryOwnershipEdge(p1_d2->guid(), token, 1);
  pmd2.CreateSharedMemoryOwnershipEdge(p2_d1->guid(), token, 2);

  process_dumps.emplace(1, &pmd1);
  process_dumps.emplace(2, &pmd2);

  auto graph = GraphProcessor::CreateMemoryGraph(process_dumps);
  auto pid_to_sizes = GraphProcessor::ComputeSharedFootprintFromGraph(*graph);
  ASSERT_EQ(pid_to_sizes[1], 128ul);
  ASSERT_EQ(pid_to_sizes[2], 128ul);
}

TEST_F(GraphProcessorTest, ComputeSharedFootprintFromGraphSameImportance) {
  Process* global_process = graph.shared_memory_graph();
  Node* global_node = global_process->CreateNode(kEmptyGuid, "global/1", false);
  global_node->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 100);

  Process* first = graph.CreateGraphForProcess(1);
  Node* shared_1 = first->CreateNode(kEmptyGuid, "shared_memory/1", false);

  Process* second = graph.CreateGraphForProcess(2);
  Node* shared_2 = second->CreateNode(kEmptyGuid, "shared_memory/2", false);

  graph.AddNodeOwnershipEdge(shared_1, global_node, 1);
  graph.AddNodeOwnershipEdge(shared_2, global_node, 1);

  auto pid_to_sizes = GraphProcessor::ComputeSharedFootprintFromGraph(graph);
  ASSERT_EQ(pid_to_sizes[1], 50ul);
  ASSERT_EQ(pid_to_sizes[2], 50ul);
}

TEST_F(GraphProcessorTest, ComputeSharedFootprintFromGraphSomeDiffImportance) {
  Process* global_process = graph.shared_memory_graph();

  Node* global_node = global_process->CreateNode(kEmptyGuid, "global/1", false);
  global_node->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 100);

  Process* first = graph.CreateGraphForProcess(1);
  Node* shared_1 = first->CreateNode(kEmptyGuid, "shared_memory/1", false);

  Process* second = graph.CreateGraphForProcess(2);
  Node* shared_2 = second->CreateNode(kEmptyGuid, "shared_memory/2", false);

  Process* third = graph.CreateGraphForProcess(3);
  Node* shared_3 = third->CreateNode(kEmptyGuid, "shared_memory/3", false);

  Process* fourth = graph.CreateGraphForProcess(4);
  Node* shared_4 = fourth->CreateNode(kEmptyGuid, "shared_memory/4", false);

  Process* fifth = graph.CreateGraphForProcess(5);
  Node* shared_5 = fifth->CreateNode(kEmptyGuid, "shared_memory/5", false);

  graph.AddNodeOwnershipEdge(shared_1, global_node, 1);
  graph.AddNodeOwnershipEdge(shared_2, global_node, 2);
  graph.AddNodeOwnershipEdge(shared_3, global_node, 3);
  graph.AddNodeOwnershipEdge(shared_4, global_node, 3);
  graph.AddNodeOwnershipEdge(shared_5, global_node, 3);

  auto pid_to_sizes = GraphProcessor::ComputeSharedFootprintFromGraph(graph);
  ASSERT_EQ(pid_to_sizes[1], 0ul);
  ASSERT_EQ(pid_to_sizes[2], 0ul);
  ASSERT_EQ(pid_to_sizes[3], 33ul);
  ASSERT_EQ(pid_to_sizes[4], 33ul);
  ASSERT_EQ(pid_to_sizes[5], 33ul);
}

TEST_F(GraphProcessorTest, MarkWeakParentsSimple) {
  Process* process = graph.CreateGraphForProcess(1);
  Node* parent = process->CreateNode(kEmptyGuid, "parent", false);
  Node* first = process->CreateNode(kEmptyGuid, "parent/first", true);
  Node* second = process->CreateNode(kEmptyGuid, "parent/second", false);

  // Case where one child is not weak.
  parent->set_explicit(false);
  first->set_explicit(true);
  second->set_explicit(true);

  // The function should be a no-op.
  MarkImplicitWeakParentsRecursively(parent);
  ASSERT_FALSE(parent->is_weak());
  ASSERT_TRUE(first->is_weak());
  ASSERT_FALSE(second->is_weak());

  // Case where all children is weak.
  second->set_weak(true);

  // The function should mark parent as weak.
  MarkImplicitWeakParentsRecursively(parent);
  ASSERT_TRUE(parent->is_weak());
  ASSERT_TRUE(first->is_weak());
  ASSERT_TRUE(second->is_weak());
}

TEST_F(GraphProcessorTest, MarkWeakParentsComplex) {
  Process* process = graph.CreateGraphForProcess(1);

  // |first| is explicitly strong but |first_child| is implicitly so.
  Node* parent = process->CreateNode(kEmptyGuid, "parent", false);
  Node* first = process->CreateNode(kEmptyGuid, "parent/f", false);
  Node* first_child = process->CreateNode(kEmptyGuid, "parent/f/c", false);
  Node* first_gchild = process->CreateNode(kEmptyGuid, "parent/f/c/c", true);

  parent->set_explicit(false);
  first->set_explicit(true);
  first_child->set_explicit(false);
  first_gchild->set_explicit(true);

  // That should lead to |first_child| marked implicitly weak.
  MarkImplicitWeakParentsRecursively(parent);
  ASSERT_FALSE(parent->is_weak());
  ASSERT_FALSE(first->is_weak());
  ASSERT_TRUE(first_child->is_weak());
  ASSERT_TRUE(first_gchild->is_weak());

  // Reset and change so that first is now only implicitly strong.
  first->set_explicit(false);
  first_child->set_weak(false);

  // The whole chain should now be weak.
  MarkImplicitWeakParentsRecursively(parent);
  ASSERT_TRUE(parent->is_weak());
  ASSERT_TRUE(first->is_weak());
  ASSERT_TRUE(first_child->is_weak());
  ASSERT_TRUE(first_gchild->is_weak());
}

TEST_F(GraphProcessorTest, MarkWeakOwners) {
  Process* process = graph.CreateGraphForProcess(1);

  // Make only the ultimate owned node weak.
  Node* owner = process->CreateNode(kEmptyGuid, "owner", false);
  Node* owned = process->CreateNode(kEmptyGuid, "owned", false);
  Node* owned_2 = process->CreateNode(kEmptyGuid, "owned2", true);

  graph.AddNodeOwnershipEdge(owner, owned, 0);
  graph.AddNodeOwnershipEdge(owned, owned_2, 0);

  // Starting from leaf node should lead to everything being weak.
  MarkWeakOwnersAndChildrenRecursively(process->root());
  ASSERT_TRUE(owner->is_weak());
  ASSERT_TRUE(owned->is_weak());
  ASSERT_TRUE(owned_2->is_weak());
}

TEST_F(GraphProcessorTest, MarkWeakParent) {
  Process* process = graph.CreateGraphForProcess(1);
  Node* parent = process->CreateNode(kEmptyGuid, "parent", true);
  Node* child = process->CreateNode(kEmptyGuid, "parent/c", false);
  Node* child_2 = process->CreateNode(kEmptyGuid, "parent/c/c", false);

  // Starting from parent node should lead to everything being weak.
  MarkWeakOwnersAndChildrenRecursively(process->root());
  ASSERT_TRUE(parent->is_weak());
  ASSERT_TRUE(child->is_weak());
  ASSERT_TRUE(child_2->is_weak());
}

TEST_F(GraphProcessorTest, MarkWeakParentOwner) {
  Process* process = graph.CreateGraphForProcess(1);

  // Make only the parent node weak.
  Node* parent = process->CreateNode(kEmptyGuid, "parent", true);
  Node* child = process->CreateNode(kEmptyGuid, "parent/c", false);
  Node* child_2 = process->CreateNode(kEmptyGuid, "parent/c/c", false);
  Node* owner = process->CreateNode(kEmptyGuid, "owner", false);

  graph.AddNodeOwnershipEdge(owner, parent, 0);

  // Starting from parent node should lead to everything being weak.
  MarkWeakOwnersAndChildrenRecursively(process->root());
  ASSERT_TRUE(parent->is_weak());
  ASSERT_TRUE(child->is_weak());
  ASSERT_TRUE(child_2->is_weak());
  ASSERT_TRUE(owner->is_weak());
}

TEST_F(GraphProcessorTest, RemoveWeakNodesRecursively) {
  Process* process = graph.CreateGraphForProcess(1);

  // Make only the child node weak.
  Node* parent = process->CreateNode(kEmptyGuid, "parent", false);
  Node* child = process->CreateNode(kEmptyGuid, "parent/c", true);
  process->CreateNode(kEmptyGuid, "parent/c/c", false);
  Node* owned = process->CreateNode(kEmptyGuid, "parent/owned", false);

  graph.AddNodeOwnershipEdge(child, owned, 0);

  // Starting from parent node should lead child and child_2 being
  // removed and owned to have the edge from it removed.
  RemoveWeakNodesRecursively(parent);

  ASSERT_EQ(parent->children()->size(), 1ul);
  ASSERT_EQ(parent->children()->begin()->second, owned);

  ASSERT_TRUE(owned->owned_by_edges()->empty());
}

TEST_F(GraphProcessorTest, RemoveWeakNodesRecursivelyBetweenGraphs) {
  Process* f_process = graph.CreateGraphForProcess(1);
  Process* s_process = graph.CreateGraphForProcess(2);

  // Make only the child node weak.
  Node* child = f_process->CreateNode(kEmptyGuid, "c", true);
  f_process->CreateNode(kEmptyGuid, "c/c", false);
  Node* owned = s_process->CreateNode(kEmptyGuid, "owned", false);

  graph.AddNodeOwnershipEdge(child, owned, 0);

  // Starting from root node should lead child and child_2 being
  // removed.
  RemoveWeakNodesRecursively(f_process->root());

  ASSERT_EQ(f_process->root()->children()->size(), 0ul);
  ASSERT_EQ(s_process->root()->children()->size(), 1ul);

  // This should be false until our next pass.
  ASSERT_FALSE(owned->owned_by_edges()->empty());

  RemoveWeakNodesRecursively(s_process->root());

  // We should now have cleaned up the owned node's edges.
  ASSERT_TRUE(owned->owned_by_edges()->empty());
}

TEST_F(GraphProcessorTest, AssignTracingOverhead) {
  Process* process = graph.CreateGraphForProcess(1);

  // Now add an allocator node.
  process->CreateNode(kEmptyGuid, "malloc", false);

  // If the tracing node does not exist, this should do nothing.
  AssignTracingOverhead("malloc", &graph, process);
  ASSERT_TRUE(process->root()->GetChild("malloc")->children()->empty());

  // Now add a tracing node.
  process->CreateNode(kEmptyGuid, "tracing", false);

  // This should now add a node with the allocator.
  AssignTracingOverhead("malloc", &graph, process);
  ASSERT_NE(process->FindNode("malloc/allocated_objects/tracing_overhead"),
            nullptr);
}

TEST_F(GraphProcessorTest, AggregateNumericWithNameForNode) {
  Process* process = graph.CreateGraphForProcess(1);

  Node* c1 = process->CreateNode(kEmptyGuid, "c1", false);
  Node* c2 = process->CreateNode(kEmptyGuid, "c2", false);
  Node* c3 = process->CreateNode(kEmptyGuid, "c3", false);

  c1->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 100);
  c2->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 256);
  c3->AddEntry("other_numeric", Node::Entry::ScalarUnits::kBytes, 1000);

  Node* root = process->root();
  Node::Entry entry = AggregateNumericWithNameForNode(root, "random_numeric");
  ASSERT_EQ(entry.value_uint64, 356ul);
  ASSERT_EQ(entry.units, Node::Entry::ScalarUnits::kBytes);
}

TEST_F(GraphProcessorTest, AggregateNumericsRecursively) {
  Process* process = graph.CreateGraphForProcess(1);

  Node* c1 = process->CreateNode(kEmptyGuid, "c1", false);
  Node* c2 = process->CreateNode(kEmptyGuid, "c2", false);
  Node* c2_c1 = process->CreateNode(kEmptyGuid, "c2/c1", false);
  Node* c2_c2 = process->CreateNode(kEmptyGuid, "c2/c2", false);
  Node* c3_c1 = process->CreateNode(kEmptyGuid, "c3/c1", false);
  Node* c3_c2 = process->CreateNode(kEmptyGuid, "c3/c2", false);

  // If an entry already exists in the parent, the child should not
  // ovewrite it. If nothing exists, then the child can aggregrate.
  c1->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 100);
  c2->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 256);
  c2_c1->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 256);
  c2_c2->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 256);
  c3_c1->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 10);
  c3_c2->AddEntry("random_numeric", Node::Entry::ScalarUnits::kBytes, 10);

  Node* root = process->root();
  AggregateNumericsRecursively(root);
  ASSERT_EQ(root->entries()->size(), 1ul);

  auto entry = root->entries()->begin()->second;
  ASSERT_EQ(entry.value_uint64, 376ul);
  ASSERT_EQ(entry.units, Node::Entry::ScalarUnits::kBytes);
}

TEST_F(GraphProcessorTest, AggregateSizeForDescendantNode) {
  Process* process = graph.CreateGraphForProcess(1);

  Node* c1 = process->CreateNode(kEmptyGuid, "c1", false);
  Node* c2 = process->CreateNode(kEmptyGuid, "c2", false);
  Node* c2_c1 = process->CreateNode(kEmptyGuid, "c2/c1", false);
  Node* c2_c2 = process->CreateNode(kEmptyGuid, "c2/c2", false);
  Node* c3_c1 = process->CreateNode(kEmptyGuid, "c3/c1", false);
  Node* c3_c2 = process->CreateNode(kEmptyGuid, "c3/c2", false);

  c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 100);
  c2_c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 256);
  c2_c2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 256);
  c3_c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 10);
  c3_c2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 10);

  graph.AddNodeOwnershipEdge(c2_c2, c3_c2, 0);

  // Aggregating root should give size of (100 + 256 + 10 * 2) = 376.
  // |c2_c2| is not counted because it is owns by |c3_c2|.
  Node* root = process->root();
  ASSERT_EQ(376ul, *AggregateSizeForDescendantNode(root, root));

  // Aggregating c2 should give size of (256 * 2) = 512. |c2_c2| is counted
  // because |c3_c2| is not a child of |c2|.
  ASSERT_EQ(512ul, *AggregateSizeForDescendantNode(c2, c2));
}

TEST_F(GraphProcessorTest, CalculateSizeForNode) {
  Process* process = graph.CreateGraphForProcess(1);

  Node* c1 = process->CreateNode(kEmptyGuid, "c1", false);
  Node* c2 = process->CreateNode(kEmptyGuid, "c2", false);
  Node* c2_c1 = process->CreateNode(kEmptyGuid, "c2/c1", false);
  Node* c2_c2 = process->CreateNode(kEmptyGuid, "c2/c2", false);
  Node* c3 = process->CreateNode(kEmptyGuid, "c3", false);
  Node* c3_c1 = process->CreateNode(kEmptyGuid, "c3/c1", false);
  Node* c3_c2 = process->CreateNode(kEmptyGuid, "c3/c2", false);

  c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 600);
  c2_c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 10);
  c2_c2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 10);
  c3->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 600);
  c3_c1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 256);
  c3_c2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 256);

  graph.AddNodeOwnershipEdge(c2_c2, c3_c2, 0);

  // Compute size entry for |c2| since computations for |c2_c1| and |c2_c2|
  // are already complete.
  CalculateSizeForNode(c2);

  // Check that |c2| now has a size entry of 20 (sum of children).
  auto c2_entry = c2->entries()->begin()->second;
  ASSERT_EQ(c2_entry.value_uint64, 20ul);
  ASSERT_EQ(c2_entry.units, Node::Entry::ScalarUnits::kBytes);

  // Compute size entry for |c3_c2| which should not change in size.
  CalculateSizeForNode(c3_c2);

  // Check that |c3_c2| now has unchanged size.
  auto c3_c2_entry = c3_c2->entries()->begin()->second;
  ASSERT_EQ(c3_c2_entry.value_uint64, 256ul);
  ASSERT_EQ(c3_c2_entry.units, Node::Entry::ScalarUnits::kBytes);

  // Compute size entry for |c3| which should add an unspecified node.
  CalculateSizeForNode(c3);

  // Check that |c3| has unchanged size.
  auto c3_entry = c3->entries()->begin()->second;
  ASSERT_EQ(c3_entry.value_uint64, 600ul);
  ASSERT_EQ(c3_entry.units, Node::Entry::ScalarUnits::kBytes);

  // Check that the unspecified node is a child of |c3| and has size
  // 600 - 512 = 88.
  Node* c3_child = c3->children()->find("<unspecified>")->second;
  auto c3_child_entry = c3_child->entries()->begin()->second;
  ASSERT_EQ(c3_child_entry.value_uint64, 88ul);
  ASSERT_EQ(c3_child_entry.units, Node::Entry::ScalarUnits::kBytes);

  // Compute size entry for |root| which should aggregate children sizes.
  CalculateSizeForNode(process->root());

  // Check that |root| has been assigned a size of 600 + 10 + 600 = 1210.
  // Note that |c2_c2| is not counted because it ows |c3_c2| which is a
  // descendant of |root|.
  auto root_entry = process->root()->entries()->begin()->second;
  ASSERT_EQ(root_entry.value_uint64, 1210ul);
  ASSERT_EQ(root_entry.units, Node::Entry::ScalarUnits::kBytes);
}

TEST_F(GraphProcessorTest, CalculateDumpSubSizes) {
  Process* process_1 = graph.CreateGraphForProcess(1);
  Process* process_2 = graph.CreateGraphForProcess(2);

  Node* parent_1 = process_1->CreateNode(kEmptyGuid, "parent", false);
  Node* child_1 = process_1->CreateNode(kEmptyGuid, "parent/child", false);

  Node* parent_2 = process_2->CreateNode(kEmptyGuid, "parent", false);
  Node* child_2 = process_2->CreateNode(kEmptyGuid, "parent/child", false);

  graph.AddNodeOwnershipEdge(parent_1, parent_2, 0);

  process_1->root()->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 4);
  parent_1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 4);
  child_1->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 4);
  process_2->root()->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 5);
  parent_2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 5);
  child_2->AddEntry("size", Node::Entry::ScalarUnits::kBytes, 5);

  // Each of these nodes should have owner/owned same as size itself.
  CalculateDumpSubSizes(child_1);
  ASSERT_EQ(child_1->not_owned_sub_size(), 4ul);
  ASSERT_EQ(child_1->not_owning_sub_size(), 4ul);
  CalculateDumpSubSizes(child_2);
  ASSERT_EQ(child_2->not_owned_sub_size(), 5ul);
  ASSERT_EQ(child_2->not_owning_sub_size(), 5ul);

  // These nodes should also have size of children.
  CalculateDumpSubSizes(parent_1);
  ASSERT_EQ(parent_1->not_owned_sub_size(), 4ul);
  ASSERT_EQ(parent_1->not_owning_sub_size(), 4ul);
  CalculateDumpSubSizes(parent_2);
  ASSERT_EQ(parent_2->not_owned_sub_size(), 5ul);
  ASSERT_EQ(parent_2->not_owning_sub_size(), 5ul);

  // These nodes should account for edge between parents.
  CalculateDumpSubSizes(process_1->root());
  ASSERT_EQ(process_1->root()->not_owned_sub_size(), 4ul);
  ASSERT_EQ(process_1->root()->not_owning_sub_size(), 0ul);
  CalculateDumpSubSizes(process_2->root());
  ASSERT_EQ(process_2->root()->not_owned_sub_size(), 1ul);
  ASSERT_EQ(process_2->root()->not_owning_sub_size(), 5ul);
}

}  // namespace memory_instrumentation