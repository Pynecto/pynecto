#include <pynecto/event.h>

#include "internal.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNC_DROP_STRING_SLOTS 8
#define PNC_DROP_STRING_MAX   1024

static PncEvent *g_ring_buffer   = NULL;
static pnc_u32   g_ring_capacity = 0;
static pnc_u32   g_ring_head     = 0; /* index of oldest queued event */
static pnc_u32   g_ring_count    = 0; /* number of queued events */
static pnc_u32   g_user_event_type = 0;
static pnc_bool  g_queue_ready   = PNC_FALSE;

/* Drop-event file/text strings: SDL3 hands ownership of these to the app,
 * but PncDropEvent carries raw const char* in a fixed-size value type with
 * no destructor. Copy into a small rotating pool instead — valid until
 * PNC_DROP_STRING_SLOTS further drops arrive, which is fine since drops are
 * low-frequency interactive events, not a hot path. */
static char      g_drop_strings[PNC_DROP_STRING_SLOTS][PNC_DROP_STRING_MAX];
static pnc_u32   g_drop_string_next = 0;

/* -------------------------------------------------------------------- */
/* Queue lifecycle — driven by context.c                                 */
/* -------------------------------------------------------------------- */

pnc_bool pnc__event_queue_init(pnc_u32 capacity) {
    if (g_queue_ready) {
        return PNC_TRUE;
    }

    g_ring_buffer = (PncEvent *)malloc(sizeof(PncEvent) * capacity);
    if (g_ring_buffer == NULL) {
        pnc__set_error("failed to allocate event ring buffer (%u entries)", capacity);
        return PNC_FALSE;
    }

    g_ring_capacity = capacity;
    g_ring_head = 0;
    g_ring_count = 0;

    g_user_event_type = SDL_RegisterEvents(1);
    if (g_user_event_type == (pnc_u32)-1) {
        pnc__set_error_from_sdl();
        free(g_ring_buffer);
        g_ring_buffer = NULL;
        return PNC_FALSE;
    }

    g_queue_ready = PNC_TRUE;
    return PNC_TRUE;
}

void pnc__event_queue_shutdown(void) {
    if (!g_queue_ready) {
        return;
    }

    free(g_ring_buffer);
    g_ring_buffer = NULL;
    g_ring_capacity = 0;
    g_ring_head = 0;
    g_ring_count = 0;
    g_queue_ready = PNC_FALSE;
}

/* -------------------------------------------------------------------- */
/* Ring buffer helpers                                                   */
/* -------------------------------------------------------------------- */

static PncEvent *ring_tail_ptr(void) {
    pnc_u32 idx;

    if (g_ring_count == 0) {
        return NULL;
    }

    idx = (g_ring_head + g_ring_count - 1) % g_ring_capacity;
    return &g_ring_buffer[idx];
}

static void ring_push(const PncEvent *event) {
    pnc_u32 idx;

    if (g_ring_count == g_ring_capacity) {
        /* overflow: drop oldest, per TRD §2.3 */
        g_ring_head = (g_ring_head + 1) % g_ring_capacity;
        g_ring_count--;
#if defined(PNC_DEBUG)
        fprintf(stderr, "[Pynecto] event queue overflow — dropped oldest event\n");
#endif
    }

    idx = (g_ring_head + g_ring_count) % g_ring_capacity;
    g_ring_buffer[idx] = *event;
    g_ring_count++;
}

static pnc_bool ring_pop(PncEvent *out) {
    if (g_ring_count == 0) {
        return PNC_FALSE;
    }

    *out = g_ring_buffer[g_ring_head];
    g_ring_head = (g_ring_head + 1) % g_ring_capacity;
    g_ring_count--;
    return PNC_TRUE;
}

