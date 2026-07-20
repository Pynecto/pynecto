#ifndef PYNECTO_WINDOW_H
#define PYNECTO_WINDOW_H

#include <pynecto/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef pnc_u32 PncWindowFlags;

#define PNC_WINDOW_NONE           ((PncWindowFlags)0x00000000)
#define PNC_WINDOW_RESIZABLE      ((PncWindowFlags)0x00000001)
#define PNC_WINDOW_BORDERLESS     ((PncWindowFlags)0x00000002)
#define PNC_WINDOW_FULLSCREEN     ((PncWindowFlags)0x00000004)
#define PNC_WINDOW_MAXIMIZED      ((PncWindowFlags)0x00000008)
#define PNC_WINDOW_MINIMIZED      ((PncWindowFlags)0x00000010)
#define PNC_WINDOW_HIGH_DPI       ((PncWindowFlags)0x00000020)
#define PNC_WINDOW_ALWAYS_ON_TOP  ((PncWindowFlags)0x00000040)
#define PNC_WINDOW_HIDDEN         ((PncWindowFlags)0x00000080)
#define PNC_WINDOW_OPENGL         ((PncWindowFlags)0x00000100)
#define PNC_WINDOW_VULKAN         ((PncWindowFlags)0x00000200)

#define PNC_WINDOW_POS_CENTERED   (-1)
#define PNC_WINDOW_POS_DEFAULT    (-2)

typedef struct PncWindowConfig {
    const char     *title;
    pnc_i32         width;
    pnc_i32         height;
    pnc_i32         min_width;
    pnc_i32         min_height;
    pnc_i32         max_width;
    pnc_i32         max_height;
    pnc_i32         x;
    pnc_i32         y;
    PncWindowFlags  flags;
    pnc_i32         display_index;
    pnc_i32         gl_major;
    pnc_i32         gl_minor;
    pnc_bool        gl_debug;
    pnc_f32         clear_color_r;
    pnc_f32         clear_color_g;
    pnc_f32         clear_color_b;
    pnc_f32         clear_color_a;

} PncWindowConfig;

typedef struct PncWindowState {
    pnc_i32   width;
    pnc_i32   height;
    pnc_i32   width_px;
    pnc_i32   height_px;
    pnc_i32   x;
    pnc_i32   y;
    pnc_f32   dpi_scale;
    pnc_bool  focused;
    pnc_bool  minimized;
    pnc_bool  maximized;
    pnc_bool  fullscreen;
    pnc_bool  visible;
    pnc_bool  mouse_over;
} PncWindowState;

typedef struct PncWindow PncWindow;

PNC_API PncWindowConfig pnc_window_config_default(void);
PNC_API PncWindow *pnc_window_create(const PncWindowConfig *config);
PNC_API void pnc_window_destroy(PncWindow *window);
PNC_API void pnc_window_show(PncWindow *window);
PNC_API void pnc_window_hide(PncWindow *window);

PNC_API void pnc_window_set_title(PncWindow *window, const char *title);
PNC_API pnc_bool pnc_window_set_size(PncWindow *window, pnc_i32 width, pnc_i32 height);
PNC_API void pnc_window_set_position(PncWindow *window, pnc_i32 x, pnc_i32 y);
PNC_API void pnc_window_set_min_size(PncWindow *window, pnc_i32 min_w, pnc_i32 min_h);
PNC_API void pnc_window_set_max_size(PncWindow *window, pnc_i32 max_w, pnc_i32 max_h);
PNC_API pnc_bool pnc_window_set_fullscreen(PncWindow *window, pnc_bool fullscreen);
PNC_API void pnc_window_set_resizable(PncWindow *window, pnc_bool resizable);
PNC_API pnc_bool pnc_window_set_opacity(PncWindow *window, pnc_f32 opacity);
PNC_API void pnc_window_focus(PncWindow *window);
PNC_API void pnc_window_maximize(PncWindow *window);
PNC_API void pnc_window_minimize(PncWindow *window);
PNC_API void pnc_window_restore(PncWindow *window);

PNC_API void pnc_window_get_state(const PncWindow *window, PncWindowState *state);
PNC_API pnc_u32 pnc_window_get_id(const PncWindow *window);
PNC_API const char *pnc_window_get_title(const PncWindow *window);
PNC_API pnc_i32 pnc_window_get_width(const PncWindow *window);
PNC_API pnc_i32 pnc_window_get_height(const PncWindow *window);
PNC_API pnc_f32 pnc_window_get_dpi_scale(const PncWindow *window);

PNC_API pnc_bool pnc_window_is_focused(const PncWindow *window);
PNC_API pnc_bool pnc_window_is_minimized(const PncWindow *window);
PNC_API pnc_bool pnc_window_is_maximized(const PncWindow *window);
PNC_API pnc_bool pnc_window_is_fullscreen(const PncWindow *window);
PNC_API pnc_bool pnc_window_is_visible(const PncWindow *window);

PNC_API pnc_bool pnc_window_gl_make_current(PncWindow *window);
PNC_API void pnc_window_gl_swap_buffers(PncWindow *window);
PNC_API pnc_bool pnc_window_gl_set_swap_interval(PncWindow *window, pnc_i32 interval);

PNC_API const char *pnc_get_error(void);

#ifdef __cplusplus
}
#endif

#endif