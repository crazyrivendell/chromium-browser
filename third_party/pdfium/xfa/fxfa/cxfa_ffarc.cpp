// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffarc.h"

CXFA_FFArc::CXFA_FFArc(CXFA_WidgetAcc* pDataAcc) : CXFA_FFDraw(pDataAcc) {}

CXFA_FFArc::~CXFA_FFArc() {}

void CXFA_FFArc::RenderWidget(CXFA_Graphics* pGS,
                              const CFX_Matrix& matrix,
                              uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CXFA_ValueData valueData = m_pDataAcc->GetFormValueData();
  if (!valueData.HasValidNode())
    return;

  CFX_RectF rtArc = GetRectWithoutRotate();
  CXFA_MarginData marginData = m_pDataAcc->GetMarginData();
  if (marginData.HasValidNode())
    XFA_RectWidthoutMargin(rtArc, marginData);

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  DrawBorder(pGS, valueData.GetArcData(), rtArc, mtRotate);
}
