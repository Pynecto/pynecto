#include <pynecto/context.h>

#include "internal.h"

#include <SDL3/SDL.h>

static pnc_bool g_initialized     = PNC_FALSE;
static pnc_u64  g_perf_freq       = 0;
static pnc_u64  g_perf_epoch      = 0;
static pnc_u64  g_last_time_ticks = 0;

PncContextConfig pnc_context_config_default(void) {
    PncContextConfig config;
    config.headless = PNC_FALSE;
    config.event_queue_capacity = 0;
    return config;
}

pnc_bool pnc_context_init(const PncContextConfig *config) {
    PncContextConfig local_config;
    pnc_u32 queue_capacity;

    if (g_initialized) {
        return PNC_TRUE;
    }

    local_config = (config != NULL) ? *config : pnc_context_config_default();

    if (local_config.headless) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        pnc__set_error_from_sdl();
        return PNC_FALSE;
    }

    queue_capacity = (local_config.event_queue_capacity != 0) ? local_config.event_queue_capacity : 1024;

    if (!pnc__event_queue_init(queue_capacity)) {
        SDL_Quit();
        return PNC_FALSE;
    }

    g_perf_freq = SDL_GetPerformanceFrequency();
    g_perf_epoch = SDL_GetPerformanceCounter();
    g_last_time_ticks = g_perf_epoch;
    g_initialized = PNC_TRUE;

    return PNC_TRUE;
}

void pnc_context_shutdown(void) {
    if (!g_initialized) {
        return;
    }

    PNC_ASSERT(pnc__window_count() == 0);

    pnc__event_queue_shutdown();
    SDL_Quit();

    g_initialized = PNC_FALSE;
    g_perf_freq = 0;
    g_perf_epoch = 0;
    g_last_time_ticks = 0;
}

pnc_bool pnc_context_is_initialized(void) {
    return g_initialized;
}

pnc_u64 pnc_time_ms(void) {
    pnc_u64 now, elapsed_ticks;

    PNC_ASSERT(g_initialized);

    now = SDL_GetPerformanceCounter();
    elapsed_ticks = now - g_perf_epoch;

    return (elapsed_ticks * (pnc_u64)1000) / g_perf_freq;
}

pnc_f64 pnc_time_delta(void) {
    pnc_u64 now, elapsed_ticks;
    pnc_f64 delta;

    PNC_ASSERT(g_initialized);

    now = SDL_GetPerformanceCounter();
    elapsed_ticks = now - g_last_time_ticks;
    delta = (pnc_f64)elapsed_ticks / (pnc_f64)g_perf_freq;
    g_last_time_ticks = now;

    return delta;
}
