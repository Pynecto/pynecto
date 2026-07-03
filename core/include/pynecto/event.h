#ifndef PYNECTO_EVENT_H
#define PYNECTO_EVENT_H
#include <pynecto/platform.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum PncEventType {

    PNC_EVENT_NONE = 0,

    PNC_EVENT_QUIT = 1,
    PNC_EVENT_APP_BACKGROUND = 2,
    PNC_EVENT_APP_FOREGROUND = 3,

    PNC_EVENT_WINDOW_SHOWN = 100,
    PNC_EVENT_WINDOW_HIDDEN = 101,
    PNC_EVENT_WINDOW_MOVED = 102,
    PNC_EVENT_WINDOW_RESIZED = 103,
    PNC_EVENT_WINDOW_MINIMIZED = 104,
    PNC_EVENT_WINDOW_MAXIMIZED = 105,
    PNC_EVENT_WINDOW_RESTORED = 106,
    PNC_EVENT_WINDOW_FOCUS_IN = 107,
    PNC_EVENT_WINDOW_FOCUS_OUT = 108,
    PNC_EVENT_WINDOW_CLOSE = 109,
    PNC_EVENT_WINDOW_DPI_CHANGED = 110,

    PNC_EVENT_KEY_DOWN = 200,
    PNC_EVENT_KEY_UP = 201,
    PNC_EVENT_TEXT_INPUT = 202,

    PNC_EVENT_MOUSE_MOVE = 300,
    PNC_EVENT_MOUSE_DOWN = 301,
    PNC_EVENT_MOUSE_UP = 302,
    PNC_EVENT_MOUSE_SCROLL = 303,
    PNC_EVENT_MOUSE_ENTER = 304,
    PNC_EVENT_MOUSE_LEAVE = 305,

    PNC_EVENT_TOUCH_DOWN = 400,
    PNC_EVENT_TOUCH_UP = 401,
    PNC_EVENT_TOUCH_MOVE = 402,

    PNC_EVENT_DROP_FILE = 500,
    PNC_EVENT_DROP_TEXT = 501,
    PNC_EVENT_DROP_BEGIN = 502,
    PNC_EVENT_DROP_COMPLETE = 503,

    PNC_EVENT_CLIPBOARD_CHANGED = 600,

    PNC_EVENT_USER = 1000,

} PncEventType;