/* -------------------------------------------------------------------- */
/* SDL constant translation — Pnc and SDL_ values are NOT numerically
 * equivalent (e.g. PNC_MOD_NUM is 0x4000, SDL_KMOD_NUM is 0x1000, and
 * SDL's 0x4000 is SDL_KMOD_MODE/AltGr), so every mapping below is explicit.
 * PncKey is the one exception: it mirrors SDL_Scancode's USB-HID-derived
 * values 1:1, so a bounds-checked cast is correct there.                */
/* -------------------------------------------------------------------- */

static PncModifier sdl_mod_to_pnc(SDL_Keymod mod) {
    PncModifier result = PNC_MOD_NONE;

    if (mod & SDL_KMOD_LSHIFT) result |= PNC_MOD_LSHIFT;
    if (mod & SDL_KMOD_RSHIFT) result |= PNC_MOD_RSHIFT;
    if (mod & SDL_KMOD_LCTRL)  result |= PNC_MOD_LCTRL;
    if (mod & SDL_KMOD_RCTRL)  result |= PNC_MOD_RCTRL;
    if (mod & SDL_KMOD_LALT)   result |= PNC_MOD_LALT;
    if (mod & SDL_KMOD_RALT)   result |= PNC_MOD_RALT;
    if (mod & SDL_KMOD_LGUI)   result |= PNC_MOD_LSUPER;
    if (mod & SDL_KMOD_RGUI)   result |= PNC_MOD_RSUPER;
    if (mod & SDL_KMOD_CAPS)   result |= PNC_MOD_CAPS;
    if (mod & SDL_KMOD_NUM)    result |= PNC_MOD_NUM;

    return result;
}

static SDL_Keymod pnc_mod_to_sdl(PncModifier mod) {
    Uint32 result = SDL_KMOD_NONE;

    if (mod & PNC_MOD_LSHIFT) result |= SDL_KMOD_LSHIFT;
    if (mod & PNC_MOD_RSHIFT) result |= SDL_KMOD_RSHIFT;
    if (mod & PNC_MOD_LCTRL)  result |= SDL_KMOD_LCTRL;
    if (mod & PNC_MOD_RCTRL)  result |= SDL_KMOD_RCTRL;
    if (mod & PNC_MOD_LALT)   result |= SDL_KMOD_LALT;
    if (mod & PNC_MOD_RALT)   result |= SDL_KMOD_RALT;
    if (mod & PNC_MOD_LSUPER) result |= SDL_KMOD_LGUI;
    if (mod & PNC_MOD_RSUPER) result |= SDL_KMOD_RGUI;
    if (mod & PNC_MOD_CAPS)   result |= SDL_KMOD_CAPS;
    if (mod & PNC_MOD_NUM)    result |= SDL_KMOD_NUM;

    return (SDL_Keymod)result;
}

static PncKey sdl_scancode_to_pnc(SDL_Scancode scancode) {
    if ((pnc_u32)scancode >= PNC_KEY_COUNT) {
        return PNC_KEY_UNKNOWN;
    }
    return (PncKey)scancode;
}

static PncMouseButton sdl_button_to_pnc(Uint8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT:   return PNC_MOUSE_BUTTON_LEFT;
    case SDL_BUTTON_MIDDLE: return PNC_MOUSE_BUTTON_MIDDLE;
    case SDL_BUTTON_RIGHT:  return PNC_MOUSE_BUTTON_RIGHT;
    case SDL_BUTTON_X1:     return PNC_MOUSE_BUTTON_X1;
    case SDL_BUTTON_X2:     return PNC_MOUSE_BUTTON_X2;
    default:                return PNC_MOUSE_BUTTON_NONE;
    }
}

static const char *store_drop_string(const char *sdl_owned) {
    char *slot;

    if (sdl_owned == NULL) {
        return NULL;
    }

    slot = g_drop_strings[g_drop_string_next];
    g_drop_string_next = (g_drop_string_next + 1) % PNC_DROP_STRING_SLOTS;
    snprintf(slot, PNC_DROP_STRING_MAX, "%s", sdl_owned);

    return slot;
}

