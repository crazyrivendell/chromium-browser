// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/devtools/chrome_devtools_session.h"

#include "chrome/browser/devtools/protocol/browser_handler.h"
#include "chrome/browser/devtools/protocol/page_handler.h"
#include "content/public/browser/devtools_agent_host.h"

#if defined(OS_CHROMEOS)
#include "chrome/browser/devtools/protocol/window_manager_handler.h"
#endif

ChromeDevToolsSession::ChromeDevToolsSession(
    content::DevToolsAgentHost* agent_host,
    int session_id)
    : agent_host_(agent_host),
      session_id_(session_id),
      dispatcher_(std::make_unique<protocol::UberDispatcher>(this)) {
  dispatcher_->setFallThroughForNotFound(true);
  if (agent_host->GetWebContents()) {
    page_handler_ = std::make_unique<PageHandler>(agent_host->GetWebContents(),
                                                  dispatcher_.get());
  }
  browser_handler_ = std::make_unique<BrowserHandler>(dispatcher_.get());
#if defined(OS_CHROMEOS)
  window_manager_protocl_handler_ =
      std::make_unique<WindowManagerHandler>(dispatcher_.get());
#endif
}

ChromeDevToolsSession::~ChromeDevToolsSession() = default;

void ChromeDevToolsSession::sendProtocolResponse(
    int call_id,
    std::unique_ptr<protocol::Serializable> message) {
  agent_host_->SendProtocolMessageToClient(session_id_, message->serialize());
}

void ChromeDevToolsSession::sendProtocolNotification(
    std::unique_ptr<protocol::Serializable> message) {
  agent_host_->SendProtocolMessageToClient(session_id_, message->serialize());
}

void ChromeDevToolsSession::flushProtocolNotifications() {}
