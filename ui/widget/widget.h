// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WIDGET_WIDGET_H_
#define UI_WIDGET_WIDGET_H_

#include <memory>
#include <optional>
#include <string>

#include "base/math/math.h"
#include "base/memory/weak_ptr.h"

union SDL_Event;
struct SDL_Window;

namespace ui {

class WidgetDelegate {
 public:
  virtual ~WidgetDelegate() = default;

  virtual void OnWidgetDestroying() {}
};

class Widget {
 public:
  enum class WindowPlacement {
    Show = 0,
    Hide,
    Maximum,
    Minimum,
  };

  struct InitParams {
    InitParams() = default;

    InitParams(InitParams&&) = default;
    InitParams(const InitParams&) = delete;
    InitParams& operator=(const InitParams&) = delete;

    std::string title;

    bool resizable = false;

    bool fullscreen = false;

    std::optional<base::Vec2i> position;
    base::Vec2i size;

    bool activitable = true;
    std::unique_ptr<WidgetDelegate> delegate;

    WindowPlacement window_state = WindowPlacement::Show;
  };

  Widget();
  virtual ~Widget();

  Widget(const Widget&) = delete;
  Widget& operator=(const Widget&) = delete;

  void Init(InitParams params);

  void SetFullscreen(bool fullscreen);
  bool IsFullscreen();

  SDL_Window* AsSDLWindow() { return window_; }
  base::WeakPtr<Widget> AsWeakPtr() { return weak_ptr_factory_.GetWeakPtr(); }

  base::Vec2i GetPosition();
  base::Vec2i GetSize();

  static Widget* FromWindowID(uint32_t window_id);

 private:
  static void UIEventDispatcher(const SDL_Event& sdl_event);
  SDL_Window* window_;
  std::unique_ptr<WidgetDelegate> delegate_;

  base::WeakPtrFactory<Widget> weak_ptr_factory_{this};
};

}  // namespace ui

#endif  // UI_WIDGET_WIDGET_H_