static pnc_u64 ns_to_ms(Uint64 ns) {
    return (pnc_u64)(ns / 1000000ULL);
}

/* -------------------------------------------------------------------- */
/* SDL_Event -> PncEvent translation                                     */
/* -------------------------------------------------------------------- */

static PncEventType translate_window_event_type(Uint32 sdl_type) {
    switch (sdl_type) {
    case SDL_EVENT_WINDOW_SHOWN:                 return PNC_EVENT_WINDOW_SHOWN;
    case SDL_EVENT_WINDOW_HIDDEN:                return PNC_EVENT_WINDOW_HIDDEN;
    case SDL_EVENT_WINDOW_MOVED:                 return PNC_EVENT_WINDOW_MOVED;
    case SDL_EVENT_WINDOW_RESIZED:               return PNC_EVENT_WINDOW_RESIZED;
    case SDL_EVENT_WINDOW_MINIMIZED:             return PNC_EVENT_WINDOW_MINIMIZED;
    case SDL_EVENT_WINDOW_MAXIMIZED:             return PNC_EVENT_WINDOW_MAXIMIZED;
    case SDL_EVENT_WINDOW_RESTORED:              return PNC_EVENT_WINDOW_RESTORED;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:          return PNC_EVENT_WINDOW_FOCUS_IN;
    case SDL_EVENT_WINDOW_FOCUS_LOST:            return PNC_EVENT_WINDOW_FOCUS_OUT;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:       return PNC_EVENT_WINDOW_CLOSE;
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: return PNC_EVENT_WINDOW_DPI_CHANGED;
    default:                                     return PNC_EVENT_NONE;
    }
}

