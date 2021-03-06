// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_event.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Execute, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Script, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::SignData, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Extras, 1, 0},
    {XFA_Element::Submit, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Ref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Listen, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::RefOnly},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Activity, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Click},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"event";

}  // namespace

CXFA_Event::CXFA_Event(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Event,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Event::~CXFA_Event() {}
