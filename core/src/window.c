#include <pynecto/window.h>

#include "internal.h"

#include <glad/gl.h>

#include <SDL3/SDL.h>

#include <stdlib.h>

struct PncWindow {
    SDL_Window *sdl_window;
};

/* Single shared GL context for the whole process (per design decision):
 * created by the first window, reused by every later window via
 * SDL_GL_MakeCurrent, destroyed when the last window goes away. GL
 * attributes (major/minor/debug) are therefore fixed by whichever window
 * creates the context first; later windows' gl_* config fields are ignored. */
static SDL_GLContext g_shared_gl_context = NULL;
static SDL_Window    *g_current_gl_window = NULL;
static pnc_u32        g_window_count = 0;
static pnc_i32         g_shared_gl_major = 0;
static pnc_i32         g_shared_gl_minor = 0;

static SDL_WindowFlags pnc_to_sdl_window_flags(PncWindowFlags flags) {
    Uint64 result = 0;

    if (flags & PNC_WINDOW_RESIZABLE)     result |= SDL_WINDOW_RESIZABLE;
    if (flags & PNC_WINDOW_BORDERLESS)    result |= SDL_WINDOW_BORDERLESS;
    if (flags & PNC_WINDOW_FULLSCREEN)    result |= SDL_WINDOW_FULLSCREEN;
    if (flags & PNC_WINDOW_MAXIMIZED)     result |= SDL_WINDOW_MAXIMIZED;
    if (flags & PNC_WINDOW_MINIMIZED)     result |= SDL_WINDOW_MINIMIZED;
    if (flags & PNC_WINDOW_HIGH_DPI)      result |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (flags & PNC_WINDOW_ALWAYS_ON_TOP) result |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (flags & PNC_WINDOW_HIDDEN)        result |= SDL_WINDOW_HIDDEN;

    return (SDL_WindowFlags)result;
}

PncWindowConfig pnc_window_config_default(void) {
    PncWindowConfig config;

    config.title = "Pynecto";
    config.width = 1280;
    config.height = 720;
    config.min_width = 0;
    config.min_height = 0;
    config.max_width = 0;
    config.max_height = 0;
    config.x = PNC_WINDOW_POS_CENTERED;
    config.y = PNC_WINDOW_POS_CENTERED;
    config.flags = PNC_WINDOW_RESIZABLE | PNC_WINDOW_HIGH_DPI;
    config.display_index = 0;
    config.gl_major = 3;
    config.gl_minor = 3;
    config.gl_debug = PNC_FALSE;
    config.clear_color_r = 0.10f;
    config.clear_color_g = 0.10f;
    config.clear_color_b = 0.12f;
    config.clear_color_a = 1.0f;

    return config;
}