static pnc_bool translate_sdl_event(const SDL_Event *sdl, PncEvent *out) {
    memset(out, 0, sizeof(*out));

    switch (sdl->type) {

    case SDL_EVENT_QUIT:
        out->type = PNC_EVENT_QUIT;
        out->timestamp = ns_to_ms(sdl->quit.timestamp);
        out->window_id = 0;
        return PNC_TRUE;

    case SDL_EVENT_DID_ENTER_BACKGROUND:
        out->type = PNC_EVENT_APP_BACKGROUND;
        out->timestamp = ns_to_ms(sdl->common.timestamp);
        return PNC_TRUE;

    case SDL_EVENT_DID_ENTER_FOREGROUND:
        out->type = PNC_EVENT_APP_FOREGROUND;
        out->timestamp = ns_to_ms(sdl->common.timestamp);
        return PNC_TRUE;

    case SDL_EVENT_WINDOW_SHOWN:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_MOVED:
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_MAXIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: {
        SDL_Window *sdl_window;

        out->type = translate_window_event_type(sdl->type);
        out->window.timestamp = ns_to_ms(sdl->window.timestamp);
        out->window.window_id = sdl->window.windowID;
        out->window.dpi_scale = 1.0f;

        if (sdl->type == SDL_EVENT_WINDOW_MOVED) {
            out->window.x = sdl->window.data1;
            out->window.y = sdl->window.data2;
        } else if (sdl->type == SDL_EVENT_WINDOW_RESIZED) {
            out->window.width = sdl->window.data1;
            out->window.height = sdl->window.data2;
        }

        sdl_window = SDL_GetWindowFromID(sdl->window.windowID);
        if (sdl_window != NULL) {
            out->window.dpi_scale = SDL_GetWindowDisplayScale(sdl_window);
        }

        return PNC_TRUE;
    }

    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        out->type = PNC_EVENT_MOUSE_ENTER;
        out->timestamp = ns_to_ms(sdl->window.timestamp);
        out->window_id = sdl->window.windowID;
        return PNC_TRUE;

    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        out->type = PNC_EVENT_MOUSE_LEAVE;
        out->timestamp = ns_to_ms(sdl->window.timestamp);
        out->window_id = sdl->window.windowID;
        return PNC_TRUE;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        out->type = (sdl->type == SDL_EVENT_KEY_DOWN) ? PNC_EVENT_KEY_DOWN : PNC_EVENT_KEY_UP;
        out->key.timestamp = ns_to_ms(sdl->key.timestamp);
        out->key.window_id = sdl->key.windowID;
        out->key.key = sdl_scancode_to_pnc(sdl->key.scancode);
        out->key.modifiers = sdl_mod_to_pnc(sdl->key.mod);
        out->key.repeat = sdl->key.repeat;
        out->key.pressed = sdl->key.down;
        return PNC_TRUE;

    case SDL_EVENT_TEXT_INPUT:
        out->type = PNC_EVENT_TEXT_INPUT;
        out->text.timestamp = ns_to_ms(sdl->text.timestamp);
        out->text.window_id = sdl->text.windowID;
        snprintf(out->text.text, sizeof(out->text.text), "%s", sdl->text.text);
        return PNC_TRUE;

    case SDL_EVENT_MOUSE_MOTION:
        out->type = PNC_EVENT_MOUSE_MOVE;
        out->mouse_move.timestamp = ns_to_ms(sdl->motion.timestamp);
        out->mouse_move.window_id = sdl->motion.windowID;
        out->mouse_move.x = sdl->motion.x;
        out->mouse_move.y = sdl->motion.y;
        out->mouse_move.dx = sdl->motion.xrel;
        out->mouse_move.dy = sdl->motion.yrel;
        return PNC_TRUE;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        out->type = (sdl->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? PNC_EVENT_MOUSE_DOWN : PNC_EVENT_MOUSE_UP;
        out->mouse_button.timestamp = ns_to_ms(sdl->button.timestamp);
        out->mouse_button.window_id = sdl->button.windowID;
        out->mouse_button.button = sdl_button_to_pnc(sdl->button.button);
        out->mouse_button.x = sdl->button.x;
        out->mouse_button.y = sdl->button.y;
        out->mouse_button.clicks = sdl->button.clicks;
        out->mouse_button.pressed = sdl->button.down;
        return PNC_TRUE;

    case SDL_EVENT_MOUSE_WHEEL:
        out->type = PNC_EVENT_MOUSE_SCROLL;
        out->mouse_scroll.timestamp = ns_to_ms(sdl->wheel.timestamp);
        out->mouse_scroll.window_id = sdl->wheel.windowID;
        out->mouse_scroll.dx = sdl->wheel.x;
        out->mouse_scroll.dy = sdl->wheel.y;
        out->mouse_scroll.x = sdl->wheel.mouse_x;
        out->mouse_scroll.y = sdl->wheel.mouse_y;
        out->mouse_scroll.flipped = (sdl->wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
        return PNC_TRUE;

    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_MOTION:
        out->type = (sdl->type == SDL_EVENT_FINGER_DOWN) ? PNC_EVENT_TOUCH_DOWN
                  : (sdl->type == SDL_EVENT_FINGER_UP)   ? PNC_EVENT_TOUCH_UP
                  :                                        PNC_EVENT_TOUCH_MOVE;
        out->touch.timestamp = ns_to_ms(sdl->tfinger.timestamp);
        out->touch.window_id = sdl->tfinger.windowID;
        out->touch.touch_id = (pnc_i64)sdl->tfinger.touchID;
        out->touch.finger_id = (pnc_i64)sdl->tfinger.fingerID;
        out->touch.x = sdl->tfinger.x;
        out->touch.y = sdl->tfinger.y;
        out->touch.dx = sdl->tfinger.dx;
        out->touch.dy = sdl->tfinger.dy;
        out->touch.pressure = sdl->tfinger.pressure;
        return PNC_TRUE;

    case SDL_EVENT_DROP_FILE:
    case SDL_EVENT_DROP_TEXT:
    case SDL_EVENT_DROP_BEGIN:
    case SDL_EVENT_DROP_COMPLETE:
        out->type = (sdl->type == SDL_EVENT_DROP_FILE)     ? PNC_EVENT_DROP_FILE
                  : (sdl->type == SDL_EVENT_DROP_TEXT)      ? PNC_EVENT_DROP_TEXT
                  : (sdl->type == SDL_EVENT_DROP_BEGIN)     ? PNC_EVENT_DROP_BEGIN
                  :                                           PNC_EVENT_DROP_COMPLETE;
        out->drop.timestamp = ns_to_ms(sdl->drop.timestamp);
        out->drop.window_id = sdl->drop.windowID;
        out->drop.x = sdl->drop.x;
        out->drop.y = sdl->drop.y;
        if (sdl->type == SDL_EVENT_DROP_FILE) {
            out->drop.file = store_drop_string(sdl->drop.data);
        } else if (sdl->type == SDL_EVENT_DROP_TEXT) {
            out->drop.text = store_drop_string(sdl->drop.data);
        }
        return PNC_TRUE;

    case SDL_EVENT_CLIPBOARD_UPDATE:
        out->type = PNC_EVENT_CLIPBOARD_CHANGED;
        out->timestamp = ns_to_ms(sdl->clipboard.timestamp);
        return PNC_TRUE;

    default:
        return PNC_FALSE; /* unknown SDL events dropped silently */
    }
}

/* -------------------------------------------------------------------- */
/* Pump: pull whatever SDL currently has queued into our ring buffer,
 * coalescing consecutive mouse moves for the same window and handling
 * our own cross-thread injected events (see pnc_event_push).            */
/* -------------------------------------------------------------------- */

static void push_translated(const SDL_Event *sdl) {
    PncEvent event;
    PncEvent *tail;

    if (sdl->type == g_user_event_type) {
        PncEvent *heap_event = (PncEvent *)sdl->user.data1;
        if (heap_event != NULL) {
            ring_push(heap_event);
            free(heap_event);
        }
        return;
    }

    if (!translate_sdl_event(sdl, &event)) {
        return;
    }

    if (event.type == PNC_EVENT_MOUSE_MOVE) {
        tail = ring_tail_ptr();
        if (tail != NULL && tail->type == PNC_EVENT_MOUSE_MOVE &&
            tail->mouse_move.window_id == event.mouse_move.window_id) {
            tail->mouse_move.dx += event.mouse_move.dx;
            tail->mouse_move.dy += event.mouse_move.dy;
            tail->mouse_move.x = event.mouse_move.x;
            tail->mouse_move.y = event.mouse_move.y;
            tail->mouse_move.timestamp = event.mouse_move.timestamp;
            return;
        }
    }

    ring_push(&event);
}

static void drain_pending_sdl_events(void) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        push_translated(&sdl_event);
    }
}

