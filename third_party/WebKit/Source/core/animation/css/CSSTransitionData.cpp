// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/animation/css/CSSTransitionData.h"

#include "core/animation/Timing.h"

namespace blink {

CSSTransitionData::CSSTransitionData() {
  property_list_.push_back(InitialProperty());
}

CSSTransitionData::CSSTransitionData(const CSSTransitionData& other)
    : CSSTimingData(other), property_list_(other.property_list_) {}

bool CSSTransitionData::TransitionsMatchForStyleRecalc(
    const CSSTransitionData& other) const {
  return property_list_ == other.property_list_ &&
         TimingMatchForStyleRecalc(other);
}

Timing CSSTransitionData::ConvertToTiming(size_t index) const {
  DCHECK_LT(index, property_list_.size());
  // Note that the backwards fill part is required for delay to work.
  Timing timing = CSSTimingData::ConvertToTiming(index);
  timing.fill_mode = Timing::FillMode::NONE;
  return timing;
}

}  // namespace blink
