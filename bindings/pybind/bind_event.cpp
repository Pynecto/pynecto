/* event.h's PncEvent union relies on a nested anonymous struct inside an
 * anonymous union — standard C11, but a GNU/MSVC extension in C++ (no
 * standard C++ equivalent). Scoped to just this include so the rest of the
 * file still gets full -Wpedantic coverage. */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

#include <pynecto/event.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "common.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <optional>
#include <string>

namespace py = pybind11;

namespace {

/* PncModifier is `typedef pnc_u32 PncModifier` — a plain integer, not a real
 * C enum — so py::enum_<PncModifier> can't work (no underlying_type). This
 * local enum exists purely so pybind11 has a real enum to bind. Unscoped
 * (not `enum class`): pybind11's arithmetic mode only registers
 * __or__/__and__ when the enum is implicitly convertible to its underlying
 * type, which `enum class` deliberately blocks. */
enum Modifier : pnc_u32 {
    NONE = PNC_MOD_NONE,
    LSHIFT = PNC_MOD_LSHIFT,
    RSHIFT = PNC_MOD_RSHIFT,
    SHIFT = PNC_MOD_SHIFT,
    LCTRL = PNC_MOD_LCTRL,
    RCTRL = PNC_MOD_RCTRL,
    CTRL = PNC_MOD_CTRL,
    LALT = PNC_MOD_LALT,
    RALT = PNC_MOD_RALT,
    ALT = PNC_MOD_ALT,
    LSUPER = PNC_MOD_LSUPER,
    RSUPER = PNC_MOD_RSUPER,
    SUPER = PNC_MOD_SUPER,
    CAPS = PNC_MOD_CAPS,
    NUM = PNC_MOD_NUM,
};

/* -------------------------------------------------------------------- */
/* One small wrapper struct per Python-visible event type. Where the C
 * union shares one struct across several PncEventType values (window
 * sub-events, touch, drop), the wrapper carries a `kind` field (copied
 * from the outer PncEvent.type, not present in the inner C struct) so
 * Python code can discriminate.                                         */
/* -------------------------------------------------------------------- */

struct QuitEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};

struct WindowEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    PncEventType kind;
    pnc_i32 x;
    pnc_i32 y;
    pnc_i32 width;
    pnc_i32 height;
    pnc_f32 dpi_scale;
};

struct KeyEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    PncKey key;
    Modifier modifiers;
    bool repeat;
    bool pressed;
};

struct TextInputEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    std::string text;
};

struct MouseMoveEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    pnc_f32 x;
    pnc_f32 y;
    pnc_f32 dx;
    pnc_f32 dy;
};

struct MouseButtonEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    PncMouseButton button;
    pnc_f32 x;
    pnc_f32 y;
    pnc_u8 clicks;
    bool pressed;
};

struct MouseScrollEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    pnc_f32 dx;
    pnc_f32 dy;
    pnc_f32 x;
    pnc_f32 y;
    bool flipped;
};

struct TouchEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    PncEventType kind;
    pnc_i64 touch_id;
    pnc_i64 finger_id;
    pnc_f32 x;
    pnc_f32 y;
    pnc_f32 dx;
    pnc_f32 dy;
    pnc_f32 pressure;
};

struct DropEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    PncEventType kind;
    pnc_f32 x;
    pnc_f32 y;
    std::optional<std::string> file;
    std::optional<std::string> text;
};

struct UserEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
    pnc_i32 code;
};

/* Header-only event kinds — no dedicated field beyond timestamp/window_id
 * at the C layer, but bound as distinct Python types for clean dispatch. */
struct ClipboardChangedEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};
struct MouseEnterEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};
struct MouseLeaveEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};
struct AppBackgroundEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};
struct AppForegroundEvent {
    pnc_u64 timestamp;
    pnc_u32 window_id;
};

std::optional<std::string> optional_c_str(const char *value) {
    if (value == nullptr) {
        return std::nullopt;
    }
    return std::string(value);
}