typedef enum PncKey {
    PNC_KEY_UNKNOWN = 0,

    PNC_KEY_A = 4,
    PNC_KEY_B = 5,
    PNC_KEY_C = 6,
    PNC_KEY_D = 7,
    PNC_KEY_E = 8,
    PNC_KEY_F = 9,
    PNC_KEY_G = 10,
    PNC_KEY_H = 11,
    PNC_KEY_I = 12,
    PNC_KEY_J = 13,
    PNC_KEY_K = 14,
    PNC_KEY_L = 15,
    PNC_KEY_M = 16,
    PNC_KEY_N = 17,
    PNC_KEY_O = 18,
    PNC_KEY_P = 19,
    PNC_KEY_Q = 20,
    PNC_KEY_R = 21,
    PNC_KEY_S = 22,
    PNC_KEY_T = 23,
    PNC_KEY_U = 24,
    PNC_KEY_V = 25,
    PNC_KEY_W = 26,
    PNC_KEY_X = 27,
    PNC_KEY_Y = 28,
    PNC_KEY_Z = 29,

    PNC_KEY_1 = 30,
    PNC_KEY_2 = 31,
    PNC_KEY_3 = 32,
    PNC_KEY_4 = 33,
    PNC_KEY_5 = 34,
    PNC_KEY_6 = 35,
    PNC_KEY_7 = 36,
    PNC_KEY_8 = 37,
    PNC_KEY_9 = 38,
    PNC_KEY_0 = 39,

    PNC_KEY_RETURN = 40,
    PNC_KEY_ESCAPE = 41,
    PNC_KEY_BACKSPACE = 42,
    PNC_KEY_TAB = 43,
    PNC_KEY_SPACE = 44,
    PNC_KEY_DELETE = 76,
    PNC_KEY_INSERT = 73,
    PNC_KEY_HOME = 74,
    PNC_KEY_END = 77,
    PNC_KEY_PAGE_UP = 75,
    PNC_KEY_PAGE_DOWN = 78,
    PNC_KEY_CAPS_LOCK = 57,
    PNC_KEY_PRINT_SCREEN = 70,
    PNC_KEY_SCROLL_LOCK = 71,
    PNC_KEY_PAUSE = 72,

    PNC_KEY_RIGHT = 79,
    PNC_KEY_LEFT = 80,
    PNC_KEY_DOWN = 81,
    PNC_KEY_UP = 82,

    PNC_KEY_F1 = 58,
    PNC_KEY_F2 = 59,
    PNC_KEY_F3 = 60,
    PNC_KEY_F4 = 61,
    PNC_KEY_F5 = 62,
    PNC_KEY_F6 = 63,
    PNC_KEY_F7 = 64,
    PNC_KEY_F8 = 65,
    PNC_KEY_F9 = 66,
    PNC_KEY_F10 = 67,
    PNC_KEY_F11 = 68,
    PNC_KEY_F12 = 69,

    PNC_KEY_LSHIFT = 225,
    PNC_KEY_RSHIFT = 229,
    PNC_KEY_LCTRL = 224,
    PNC_KEY_RCTRL = 228,
    PNC_KEY_LALT = 226,
    PNC_KEY_RALT = 230,
    PNC_KEY_LSUPER = 227,
    PNC_KEY_RSUPER = 231,

    PNC_KEY_MINUS = 45,
    PNC_KEY_EQUALS = 46,
    PNC_KEY_LEFT_BRACKET = 47,
    PNC_KEY_RIGHT_BRACKET = 48,
    PNC_KEY_BACKSLASH = 49,
    PNC_KEY_SEMICOLON = 51,
    PNC_KEY_APOSTROPHE = 52,
    PNC_KEY_GRAVE = 53,
    PNC_KEY_COMMA = 54,
    PNC_KEY_PERIOD = 55,
    PNC_KEY_SLASH = 56,

    PNC_KEY_NUM_LOCK = 83,
    PNC_KEY_KP_DIVIDE = 84,
    PNC_KEY_KP_MULTIPLY = 85,
    PNC_KEY_KP_MINUS = 86,
    PNC_KEY_KP_PLUS = 87,
    PNC_KEY_KP_ENTER = 88,
    PNC_KEY_KP_1 = 89,
    PNC_KEY_KP_2 = 90,
    PNC_KEY_KP_3 = 91,
    PNC_KEY_KP_4 = 92,
    PNC_KEY_KP_5 = 93,
    PNC_KEY_KP_6 = 94,
    PNC_KEY_KP_7 = 95,
    PNC_KEY_KP_8 = 96,
    PNC_KEY_KP_9 = 97,
    PNC_KEY_KP_0 = 98,
    PNC_KEY_KP_PERIOD = 99,

    PNC_KEY_COUNT = 512

} PncKey;



typedef pnc_u32 PncModifier;

#define PNC_MOD_NONE    ((PncModifier)0x0000)
#define PNC_MOD_LSHIFT  ((PncModifier)0x0001)
#define PNC_MOD_RSHIFT  ((PncModifier)0x0002)
#define PNC_MOD_SHIFT   ((PncModifier)0x0003)
#define PNC_MOD_LCTRL   ((PncModifier)0x0040)
#define PNC_MOD_RCTRL   ((PncModifier)0x0080)
#define PNC_MOD_CTRL    ((PncModifier)0x00C0)
#define PNC_MOD_LALT    ((PncModifier)0x0100)
#define PNC_MOD_RALT    ((PncModifier)0x0200)
#define PNC_MOD_ALT     ((PncModifier)0x0300)
#define PNC_MOD_LSUPER  ((PncModifier)0x0400)
#define PNC_MOD_RSUPER  ((PncModifier)0x0800)
#define PNC_MOD_SUPER   ((PncModifier)0x0C00)
#define PNC_MOD_CAPS    ((PncModifier)0x2000)
#define PNC_MOD_NUM     ((PncModifier)0x4000)

