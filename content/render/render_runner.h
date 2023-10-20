// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDER_RENDER_RUNNER_H_
#define CONTENT_RENDER_RENDER_RUNNER_H_

#include "base/worker/thread_worker.h"
#include "ui/widget/widget.h"

namespace content {

class RenderRunner final {
 public:
  struct InitParams {
    base::WeakPtr<ui::Widget> ogl_window = nullptr;

    base::Rect initial_resolution;

    InitParams() = default;
    InitParams(const InitParams&) = delete;
    InitParams(InitParams&&) = default;
  };

  RenderRunner(bool sync);
  ~RenderRunner();

  RenderRunner(const RenderRunner&) = delete;
  RenderRunner& operator=(const RenderRunner&) = delete;

  void InitGLContext(InitParams inital_params);
  scoped_refptr<base::SequencedTaskRunner> GetTaskRunner();

 private:
  void CreateRenderContextInternal(InitParams renderer_settings);
  void ReleaseContextInternal();

  SDL_GLContext glctx_ = nullptr;
  std::unique_ptr<base::ThreadWorker> render_worker_;

  base::WeakPtrFactory<RenderRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_RENDER_RENDER_RUNNER_H_