/* -------------------------------------------------------------------- */
/* Public API                                                             */
/* -------------------------------------------------------------------- */

pnc_bool pnc_event_poll(PncEvent *event) {
    if (event == NULL) {
        return PNC_FALSE;
    }

    if (ring_pop(event)) {
        return PNC_TRUE;
    }

    drain_pending_sdl_events();

    return ring_pop(event);
}

pnc_bool pnc_event_wait(PncEvent *event, pnc_u32 timeout) {
    SDL_Event sdl_event;

    if (event == NULL) {
        return PNC_FALSE;
    }

    if (ring_pop(event)) {
        return PNC_TRUE;
    }

    if (!SDL_WaitEventTimeout(&sdl_event, (Sint32)timeout)) {
        return PNC_FALSE;
    }

    push_translated(&sdl_event);
    drain_pending_sdl_events();

    return ring_pop(event);
}

pnc_bool pnc_event_push(const PncEvent *event) {
    PncEvent *heap_copy;
    SDL_Event sdl_event;

    if (event == NULL) {
        pnc__set_error("pnc_event_push: event must not be NULL");
        return PNC_FALSE;
    }

    heap_copy = (PncEvent *)malloc(sizeof(PncEvent));
    if (heap_copy == NULL) {
        pnc__set_error("pnc_event_push: out of memory");
        return PNC_FALSE;
    }
    *heap_copy = *event;

    memset(&sdl_event, 0, sizeof(sdl_event));
    sdl_event.type = g_user_event_type;
    sdl_event.user.data1 = heap_copy;
    sdl_event.user.data2 = NULL;

    if (!SDL_PushEvent(&sdl_event)) {
        pnc__set_error_from_sdl();
        free(heap_copy);
        return PNC_FALSE;
    }

    return PNC_TRUE;
}