PncWindow *pnc_window_create(const PncWindowConfig *config) {
    PncWindowConfig local_config;
    SDL_WindowFlags sdl_flags;
    SDL_Window *sdl_window;
    PncWindow *window;
    pnc_i32 gl_major, gl_minor;
    pnc_bool is_first_window;

    if (config == NULL) {
        pnc__set_error("pnc_window_create: config must not be NULL");
        return NULL;
    }

    local_config = *config;

    if (local_config.width <= 0 || local_config.height <= 0) {
        pnc__set_error("pnc_window_create: width/height must be positive");
        return NULL;
    }

    if (local_config.flags & PNC_WINDOW_VULKAN) {
        pnc__set_error("pnc_window_create: Vulkan is not supported in this build");
        return NULL;
    }

    is_first_window = (g_window_count == 0);

    sdl_flags = pnc_to_sdl_window_flags(local_config.flags) | SDL_WINDOW_OPENGL;

    gl_major = is_first_window ? ((local_config.gl_major != 0) ? local_config.gl_major : 3) : g_shared_gl_major;
    gl_minor = is_first_window ? ((local_config.gl_minor != 0) ? local_config.gl_minor : 3) : g_shared_gl_minor;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    if (is_first_window && local_config.gl_debug) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    }

    sdl_window = SDL_CreateWindow(local_config.title != NULL ? local_config.title : "", local_config.width,
                                   local_config.height, sdl_flags);
    if (sdl_window == NULL) {
        pnc__set_error_from_sdl();
        return NULL;
    }

    if (local_config.x != PNC_WINDOW_POS_DEFAULT || local_config.y != PNC_WINDOW_POS_DEFAULT) {
        int actual_x = (local_config.x == PNC_WINDOW_POS_CENTERED) ? SDL_WINDOWPOS_CENTERED : local_config.x;
        int actual_y = (local_config.y == PNC_WINDOW_POS_CENTERED) ? SDL_WINDOWPOS_CENTERED : local_config.y;
        SDL_SetWindowPosition(sdl_window, actual_x, actual_y);
    }

    if (local_config.min_width > 0 || local_config.min_height > 0) {
        SDL_SetWindowMinimumSize(sdl_window, local_config.min_width, local_config.min_height);
    }
    if (local_config.max_width > 0 || local_config.max_height > 0) {
        SDL_SetWindowMaximumSize(sdl_window, local_config.max_width, local_config.max_height);
    }

    if (is_first_window) {
        int glad_version;

        g_shared_gl_context = SDL_GL_CreateContext(sdl_window);
        if (g_shared_gl_context == NULL) {
            pnc__set_error_from_sdl();
            SDL_DestroyWindow(sdl_window);
            return NULL;
        }

        if (!SDL_GL_MakeCurrent(sdl_window, g_shared_gl_context)) {
            pnc__set_error_from_sdl();
            SDL_GL_DestroyContext(g_shared_gl_context);
            g_shared_gl_context = NULL;
            SDL_DestroyWindow(sdl_window);
            return NULL;
        }

        glad_version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        if (glad_version == 0 || !GLAD_GL_VERSION_3_3) {
            pnc__set_error("failed to load OpenGL 3.3 core functions (driver too old?)");
            SDL_GL_DestroyContext(g_shared_gl_context);
            g_shared_gl_context = NULL;
            SDL_DestroyWindow(sdl_window);
            return NULL;
        }

        g_shared_gl_major = gl_major;
        g_shared_gl_minor = gl_minor;
        g_current_gl_window = sdl_window;
    }

    window = (PncWindow *)malloc(sizeof(PncWindow));
    if (window == NULL) {
        pnc__set_error("pnc_window_create: out of memory");
        /* If this was the first window, the shared GL context now has no
         * owning window — an extremely unlikely allocation failure right
         * after window/context creation succeeded; accepted as a Phase 1
         * edge case rather than unwinding shared GL state here. */
        SDL_DestroyWindow(sdl_window);
        return NULL;
    }
    window->sdl_window = sdl_window;

    g_window_count++;

    return window;
}

void pnc_window_destroy(PncWindow *window) {
    if (window == NULL) {
        return;
    }

    if (g_current_gl_window == window->sdl_window) {
        g_current_gl_window = NULL;
    }

    SDL_DestroyWindow(window->sdl_window);
    free(window);

    if (g_window_count > 0) {
        g_window_count--;
    }

    if (g_window_count == 0 && g_shared_gl_context != NULL) {
        SDL_GL_DestroyContext(g_shared_gl_context);
        g_shared_gl_context = NULL;
    }
}

