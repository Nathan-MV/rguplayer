// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_graphics.h"

#include "binding/mri/init_bitmap.h"
#include "content/public/graphics.h"

namespace binding {

MRI_METHOD(graphics_update) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->Update();

  return Qnil;
}

MRI_METHOD(graphics_freeze) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->Freeze();

  return Qnil;
}

MRI_METHOD(graphics_transition) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int duration = 8;
  VALUE trans_bitmap = Qnil;
  int vague = 40;
  MriParseArgsTo(argc, argv, "|ioi", &duration, &trans_bitmap, &vague);

  scoped_refptr<content::Bitmap> transmap;
  if (RB_TYPE_P(trans_bitmap, RUBY_T_STRING)) {
    std::string filename(RSTRING_PTR(trans_bitmap), RSTRING_LEN(trans_bitmap));
    if (!filename.empty()) {
      MRI_GUARD(transmap = new content::Bitmap(screen, filename););
    }
  } else if (rb_typeddata_is_kind_of(trans_bitmap, &kBitmapDataType)) {
    MRI_GUARD(transmap = MriCheckStructData<content::Bitmap>(trans_bitmap,
                                                             kBitmapDataType););
  }

  MRI_GUARD(screen->Transition(duration, transmap, vague););

  return Qnil;
}

MRI_METHOD(graphics_wait) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->Wait(v);

  return Qnil;
}

MRI_METHOD(graphics_fadein) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->FadeIn(v);

  return Qnil;
}

MRI_METHOD(graphics_fadeout) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int v;
  MriParseArgsTo(argc, argv, "i", &v);

  screen->FadeOut(v);

  return Qnil;
}

MRI_METHOD(graphics_snap_to_bitmap) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  scoped_refptr<content::Bitmap> snap;
  MRI_GUARD(snap = screen->SnapToBitmap(););

  VALUE obj = MriWrapObject(snap, kBitmapDataType);
  bitmap_init_prop(snap, obj);

  return obj;
}

MRI_METHOD(graphics_frame_reset) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  screen->FrameReset();

  return Qnil;
}

MRI_METHOD(graphics_width) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetWidth());
}

MRI_METHOD(graphics_height) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetHeight());
}

MRI_METHOD(graphics_resize_screen) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int w, h;
  MriParseArgsTo(argc, argv, "ii", &w, &h);

  screen->ResizeScreen(base::Vec2i(w, h));

  return Qnil;
}

#define GRAPHICS_DEFINE_ATTR(name)            \
  MRI_METHOD(graphics_get_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    return rb_fix_new(screen->Get##name());   \
  }                                           \
  MRI_METHOD(graphics_set_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    int v;                                    \
    MriParseArgsTo(argc, argv, "i", &v);      \
    screen->Set##name(v);                     \
    return Qnil;                              \
  }

#define GRAPHICS_DEFINE_ATTR_BOOL(name)       \
  MRI_METHOD(graphics_get_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    return MRI_BOOL_NEW(screen->Get##name()); \
  }                                           \
  MRI_METHOD(graphics_set_##name) {           \
    scoped_refptr<content::Graphics> screen = \
        MriGetGlobalRunner()->graphics();     \
    bool v;                                   \
    MriParseArgsTo(argc, argv, "b", &v);      \
    screen->Set##name(v);                     \
    return Qnil;                              \
  }

GRAPHICS_DEFINE_ATTR(FrameRate);
GRAPHICS_DEFINE_ATTR(FrameCount);
GRAPHICS_DEFINE_ATTR(Brightness);
GRAPHICS_DEFINE_ATTR(VSync);
GRAPHICS_DEFINE_ATTR_BOOL(Fullscreen);
GRAPHICS_DEFINE_ATTR_BOOL(FrameSkip);
GRAPHICS_DEFINE_ATTR_BOOL(BackgroundRunning);

MRI_METHOD(graphics_play_movie) {
  LOG(WARNING) << "Not implement Graphics#play_movie";
  return Qnil;
}

MRI_METHOD(graphics_reset) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();
  screen->Reset();

  return Qnil;
}

MRI_METHOD(graphics_resize_window) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();
  int w, h;
  MriParseArgsTo(argc, argv, "ii", &w, &h);
  screen->ResizeWindow(w, h);
  return Qnil;
}

MRI_METHOD(graphics_window_handle) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return UINT2NUM(screen->GetWindowHandle());
}

MRI_METHOD(graphics_display_width) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetDisplayWidth());
}

MRI_METHOD(graphics_display_height) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  return rb_fix_new(screen->GetDisplayHeight());
}