void pnc_event_flush(PncEventType type) {
    PncEvent *compacted;
    pnc_u32 write_count, i, read_idx;

    drain_pending_sdl_events();

    if (g_ring_count == 0) {
        return;
    }

    compacted = (PncEvent *)malloc(sizeof(PncEvent) * g_ring_count);
    if (compacted == NULL) {
        return; /* best-effort: nothing safe to do without extra memory */
    }

    write_count = 0;
    for (i = 0; i < g_ring_count; i++) {
        read_idx = (g_ring_head + i) % g_ring_capacity;
        if (g_ring_buffer[read_idx].type != type) {
            compacted[write_count++] = g_ring_buffer[read_idx];
        }
    }

    for (i = 0; i < write_count; i++) {
        g_ring_buffer[i] = compacted[i];
    }

    g_ring_head = 0;
    g_ring_count = write_count;

    free(compacted);
}

pnc_bool pnc_event_peek(PncEvent *event) {
    if (event == NULL) {
        return PNC_FALSE;
    }

    if (g_ring_count == 0) {
        drain_pending_sdl_events();
    }

    if (g_ring_count == 0) {
        return PNC_FALSE;
    }

    *event = g_ring_buffer[g_ring_head];
    return PNC_TRUE;
}

const char *pnc_key_name(PncKey key) {
    if ((pnc_u32)key >= PNC_KEY_COUNT) {
        return "Unknown";
    }
    return SDL_GetScancodeName((SDL_Scancode)key);
}

pnc_u32 pnc_key_to_char(PncKey key, PncModifier modifiers) {
    SDL_Keycode keycode;

    if ((pnc_u32)key >= PNC_KEY_COUNT) {
        return 0;
    }

    keycode = SDL_GetKeyFromScancode((SDL_Scancode)key, pnc_mod_to_sdl(modifiers), false);

    if (keycode & SDLK_SCANCODE_MASK) {
        return 0; /* non-printable (arrows, function keys, etc.) */
    }

    return (pnc_u32)keycode;
}

char *pnc_modifier_name(PncModifier modifiers, char *buf, pnc_usize buf_size) {
    static const struct {
        PncModifier bit;
        const char *name;
    } entries[] = {
        { PNC_MOD_LCTRL,  "LCtrl" },  { PNC_MOD_RCTRL,  "RCtrl" },
        { PNC_MOD_LSHIFT, "LShift" }, { PNC_MOD_RSHIFT, "RShift" },
        { PNC_MOD_LALT,   "LAlt" },   { PNC_MOD_RALT,   "RAlt" },
        { PNC_MOD_LSUPER, "LSuper" }, { PNC_MOD_RSUPER, "RSuper" },
        { PNC_MOD_CAPS,   "Caps" },   { PNC_MOD_NUM,    "Num" },
    };
    pnc_usize written = 0;
    pnc_usize i;

    if (buf == NULL || buf_size == 0) {
        return buf;
    }

    buf[0] = '\0';

    for (i = 0; i < PNC_ARRAY_LEN(entries); i++) {
        int n;

        if ((modifiers & entries[i].bit) == 0) {
            continue;
        }

        n = snprintf(buf + written, buf_size - written, "%s%s",
                     (written > 0) ? "+" : "", entries[i].name);

        if (n < 0 || (pnc_usize)n >= buf_size - written) {
            break; /* truncated: buffer is already safely terminated */
        }

        written += (pnc_usize)n;
    }

    return buf;
}