typedef enum PncMouseButton {
    PNC_MOUSE_BUTTON_NONE = 0,
    PNC_MOUSE_BUTTON_LEFT = 1,
    PNC_MOUSE_BUTTON_MIDDLE = 2,
    PNC_MOUSE_BUTTON_RIGHT = 3,
    PNC_MOUSE_BUTTON_X1 = 4,
    PNC_MOUSE_BUTTON_X2 = 5,
} PncMouseButton;

#define PNC_EVENT_HEADER        \
    pnc_u64      timestamp;     \
    pnc_u32      window_id;

typedef struct PncQuitEvent {
    PNC_EVENT_HEADER
} PncQuitEvent;

/* --- Window -------------------------------------------------------------- */

typedef struct PncWindowEvent {
    PNC_EVENT_HEADER
    pnc_i32   x;
    pnc_i32   y;
    pnc_i32   width;
    pnc_i32   height;
    pnc_f32   dpi_scale;
} PncWindowEvent;

typedef struct PncKeyEvent {
    PNC_EVENT_HEADER
    PncKey       key;
    PncModifier  modifiers;
    pnc_bool     repeat;
    pnc_bool     pressed;
} PncKeyEvent;

typedef struct PncTextInputEvent {
    PNC_EVENT_HEADER
    char         text[32];
} PncTextInputEvent;

typedef struct PncMouseMoveEvent {
    PNC_EVENT_HEADER
    pnc_f32   x;
    pnc_f32   y;
    pnc_f32   dx;
    pnc_f32   dy;
} PncMouseMoveEvent;

typedef struct PncMouseButtonEvent {
    PNC_EVENT_HEADER
    PncMouseButton  button;
    pnc_f32         x;
    pnc_f32         y;
    pnc_u8          clicks;
    pnc_bool        pressed;
} PncMouseButtonEvent;

typedef struct PncMouseScrollEvent {
    PNC_EVENT_HEADER
    pnc_f32   dx;
    pnc_f32   dy;
    pnc_f32   x;
    pnc_f32   y;
    pnc_bool  flipped;
} PncMouseScrollEvent;

typedef struct PncTouchEvent {
    PNC_EVENT_HEADER
    pnc_i64   touch_id;
    pnc_i64   finger_id;
    pnc_f32   x;
    pnc_f32   y;
    pnc_f32   dx;
    pnc_f32   dy;
    pnc_f32   pressure;
} PncTouchEvent;

typedef struct PncDropEvent {
    PNC_EVENT_HEADER
    const char  *file;
    const char  *text;
    pnc_f32      x;
    pnc_f32      y;
} PncDropEvent;

typedef struct PncUserEvent {
    PNC_EVENT_HEADER
    pnc_i32   code;
    void     *data1;
    void     *data2;
} PncUserEvent;



typedef struct PncEvent {
    PncEventType type;

    union {
        struct {
            pnc_u64  timestamp;
            pnc_u32  window_id;
        };

        PncQuitEvent         quit;
        PncWindowEvent       window;
        PncKeyEvent          key;
        PncTextInputEvent    text;
        PncMouseMoveEvent    mouse_move;
        PncMouseButtonEvent  mouse_button;
        PncMouseScrollEvent  mouse_scroll;
        PncTouchEvent        touch;
        PncDropEvent         drop;
        PncUserEvent         user;

        pnc_u8 _padding[128];
    };

} PncEvent;

/* type (4 bytes) is padded to 8 for the union's 8-byte alignment
 * (pnc_u64 / pointer members), so the layout is 8 + 128, not 4 + 128. */
PNC_STATIC_ASSERT(sizeof(PncEvent) == 136,
    "PncEvent size contract broken");

PNC_API pnc_bool pnc_event_poll(PncEvent *event);

PNC_API pnc_bool pnc_event_wait(PncEvent *event, pnc_u32 timeout);

PNC_API pnc_bool pnc_event_push(const PncEvent *event);

PNC_API void pnc_event_flush(PncEventType type);

PNC_API pnc_bool pnc_event_peek(PncEvent *event);



PNC_API const char *pnc_key_name(PncKey key);

PNC_API pnc_u32 pnc_key_to_char(PncKey key, PncModifier modifiers);

PNC_API char *pnc_modifier_name(PncModifier modifiers, char *buf, pnc_usize buf_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PYNECTO_EVENT_H */