#include <pynecto/window.h>

#include "internal.h"

#include <SDL3/SDL.h>

#include <stdarg.h>
#include <stdio.h>

#if defined(PNC_COMPILER_MSVC)
#   define PNC_THREAD_LOCAL __declspec(thread)
#else
#   define PNC_THREAD_LOCAL _Thread_local
#endif

#define PNC_ERROR_BUFFER_SIZE 256

static PNC_THREAD_LOCAL char g_error_buffer[PNC_ERROR_BUFFER_SIZE] = {0};

void pnc__set_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(g_error_buffer, sizeof(g_error_buffer), fmt, args);
    va_end(args);
}

void pnc__set_error_from_sdl(void) {
    const char *sdl_err = SDL_GetError();
    if (sdl_err != NULL && sdl_err[0] != '\0') {
        pnc__set_error("%s", sdl_err);
    } else {
        pnc__set_error("unknown SDL error");
    }
}

const char *pnc_get_error(void) {
    return g_error_buffer;
}