py::object translate(const PncEvent &event) {
    switch (event.type) {
    case PNC_EVENT_QUIT:
        return py::cast(QuitEvent{event.quit.timestamp, event.quit.window_id});

    case PNC_EVENT_WINDOW_SHOWN:
    case PNC_EVENT_WINDOW_HIDDEN:
    case PNC_EVENT_WINDOW_MOVED:
    case PNC_EVENT_WINDOW_RESIZED:
    case PNC_EVENT_WINDOW_MINIMIZED:
    case PNC_EVENT_WINDOW_MAXIMIZED:
    case PNC_EVENT_WINDOW_RESTORED:
    case PNC_EVENT_WINDOW_FOCUS_IN:
    case PNC_EVENT_WINDOW_FOCUS_OUT:
    case PNC_EVENT_WINDOW_CLOSE:
    case PNC_EVENT_WINDOW_DPI_CHANGED:
        return py::cast(WindowEvent{event.window.timestamp, event.window.window_id, event.type, event.window.x,
                                     event.window.y, event.window.width, event.window.height,
                                     event.window.dpi_scale});

    case PNC_EVENT_KEY_DOWN:
    case PNC_EVENT_KEY_UP:
        return py::cast(KeyEvent{event.key.timestamp, event.key.window_id, event.key.key,
                                  static_cast<Modifier>(event.key.modifiers), static_cast<bool>(event.key.repeat),
                                  static_cast<bool>(event.key.pressed)});

    case PNC_EVENT_TEXT_INPUT:
        return py::cast(TextInputEvent{event.text.timestamp, event.text.window_id, std::string(event.text.text)});

    case PNC_EVENT_MOUSE_MOVE:
        return py::cast(MouseMoveEvent{event.mouse_move.timestamp, event.mouse_move.window_id, event.mouse_move.x,
                                        event.mouse_move.y, event.mouse_move.dx, event.mouse_move.dy});

    case PNC_EVENT_MOUSE_DOWN:
    case PNC_EVENT_MOUSE_UP:
        return py::cast(MouseButtonEvent{event.mouse_button.timestamp, event.mouse_button.window_id,
                                          event.mouse_button.button, event.mouse_button.x, event.mouse_button.y,
                                          event.mouse_button.clicks, static_cast<bool>(event.mouse_button.pressed)});

    case PNC_EVENT_MOUSE_SCROLL:
        return py::cast(MouseScrollEvent{event.mouse_scroll.timestamp, event.mouse_scroll.window_id,
                                          event.mouse_scroll.dx, event.mouse_scroll.dy, event.mouse_scroll.x,
                                          event.mouse_scroll.y, static_cast<bool>(event.mouse_scroll.flipped)});

    case PNC_EVENT_MOUSE_ENTER:
        return py::cast(MouseEnterEvent{event.timestamp, event.window_id});

    case PNC_EVENT_MOUSE_LEAVE:
        return py::cast(MouseLeaveEvent{event.timestamp, event.window_id});

    case PNC_EVENT_TOUCH_DOWN:
    case PNC_EVENT_TOUCH_UP:
    case PNC_EVENT_TOUCH_MOVE:
        return py::cast(TouchEvent{event.touch.timestamp, event.touch.window_id, event.type, event.touch.touch_id,
                                    event.touch.finger_id, event.touch.x, event.touch.y, event.touch.dx,
                                    event.touch.dy, event.touch.pressure});

    case PNC_EVENT_DROP_FILE:
    case PNC_EVENT_DROP_TEXT:
    case PNC_EVENT_DROP_BEGIN:
    case PNC_EVENT_DROP_COMPLETE:
        return py::cast(DropEvent{event.drop.timestamp, event.drop.window_id, event.type, event.drop.x,
                                   event.drop.y, optional_c_str(event.drop.file), optional_c_str(event.drop.text)});

    case PNC_EVENT_CLIPBOARD_CHANGED:
        return py::cast(ClipboardChangedEvent{event.timestamp, event.window_id});

    case PNC_EVENT_APP_BACKGROUND:
        return py::cast(AppBackgroundEvent{event.timestamp, event.window_id});

    case PNC_EVENT_APP_FOREGROUND:
        return py::cast(AppForegroundEvent{event.timestamp, event.window_id});

    case PNC_EVENT_USER:
        return py::cast(UserEvent{event.user.timestamp, event.user.window_id, event.user.code});

    default:
        return py::none();
    }
}

/* PncModifier, not Modifier: combining flags via `a | b` through pybind11's
 * arithmetic-enum operators yields a plain int, not another Modifier
 * instance, so these must accept both a single enum member and an
 * already-combined int (a Modifier member converts to PncModifier fine via
 * its __index__). Same reasoning as WindowConfig.flags's setter. */
std::string modifier_name_str(PncModifier modifiers) {
    char buf[64];
    pnc_modifier_name(modifiers, buf, sizeof(buf));
    return std::string(buf);
}

