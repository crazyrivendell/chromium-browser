// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_break.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::BeforeTarget, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::OverflowTarget, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::OverflowLeader, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::OverflowTrailer, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::StartNew, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::BookendTrailer, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::After, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Auto},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::BookendLeader, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::AfterTarget, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Before, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Auto},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"break";

}  // namespace

CXFA_Break::CXFA_Break(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Break,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Break::~CXFA_Break() {}
