// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/content_compositor.h"
#include "content_compositor.h"

namespace content {

WorkerTreeCompositor::WorkerTreeCompositor() {}

WorkerTreeCompositor::~WorkerTreeCompositor() {
  binding_runner_->RequestQuit();
  binding_runner_.reset();
}

void WorkerTreeCompositor::InitCC(ContentInitParams params) {
  config_ = params.config;

  event_runner_ = new EventRunner();
  binding_runner_ = new BindingRunner();

  event_runner_->InitEventDispatcher(params.host_window);

  // Init renderer in binding thread for sync mode
  binding_runner_->InitBindingComponents(params);
}

void WorkerTreeCompositor::ContentMain() {
  // Launch script thread
  binding_runner_->BindingMain();

  // Launch event loop
  event_runner_->EventMain();
}

}  // namespace content