void pnc_window_show(PncWindow *window) {
    if (!SDL_ShowWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_hide(PncWindow *window) {
    if (!SDL_HideWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_set_title(PncWindow *window, const char *title) {
    if (!SDL_SetWindowTitle(window->sdl_window, title)) {
        pnc__set_error_from_sdl();
    }
}

pnc_bool pnc_window_set_size(PncWindow *window, pnc_i32 width, pnc_i32 height) {
    if (!SDL_SetWindowSize(window->sdl_window, width, height)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }
    return PNC_TRUE;
}

void pnc_window_set_position(PncWindow *window, pnc_i32 x, pnc_i32 y) {
    if (!SDL_SetWindowPosition(window->sdl_window, x, y)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_set_min_size(PncWindow *window, pnc_i32 min_w, pnc_i32 min_h) {
    if (!SDL_SetWindowMinimumSize(window->sdl_window, min_w, min_h)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_set_max_size(PncWindow *window, pnc_i32 max_w, pnc_i32 max_h) {
    if (!SDL_SetWindowMaximumSize(window->sdl_window, max_w, max_h)) {
        pnc__set_error_from_sdl();
    }
}

pnc_bool pnc_window_set_fullscreen(PncWindow *window, pnc_bool fullscreen) {
    if (!SDL_SetWindowFullscreen(window->sdl_window, fullscreen)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }
    return PNC_TRUE;
}

void pnc_window_set_resizable(PncWindow *window, pnc_bool resizable) {
    if (!SDL_SetWindowResizable(window->sdl_window, resizable)) {
        pnc__set_error_from_sdl();
    }
}

pnc_bool pnc_window_set_opacity(PncWindow *window, pnc_f32 opacity) {
    if (!SDL_SetWindowOpacity(window->sdl_window, opacity)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }
    return PNC_TRUE;
}

void pnc_window_focus(PncWindow *window) {
    if (!SDL_RaiseWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_maximize(PncWindow *window) {
    if (!SDL_MaximizeWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_minimize(PncWindow *window) {
    if (!SDL_MinimizeWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_restore(PncWindow *window) {
    if (!SDL_RestoreWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

void pnc_window_get_state(const PncWindow *window, PncWindowState *state) {
    SDL_WindowFlags flags;
    int w = 0, h = 0, wpx = 0, hpx = 0, x = 0, y = 0;

    if (window == NULL || state == NULL) {
        return;
    }

    flags = SDL_GetWindowFlags(window->sdl_window);

    SDL_GetWindowSize(window->sdl_window, &w, &h);
    SDL_GetWindowSizeInPixels(window->sdl_window, &wpx, &hpx);
    SDL_GetWindowPosition(window->sdl_window, &x, &y);

    state->width = w;
    state->height = h;
    state->width_px = wpx;
    state->height_px = hpx;
    state->x = x;
    state->y = y;
    state->dpi_scale = SDL_GetWindowDisplayScale(window->sdl_window);
    state->focused = (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
    state->minimized = (flags & SDL_WINDOW_MINIMIZED) != 0;
    state->maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
    state->fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    state->visible = (flags & SDL_WINDOW_HIDDEN) == 0;
    state->mouse_over = (flags & SDL_WINDOW_MOUSE_FOCUS) != 0;
}

pnc_u32 pnc_window_get_id(const PncWindow *window) {
    return SDL_GetWindowID(window->sdl_window);
}

const char *pnc_window_get_title(const PncWindow *window) {
    return SDL_GetWindowTitle(window->sdl_window);
}

pnc_i32 pnc_window_get_width(const PncWindow *window) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window->sdl_window, &w, &h);
    return w;
}

pnc_i32 pnc_window_get_height(const PncWindow *window) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window->sdl_window, &w, &h);
    return h;
}

pnc_f32 pnc_window_get_dpi_scale(const PncWindow *window) {
    return SDL_GetWindowDisplayScale(window->sdl_window);
}

pnc_bool pnc_window_is_focused(const PncWindow *window) {
    return (SDL_GetWindowFlags(window->sdl_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

pnc_bool pnc_window_is_minimized(const PncWindow *window) {
    return (SDL_GetWindowFlags(window->sdl_window) & SDL_WINDOW_MINIMIZED) != 0;
}

pnc_bool pnc_window_is_maximized(const PncWindow *window) {
    return (SDL_GetWindowFlags(window->sdl_window) & SDL_WINDOW_MAXIMIZED) != 0;
}

pnc_bool pnc_window_is_fullscreen(const PncWindow *window) {
    return (SDL_GetWindowFlags(window->sdl_window) & SDL_WINDOW_FULLSCREEN) != 0;
}

pnc_bool pnc_window_is_visible(const PncWindow *window) {
    return (SDL_GetWindowFlags(window->sdl_window) & SDL_WINDOW_HIDDEN) == 0;
}

pnc_bool pnc_window_gl_make_current(PncWindow *window) {
    if (window == NULL) {
        pnc__set_error("pnc_window_gl_make_current: window must not be NULL");
        return PNC_FALSE;
    }

    if (g_current_gl_window == window->sdl_window) {
        return PNC_TRUE;
    }

    if (!SDL_GL_MakeCurrent(window->sdl_window, g_shared_gl_context)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }

    g_current_gl_window = window->sdl_window;
    return PNC_TRUE;
}

void pnc_window_gl_swap_buffers(PncWindow *window) {
    if (!SDL_GL_SwapWindow(window->sdl_window)) {
        pnc__set_error_from_sdl();
    }
}

pnc_bool pnc_window_gl_set_swap_interval(PncWindow *window, pnc_i32 interval) {
    PNC_UNUSED(window); /* swap interval is global to whichever context is current */

    if (!SDL_GL_SetSwapInterval(interval)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }
    return PNC_TRUE;
}

pnc_u32 pnc__window_count(void) {
    return g_window_count;
}
