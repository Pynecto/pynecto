#ifndef PYNECTO_CONTEXT_H
#define PYNECTO_CONTEXT_H

#include <pynecto/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PncContextConfig {
    pnc_bool  headless;              /* SDL dummy video driver — CI / logic-only tests */
    pnc_u32   event_queue_capacity;  /* 0 = default (1024) */
} PncContextConfig;

PNC_API PncContextConfig pnc_context_config_default(void);

PNC_API pnc_bool pnc_context_init(const PncContextConfig *config);
PNC_API void pnc_context_shutdown(void);
PNC_API pnc_bool pnc_context_is_initialized(void);

PNC_API pnc_u64 pnc_time_ms(void);
PNC_API pnc_f64 pnc_time_delta(void);

#ifdef __cplusplus
}
#endif

#endif /* PYNECTO_CONTEXT_H */
