// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides the PinchAction object, which zooms into or out of a
// page by a given scale factor:
//   1. var action = new __PinchAction(callback)
//   2. action.start(pinch_options)
'use strict';

(function() {
  function PinchGestureOptions(opt_options) {
    if (opt_options) {
      this.left_anchor_ratio_ = opt_options.left_anchor_ratio;
      this.top_anchor_ratio_ = opt_options.top_anchor_ratio;
      this.scale_factor_ = opt_options.scale_factor;
      this.speed_ = opt_options.speed;
    } else {
      this.left_anchor_ratio_ = 0.5;
      this.top_anchor_ratio_ = 0.5;
      this.scale_factor_ = 2.0;
      this.speed_ = 800;
    }
  }

  function supportedByBrowser() {
    return !!(window.chrome &&
              chrome.gpuBenchmarking &&
              chrome.gpuBenchmarking.pinchBy &&
              chrome.gpuBenchmarking.visualViewportHeight &&
              chrome.gpuBenchmarking.visualViewportWidth);
  }

  // This class zooms into or out of a page, given a number of pixels for
  // the synthetic pinch gesture to cover.
  function PinchAction(opt_callback) {
    this.beginMeasuringHook = function() {};
    this.endMeasuringHook = function() {};

    this.callback_ = opt_callback;
  }

  PinchAction.prototype.start = function(opt_options) {
    this.options_ = new PinchGestureOptions(opt_options);

    requestAnimationFrame(this.startPass_.bind(this));
  };

  PinchAction.prototype.startPass_ = function() {
    this.beginMeasuringHook();

    // TODO(bokan): Remove else-block once gpuBenchmarking is changed to take
    // all coordinates in viewport space. crbug.com/610021.
    let anchorLeft;
    let anchorTop;
    let speed = this.options_.speed_;
    if ('gesturesExpectedInViewportCoordinates' in chrome.gpuBenchmarking) {
      anchorLeft =
          __GestureCommon_GetWindowWidth() *
          this.options_.left_anchor_ratio_;
      anchorTop =
          __GestureCommon_GetWindowHeight() *
          this.options_.top_anchor_ratio_;
      speed = speed * chrome.gpuBenchmarking.pageScaleFactor();
    } else {
      const rect = __GestureCommon_GetBoundingVisibleRect(document.body);
      anchorLeft = rect.left + rect.width * this.options_.left_anchor_ratio_;
      anchorTop = rect.top + rect.height * this.options_.top_anchor_ratio_;
    }

    chrome.gpuBenchmarking.pinchBy(
        this.options_.scale_factor_,
        anchorLeft, anchorTop,
        this.onGestureComplete_.bind(this),
        speed);
  };

  PinchAction.prototype.onGestureComplete_ = function() {
    this.endMeasuringHook();

    if (this.callback_) {
      this.callback_();
    }
  };

  window.__PinchAction = PinchAction;
  window.__PinchAction_SupportedByBrowser = supportedByBrowser;
})();