pnc_u32 key_to_char(PncKey key, PncModifier modifiers) {
    return pnc_key_to_char(key, modifiers);
}

} // namespace

void bind_event(py::module_ &m) {
    py::enum_<PncEventType>(m, "EventType")
        .value("NONE", PNC_EVENT_NONE)
        .value("QUIT", PNC_EVENT_QUIT)
        .value("APP_BACKGROUND", PNC_EVENT_APP_BACKGROUND)
        .value("APP_FOREGROUND", PNC_EVENT_APP_FOREGROUND)
        .value("WINDOW_SHOWN", PNC_EVENT_WINDOW_SHOWN)
        .value("WINDOW_HIDDEN", PNC_EVENT_WINDOW_HIDDEN)
        .value("WINDOW_MOVED", PNC_EVENT_WINDOW_MOVED)
        .value("WINDOW_RESIZED", PNC_EVENT_WINDOW_RESIZED)
        .value("WINDOW_MINIMIZED", PNC_EVENT_WINDOW_MINIMIZED)
        .value("WINDOW_MAXIMIZED", PNC_EVENT_WINDOW_MAXIMIZED)
        .value("WINDOW_RESTORED", PNC_EVENT_WINDOW_RESTORED)
        .value("WINDOW_FOCUS_IN", PNC_EVENT_WINDOW_FOCUS_IN)
        .value("WINDOW_FOCUS_OUT", PNC_EVENT_WINDOW_FOCUS_OUT)
        .value("WINDOW_CLOSE", PNC_EVENT_WINDOW_CLOSE)
        .value("WINDOW_DPI_CHANGED", PNC_EVENT_WINDOW_DPI_CHANGED)
        .value("KEY_DOWN", PNC_EVENT_KEY_DOWN)
        .value("KEY_UP", PNC_EVENT_KEY_UP)
        .value("TEXT_INPUT", PNC_EVENT_TEXT_INPUT)
        .value("MOUSE_MOVE", PNC_EVENT_MOUSE_MOVE)
        .value("MOUSE_DOWN", PNC_EVENT_MOUSE_DOWN)
        .value("MOUSE_UP", PNC_EVENT_MOUSE_UP)
        .value("MOUSE_SCROLL", PNC_EVENT_MOUSE_SCROLL)
        .value("MOUSE_ENTER", PNC_EVENT_MOUSE_ENTER)
        .value("MOUSE_LEAVE", PNC_EVENT_MOUSE_LEAVE)
        .value("TOUCH_DOWN", PNC_EVENT_TOUCH_DOWN)
        .value("TOUCH_UP", PNC_EVENT_TOUCH_UP)
        .value("TOUCH_MOVE", PNC_EVENT_TOUCH_MOVE)
        .value("DROP_FILE", PNC_EVENT_DROP_FILE)
        .value("DROP_TEXT", PNC_EVENT_DROP_TEXT)
        .value("DROP_BEGIN", PNC_EVENT_DROP_BEGIN)
        .value("DROP_COMPLETE", PNC_EVENT_DROP_COMPLETE)
        .value("CLIPBOARD_CHANGED", PNC_EVENT_CLIPBOARD_CHANGED)
        .value("USER", PNC_EVENT_USER)
        .export_values();

    py::enum_<PncMouseButton>(m, "MouseButton")
        .value("NONE", PNC_MOUSE_BUTTON_NONE)
        .value("LEFT", PNC_MOUSE_BUTTON_LEFT)
        .value("MIDDLE", PNC_MOUSE_BUTTON_MIDDLE)
        .value("RIGHT", PNC_MOUSE_BUTTON_RIGHT)
        .value("X1", PNC_MOUSE_BUTTON_X1)
        .value("X2", PNC_MOUSE_BUTTON_X2)
        .export_values();

    py::enum_<Modifier>(m, "Modifier", py::arithmetic())
        .value("NONE", Modifier::NONE)
        .value("LSHIFT", Modifier::LSHIFT)
        .value("RSHIFT", Modifier::RSHIFT)
        .value("SHIFT", Modifier::SHIFT)
        .value("LCTRL", Modifier::LCTRL)
        .value("RCTRL", Modifier::RCTRL)
        .value("CTRL", Modifier::CTRL)
        .value("LALT", Modifier::LALT)
        .value("RALT", Modifier::RALT)
        .value("ALT", Modifier::ALT)
        .value("LSUPER", Modifier::LSUPER)
        .value("RSUPER", Modifier::RSUPER)
        .value("SUPER", Modifier::SUPER)
        .value("CAPS", Modifier::CAPS)
        .value("NUM", Modifier::NUM)
        .export_values();

    py::enum_<PncKey> key(m, "Key");
    key.value("UNKNOWN", PNC_KEY_UNKNOWN);
    key.value("A", PNC_KEY_A);
    key.value("B", PNC_KEY_B);
    key.value("C", PNC_KEY_C);
    key.value("D", PNC_KEY_D);
    key.value("E", PNC_KEY_E);
    key.value("F", PNC_KEY_F);
    key.value("G", PNC_KEY_G);
    key.value("H", PNC_KEY_H);
    key.value("I", PNC_KEY_I);
    key.value("J", PNC_KEY_J);
    key.value("K", PNC_KEY_K);
    key.value("L", PNC_KEY_L);
    key.value("M", PNC_KEY_M);
    key.value("N", PNC_KEY_N);
    key.value("O", PNC_KEY_O);
    key.value("P", PNC_KEY_P);
    key.value("Q", PNC_KEY_Q);
    key.value("R", PNC_KEY_R);
    key.value("S", PNC_KEY_S);
    key.value("T", PNC_KEY_T);
    key.value("U", PNC_KEY_U);
    key.value("V", PNC_KEY_V);
    key.value("W", PNC_KEY_W);
    key.value("X", PNC_KEY_X);
    key.value("Y", PNC_KEY_Y);
    key.value("Z", PNC_KEY_Z);
    key.value("NUM_1", PNC_KEY_1);
    key.value("NUM_2", PNC_KEY_2);
    key.value("NUM_3", PNC_KEY_3);
    key.value("NUM_4", PNC_KEY_4);
    key.value("NUM_5", PNC_KEY_5);
    key.value("NUM_6", PNC_KEY_6);
    key.value("NUM_7", PNC_KEY_7);
    key.value("NUM_8", PNC_KEY_8);
    key.value("NUM_9", PNC_KEY_9);
    key.value("NUM_0", PNC_KEY_0);
    key.value("RETURN", PNC_KEY_RETURN);
    key.value("ESCAPE", PNC_KEY_ESCAPE);
    key.value("BACKSPACE", PNC_KEY_BACKSPACE);
    key.value("TAB", PNC_KEY_TAB);
    key.value("SPACE", PNC_KEY_SPACE);
    key.value("DELETE", PNC_KEY_DELETE);
    key.value("INSERT", PNC_KEY_INSERT);
    key.value("HOME", PNC_KEY_HOME);
    key.value("END", PNC_KEY_END);
    key.value("PAGE_UP", PNC_KEY_PAGE_UP);
    key.value("PAGE_DOWN", PNC_KEY_PAGE_DOWN);
    key.value("CAPS_LOCK", PNC_KEY_CAPS_LOCK);
    key.value("PRINT_SCREEN", PNC_KEY_PRINT_SCREEN);
    key.value("SCROLL_LOCK", PNC_KEY_SCROLL_LOCK);
    key.value("PAUSE", PNC_KEY_PAUSE);
    key.value("RIGHT", PNC_KEY_RIGHT);
    key.value("LEFT", PNC_KEY_LEFT);
    key.value("DOWN", PNC_KEY_DOWN);
    key.value("UP", PNC_KEY_UP);
    key.value("F1", PNC_KEY_F1);
    key.value("F2", PNC_KEY_F2);
    key.value("F3", PNC_KEY_F3);
    key.value("F4", PNC_KEY_F4);
    key.value("F5", PNC_KEY_F5);
    key.value("F6", PNC_KEY_F6);
    key.value("F7", PNC_KEY_F7);
    key.value("F8", PNC_KEY_F8);
    key.value("F9", PNC_KEY_F9);
    key.value("F10", PNC_KEY_F10);
    key.value("F11", PNC_KEY_F11);
    key.value("F12", PNC_KEY_F12);
    key.value("LSHIFT", PNC_KEY_LSHIFT);
    key.value("RSHIFT", PNC_KEY_RSHIFT);
    key.value("LCTRL", PNC_KEY_LCTRL);
    key.value("RCTRL", PNC_KEY_RCTRL);
    key.value("LALT", PNC_KEY_LALT);
    key.value("RALT", PNC_KEY_RALT);
    key.value("LSUPER", PNC_KEY_LSUPER);
    key.value("RSUPER", PNC_KEY_RSUPER);
    key.value("MINUS", PNC_KEY_MINUS);
    key.value("EQUALS", PNC_KEY_EQUALS);
    key.value("LEFT_BRACKET", PNC_KEY_LEFT_BRACKET);
    key.value("RIGHT_BRACKET", PNC_KEY_RIGHT_BRACKET);
    key.value("BACKSLASH", PNC_KEY_BACKSLASH);
    key.value("SEMICOLON", PNC_KEY_SEMICOLON);
    key.value("APOSTROPHE", PNC_KEY_APOSTROPHE);
    key.value("GRAVE", PNC_KEY_GRAVE);
    key.value("COMMA", PNC_KEY_COMMA);
    key.value("PERIOD", PNC_KEY_PERIOD);
    key.value("SLASH", PNC_KEY_SLASH);
    key.value("NUM_LOCK", PNC_KEY_NUM_LOCK);
    key.value("KP_DIVIDE", PNC_KEY_KP_DIVIDE);
    key.value("KP_MULTIPLY", PNC_KEY_KP_MULTIPLY);
    key.value("KP_MINUS", PNC_KEY_KP_MINUS);
    key.value("KP_PLUS", PNC_KEY_KP_PLUS);
    key.value("KP_ENTER", PNC_KEY_KP_ENTER);
    key.value("KP_1", PNC_KEY_KP_1);
    key.value("KP_2", PNC_KEY_KP_2);
    key.value("KP_3", PNC_KEY_KP_3);
    key.value("KP_4", PNC_KEY_KP_4);
    key.value("KP_5", PNC_KEY_KP_5);
    key.value("KP_6", PNC_KEY_KP_6);
    key.value("KP_7", PNC_KEY_KP_7);
    key.value("KP_8", PNC_KEY_KP_8);
    key.value("KP_9", PNC_KEY_KP_9);
    key.value("KP_0", PNC_KEY_KP_0);
    key.value("KP_PERIOD", PNC_KEY_KP_PERIOD);
    key.export_values();

    py::class_<QuitEvent>(m, "QuitEvent")
        .def_readonly("timestamp", &QuitEvent::timestamp)
        .def_readonly("window_id", &QuitEvent::window_id);

    py::class_<WindowEvent>(m, "WindowEvent")
        .def_readonly("timestamp", &WindowEvent::timestamp)
        .def_readonly("window_id", &WindowEvent::window_id)
        .def_readonly("kind", &WindowEvent::kind)
        .def_readonly("x", &WindowEvent::x)
        .def_readonly("y", &WindowEvent::y)
        .def_readonly("width", &WindowEvent::width)
        .def_readonly("height", &WindowEvent::height)
        .def_readonly("dpi_scale", &WindowEvent::dpi_scale);

    py::class_<KeyEvent>(m, "KeyEvent")
        .def_readonly("timestamp", &KeyEvent::timestamp)
        .def_readonly("window_id", &KeyEvent::window_id)
        .def_readonly("key", &KeyEvent::key)
        .def_readonly("modifiers", &KeyEvent::modifiers)
        .def_readonly("repeat", &KeyEvent::repeat)
        .def_readonly("pressed", &KeyEvent::pressed);

    py::class_<TextInputEvent>(m, "TextInputEvent")
        .def_readonly("timestamp", &TextInputEvent::timestamp)
        .def_readonly("window_id", &TextInputEvent::window_id)
        .def_readonly("text", &TextInputEvent::text);

    py::class_<MouseMoveEvent>(m, "MouseMoveEvent")
        .def_readonly("timestamp", &MouseMoveEvent::timestamp)
        .def_readonly("window_id", &MouseMoveEvent::window_id)
        .def_readonly("x", &MouseMoveEvent::x)
        .def_readonly("y", &MouseMoveEvent::y)
        .def_readonly("dx", &MouseMoveEvent::dx)
        .def_readonly("dy", &MouseMoveEvent::dy);

    py::class_<MouseButtonEvent>(m, "MouseButtonEvent")
        .def_readonly("timestamp", &MouseButtonEvent::timestamp)
        .def_readonly("window_id", &MouseButtonEvent::window_id)
        .def_readonly("button", &MouseButtonEvent::button)
        .def_readonly("x", &MouseButtonEvent::x)
        .def_readonly("y", &MouseButtonEvent::y)
        .def_readonly("clicks", &MouseButtonEvent::clicks)
        .def_readonly("pressed", &MouseButtonEvent::pressed);

    py::class_<MouseScrollEvent>(m, "MouseScrollEvent")
        .def_readonly("timestamp", &MouseScrollEvent::timestamp)
        .def_readonly("window_id", &MouseScrollEvent::window_id)
        .def_readonly("dx", &MouseScrollEvent::dx)
        .def_readonly("dy", &MouseScrollEvent::dy)
        .def_readonly("x", &MouseScrollEvent::x)
        .def_readonly("y", &MouseScrollEvent::y)
        .def_readonly("flipped", &MouseScrollEvent::flipped);

    py::class_<MouseEnterEvent>(m, "MouseEnterEvent")
        .def_readonly("timestamp", &MouseEnterEvent::timestamp)
        .def_readonly("window_id", &MouseEnterEvent::window_id);

    py::class_<MouseLeaveEvent>(m, "MouseLeaveEvent")
        .def_readonly("timestamp", &MouseLeaveEvent::timestamp)
        .def_readonly("window_id", &MouseLeaveEvent::window_id);

    py::class_<TouchEvent>(m, "TouchEvent")
        .def_readonly("timestamp", &TouchEvent::timestamp)
        .def_readonly("window_id", &TouchEvent::window_id)
        .def_readonly("kind", &TouchEvent::kind)
        .def_readonly("touch_id", &TouchEvent::touch_id)
        .def_readonly("finger_id", &TouchEvent::finger_id)
        .def_readonly("x", &TouchEvent::x)
        .def_readonly("y", &TouchEvent::y)
        .def_readonly("dx", &TouchEvent::dx)
        .def_readonly("dy", &TouchEvent::dy)
        .def_readonly("pressure", &TouchEvent::pressure);

    py::class_<DropEvent>(m, "DropEvent")
        .def_readonly("timestamp", &DropEvent::timestamp)
        .def_readonly("window_id", &DropEvent::window_id)
        .def_readonly("kind", &DropEvent::kind)
        .def_readonly("x", &DropEvent::x)
        .def_readonly("y", &DropEvent::y)
        .def_readonly("file", &DropEvent::file)
        .def_readonly("text", &DropEvent::text);

    py::class_<ClipboardChangedEvent>(m, "ClipboardChangedEvent")
        .def_readonly("timestamp", &ClipboardChangedEvent::timestamp)
        .def_readonly("window_id", &ClipboardChangedEvent::window_id);

    py::class_<AppBackgroundEvent>(m, "AppBackgroundEvent")
        .def_readonly("timestamp", &AppBackgroundEvent::timestamp)
        .def_readonly("window_id", &AppBackgroundEvent::window_id);

    py::class_<AppForegroundEvent>(m, "AppForegroundEvent")
        .def_readonly("timestamp", &AppForegroundEvent::timestamp)
        .def_readonly("window_id", &AppForegroundEvent::window_id);

    py::class_<UserEvent>(m, "UserEvent")
        .def_readonly("timestamp", &UserEvent::timestamp)
        .def_readonly("window_id", &UserEvent::window_id)
        .def_readonly("code", &UserEvent::code);

    m.def("poll_event", []() -> py::object {
        PncEvent event;
        if (!pnc_event_poll(&event)) {
            return py::none();
        }
        return translate(event);
    });

    m.def(
        "wait_event",
        [](pnc_u32 timeout_ms) -> py::object {
            PncEvent event;
            bool got;
            {
                py::gil_scoped_release release;
                got = pnc_event_wait(&event, timeout_ms);
            }
            if (!got) {
                return py::none();
            }
            return translate(event);
        },
        py::arg("timeout_ms"));

    m.def("peek_event", []() -> py::object {
        PncEvent event;
        if (!pnc_event_peek(&event)) {
            return py::none();
        }
        return translate(event);
    });

    m.def(
        "push_user_event",
        [](pnc_i32 code) {
            PncEvent event;
            event.type = PNC_EVENT_USER;
            event.user.timestamp = 0;
            event.user.window_id = 0;
            event.user.code = code;
            event.user.data1 = nullptr;
            event.user.data2 = nullptr;
            pnc::check(pnc_event_push(&event));
        },
        py::arg("code"));

    m.def("flush_events", &pnc_event_flush, py::arg("event_type"));
    m.def("key_name", &pnc_key_name, py::arg("key"));
    m.def("key_to_char", &key_to_char, py::arg("key"), py::arg("modifiers"));
    m.def("modifier_name", &modifier_name_str, py::arg("modifiers"));
}