MRI_METHOD(graphics_set_offset) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();
  int x, y;
  MriParseArgsTo(argc, argv, "ii", &x, &y);
  screen->SetDrawableOffset(base::Vec2i(x, y));
  return Qnil;
}

MRI_METHOD(graphics_get_offset) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();
  auto size = screen->GetDrawableOffset();

  VALUE ret = rb_ary_new();
  rb_ary_push(ret, INT2FIX(size.x));
  rb_ary_push(ret, INT2FIX(size.y));
  return ret;
}

MRI_METHOD(graphics_set_window_favicon) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  VALUE bitmap;
  MriParseArgsTo(argc, argv, "o", &bitmap);

  scoped_refptr<content::Bitmap> target =
      MriCheckStructData<content::Bitmap>(bitmap, kBitmapDataType);

  screen->SetWindowFavicon(target);

  return Qnil;
}

MRI_METHOD(graphics_set_window_title) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  std::string title;
  MriParseArgsTo(argc, argv, "s", &title);

  screen->SetWindowTitle(title);

  return Qnil;
}

MRI_METHOD(graphics_set_window_minimum_size) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  int min_w, min_h;
  MriParseArgsTo(argc, argv, "ii", &min_w, &min_h);

  screen->SetWindowMinimumSize(base::Vec2i(min_w, min_h));

  return Qnil;
}

MRI_METHOD(graphics_set_window_aspect_ratio) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  double min_r, max_r;
  MriParseArgsTo(argc, argv, "ff", &min_r, &max_r);

  screen->SetWindowAspectRatio(min_r, max_r);

  return Qnil;
}

MRI_METHOD(graphics_set_window_always_on_top) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  bool on_top;
  MriParseArgsTo(argc, argv, "b", &on_top);

  screen->SetWindowAlwaysOnTop(on_top);

  return Qnil;
}

void InitGraphicsBinding() {
  VALUE module = rb_define_module("Graphics");

  MriDefineModuleFunction(module, "update", graphics_update);
  MriDefineModuleFunction(module, "wait", graphics_wait);
  MriDefineModuleFunction(module, "fadeout", graphics_fadeout);
  MriDefineModuleFunction(module, "fadein", graphics_fadein);
  MriDefineModuleFunction(module, "freeze", graphics_freeze);
  MriDefineModuleFunction(module, "transition", graphics_transition);
  MriDefineModuleFunction(module, "snap_to_bitmap", graphics_snap_to_bitmap);
  MriDefineModuleFunction(module, "frame_reset", graphics_frame_reset);
  MriDefineModuleFunction(module, "width", graphics_width);
  MriDefineModuleFunction(module, "height", graphics_height);
  MriDefineModuleFunction(module, "resize_screen", graphics_resize_screen);
  MriDefineModuleFunction(module, "play_movie", graphics_play_movie);
  MriDefineModuleFunction(module, "__reset__", graphics_reset);

  MriDefineModuleAttr(module, "frame_rate", graphics, FrameRate);
  MriDefineModuleAttr(module, "frame_count", graphics, FrameCount);
  MriDefineModuleAttr(module, "brightness", graphics, Brightness);

  MriDefineModuleFunction(module, "window_handle", graphics_window_handle);
  MriDefineModuleFunction(module, "resize_window", graphics_resize_window);
  MriDefineModuleAttr(module, "vsync", graphics, VSync);
  MriDefineModuleAttr(module, "fullscreen", graphics, Fullscreen);
  MriDefineModuleAttr(module, "frame_skip", graphics, FrameSkip);
  MriDefineModuleAttr(module, "background_running", graphics,
                      BackgroundRunning);

  MriDefineModuleFunction(module, "display_width", graphics_display_width);
  MriDefineModuleFunction(module, "display_height", graphics_display_height);

  MriDefineModuleFunction(module, "set_drawable_offset", graphics_set_offset);
  MriDefineModuleFunction(module, "get_drawable_offset", graphics_get_offset);

  MriDefineModuleFunction(module, "set_window_favicon",
                          graphics_set_window_favicon);
  MriDefineModuleFunction(module, "set_window_title",
                          graphics_set_window_title);
  MriDefineModuleFunction(module, "set_window_minimum_size",
                          graphics_set_window_minimum_size);
  MriDefineModuleFunction(module, "set_window_aspect_ratio",
                          graphics_set_window_aspect_ratio);
  MriDefineModuleFunction(module, "set_window_always_on_top",
                          graphics_set_window_always_on_top);
}

}  // namespace binding
