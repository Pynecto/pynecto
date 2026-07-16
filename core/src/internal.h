#ifndef PYNECTO_INTERNAL_H
#define PYNECTO_INTERNAL_H

#include <pynecto/platform.h>
#include <pynecto/event.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(PNC_COMPILER_GCC) || defined(PNC_COMPILER_CLANG)
#   define PNC_PRINTF_ATTR(fmt_idx, args_idx) __attribute__((format(printf, fmt_idx, args_idx)))
#else
#   define PNC_PRINTF_ATTR(fmt_idx, args_idx)
#endif

/* platform.c — thread-local error string, shared by window.c/event.c/context.c */
void pnc__set_error(const char *fmt, ...) PNC_PRINTF_ATTR(1, 2);
void pnc__set_error_from_sdl(void);

/* event.c — ring buffer lifecycle, owned by context.c's init/shutdown */
pnc_bool pnc__event_queue_init(pnc_u32 capacity);
void pnc__event_queue_shutdown(void);

/* window.c — live window count, checked by context.c on shutdown */
pnc_u32 pnc__window_count(void);

#ifdef __cplusplus
}
#endif

#endif /* PYNECTO_INTERNAL_H */
