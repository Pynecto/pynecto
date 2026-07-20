#include <pynecto/window.h>

#include "common.h"

#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

namespace {

/* PncWindowConfig.title is a raw const char*. A plain def_readwrite would let
 * it dangle once the Python string that set it is garbage-collected, so this
 * wrapper also owns the backing storage and repoints title at it. */
struct WindowConfigWrapper : PncWindowConfig {
    std::string title_storage;

    WindowConfigWrapper() : PncWindowConfig(pnc_window_config_default()) {
        title_storage = (title != nullptr) ? title : "";
        title = title_storage.c_str();
    }

    void set_title(const std::string &new_title) {
        title_storage = new_title;
        title = title_storage.c_str();
    }

    std::string get_title() const {
        return title_storage;
    }
};

class Window {
public:
    explicit Window(const WindowConfigWrapper &config) {
        handle_ = pnc_window_create(&config);
        pnc::check_ptr(handle_);
    }

    ~Window() {
        close();
    }

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    void close() {
        if (handle_ != nullptr) {
            pnc_window_destroy(handle_);
            handle_ = nullptr;
        }
    }

    void show() {
        ensure_open();
        pnc_window_show(handle_);
    }

    void hide() {
        ensure_open();
        pnc_window_hide(handle_);
    }

    void set_title(const std::string &title) {
        ensure_open();
        pnc_window_set_title(handle_, title.c_str());
    }

    std::string get_title() {
        ensure_open();
        return pnc_window_get_title(handle_);
    }

    void set_size(pnc_i32 width, pnc_i32 height) {
        ensure_open();
        pnc::check(pnc_window_set_size(handle_, width, height));
    }

    void set_position(pnc_i32 x, pnc_i32 y) {
        ensure_open();
        pnc_window_set_position(handle_, x, y);
    }

    void set_min_size(pnc_i32 min_w, pnc_i32 min_h) {
        ensure_open();
        pnc_window_set_min_size(handle_, min_w, min_h);
    }

    void set_max_size(pnc_i32 max_w, pnc_i32 max_h) {
        ensure_open();
        pnc_window_set_max_size(handle_, max_w, max_h);
    }

    void set_fullscreen(bool fullscreen) {
        ensure_open();
        pnc::check(pnc_window_set_fullscreen(handle_, fullscreen));
    }

    void set_resizable(bool resizable) {
        ensure_open();
        pnc_window_set_resizable(handle_, resizable);
    }

    void set_opacity(pnc_f32 opacity) {
        ensure_open();
        pnc::check(pnc_window_set_opacity(handle_, opacity));
    }

    void focus() {
        ensure_open();
        pnc_window_focus(handle_);
    }

    void maximize() {
        ensure_open();
        pnc_window_maximize(handle_);
    }

    void minimize() {
        ensure_open();
        pnc_window_minimize(handle_);
    }

    void restore() {
        ensure_open();
        pnc_window_restore(handle_);
    }

    PncWindowState get_state() {
        ensure_open();
        PncWindowState state;
        pnc_window_get_state(handle_, &state);
        return state;
    }

    pnc_u32 get_id() {
        ensure_open();
        return pnc_window_get_id(handle_);
    }

    pnc_i32 get_width() {
        ensure_open();
        return pnc_window_get_width(handle_);
    }

    pnc_i32 get_height() {
        ensure_open();
        return pnc_window_get_height(handle_);
    }

    pnc_f32 get_dpi_scale() {
        ensure_open();
        return pnc_window_get_dpi_scale(handle_);
    }

    bool is_focused() {
        ensure_open();
        return pnc_window_is_focused(handle_);
    }

    bool is_minimized() {
        ensure_open();
        return pnc_window_is_minimized(handle_);
    }

    bool is_maximized() {
        ensure_open();
        return pnc_window_is_maximized(handle_);
    }

    bool is_fullscreen() {
        ensure_open();
        return pnc_window_is_fullscreen(handle_);
    }

    bool is_visible() {
        ensure_open();
        return pnc_window_is_visible(handle_);
    }

    void gl_make_current() {
        ensure_open();
        pnc::check(pnc_window_gl_make_current(handle_));
    }

    void gl_swap_buffers() {
        ensure_open();
        PncWindow *handle = handle_;
        py::gil_scoped_release release;
        pnc_window_gl_swap_buffers(handle);
    }

    void gl_set_swap_interval(pnc_i32 interval) {
        ensure_open();
        pnc::check(pnc_window_gl_set_swap_interval(handle_, interval));
    }

private:
    void ensure_open() const {
        if (handle_ == nullptr) {
            throw pnc::PncError("window is closed");
        }
    }

    PncWindow *handle_ = nullptr;
};

/* PncWindowFlags is `typedef pnc_u32 PncWindowFlags` — a plain integer, not a
 * real C enum — so py::enum_<PncWindowFlags> can't work (no underlying_type).
 * This local enum exists purely so pybind11 has a real enum to bind.
 * Unscoped (not `enum class`): pybind11's arithmetic mode only registers
 * __or__/__and__ when the enum is implicitly convertible to its underlying
 * type, which `enum class` deliberately blocks. WindowConfigWrapper::flags
 * itself stays a plain pnc_u32 field. */
enum WindowFlags : pnc_u32 {
    NONE = PNC_WINDOW_NONE,
    RESIZABLE = PNC_WINDOW_RESIZABLE,
    BORDERLESS = PNC_WINDOW_BORDERLESS,
    FULLSCREEN = PNC_WINDOW_FULLSCREEN,
    MAXIMIZED = PNC_WINDOW_MAXIMIZED,
    MINIMIZED = PNC_WINDOW_MINIMIZED,
    HIGH_DPI = PNC_WINDOW_HIGH_DPI,
    ALWAYS_ON_TOP = PNC_WINDOW_ALWAYS_ON_TOP,
    HIDDEN = PNC_WINDOW_HIDDEN,
    OPENGL = PNC_WINDOW_OPENGL,
    VULKAN = PNC_WINDOW_VULKAN,
};

} // namespace

void bind_window(py::module_ &m) {
    py::enum_<WindowFlags>(m, "WindowFlags", py::arithmetic())
        .value("NONE", WindowFlags::NONE)
        .value("RESIZABLE", WindowFlags::RESIZABLE)
        .value("BORDERLESS", WindowFlags::BORDERLESS)
        .value("FULLSCREEN", WindowFlags::FULLSCREEN)
        .value("MAXIMIZED", WindowFlags::MAXIMIZED)
        .value("MINIMIZED", WindowFlags::MINIMIZED)
        .value("HIGH_DPI", WindowFlags::HIGH_DPI)
        .value("ALWAYS_ON_TOP", WindowFlags::ALWAYS_ON_TOP)
        .value("HIDDEN", WindowFlags::HIDDEN)
        .value("OPENGL", WindowFlags::OPENGL)
        .value("VULKAN", WindowFlags::VULKAN)
        .export_values();

    py::class_<WindowConfigWrapper>(m, "WindowConfig")
        .def(py::init<>())
        .def_property("title", &WindowConfigWrapper::get_title, &WindowConfigWrapper::set_title)
        .def_readwrite("width", &WindowConfigWrapper::width)
        .def_readwrite("height", &WindowConfigWrapper::height)
        .def_readwrite("min_width", &WindowConfigWrapper::min_width)
        .def_readwrite("min_height", &WindowConfigWrapper::min_height)
        .def_readwrite("max_width", &WindowConfigWrapper::max_width)
        .def_readwrite("max_height", &WindowConfigWrapper::max_height)
        .def_readwrite("x", &WindowConfigWrapper::x)
        .def_readwrite("y", &WindowConfigWrapper::y)
        .def_property(
            "flags",
            [](const WindowConfigWrapper &self) { return static_cast<WindowFlags>(self.flags); },
            /* pnc_u32, not WindowFlags: combining flags via `a | b` through
             * pybind11's arithmetic-enum operators yields a plain int, not
             * another WindowFlags instance, so the setter must accept both a
             * single enum member and an already-combined int (a WindowFlags
             * member converts to pnc_u32 fine via its __index__). */
            [](WindowConfigWrapper &self, pnc_u32 value) { self.flags = value; })
        .def_readwrite("display_index", &WindowConfigWrapper::display_index)
        .def_readwrite("gl_major", &WindowConfigWrapper::gl_major)
        .def_readwrite("gl_minor", &WindowConfigWrapper::gl_minor)
        .def_readwrite("gl_debug", &WindowConfigWrapper::gl_debug)
        .def_readwrite("clear_color_r", &WindowConfigWrapper::clear_color_r)
        .def_readwrite("clear_color_g", &WindowConfigWrapper::clear_color_g)
        .def_readwrite("clear_color_b", &WindowConfigWrapper::clear_color_b)
        .def_readwrite("clear_color_a", &WindowConfigWrapper::clear_color_a);

    py::class_<PncWindowState>(m, "WindowState")
        .def_readonly("width", &PncWindowState::width)
        .def_readonly("height", &PncWindowState::height)
        .def_readonly("width_px", &PncWindowState::width_px)
        .def_readonly("height_px", &PncWindowState::height_px)
        .def_readonly("x", &PncWindowState::x)
        .def_readonly("y", &PncWindowState::y)
        .def_readonly("dpi_scale", &PncWindowState::dpi_scale)
        .def_readonly("focused", &PncWindowState::focused)
        .def_readonly("minimized", &PncWindowState::minimized)
        .def_readonly("maximized", &PncWindowState::maximized)
        .def_readonly("fullscreen", &PncWindowState::fullscreen)
        .def_readonly("visible", &PncWindowState::visible)
        .def_readonly("mouse_over", &PncWindowState::mouse_over);

    py::class_<Window>(m, "Window")
        .def(py::init<const WindowConfigWrapper &>(), py::arg("config"))
        .def("show", &Window::show)
        .def("hide", &Window::hide)
        .def_property("title", &Window::get_title, &Window::set_title)
        .def("set_size", &Window::set_size, py::arg("width"), py::arg("height"))
        .def("set_position", &Window::set_position, py::arg("x"), py::arg("y"))
        .def("set_min_size", &Window::set_min_size, py::arg("min_width"), py::arg("min_height"))
        .def("set_max_size", &Window::set_max_size, py::arg("max_width"), py::arg("max_height"))
        .def("set_fullscreen", &Window::set_fullscreen, py::arg("fullscreen"))
        .def("set_resizable", &Window::set_resizable, py::arg("resizable"))
        .def("set_opacity", &Window::set_opacity, py::arg("opacity"))
        .def("focus", &Window::focus)
        .def("maximize", &Window::maximize)
        .def("minimize", &Window::minimize)
        .def("restore", &Window::restore)
        .def("get_state", &Window::get_state)
        .def("get_id", &Window::get_id)
        .def("get_width", &Window::get_width)
        .def("get_height", &Window::get_height)
        .def("get_dpi_scale", &Window::get_dpi_scale)
        .def("is_focused", &Window::is_focused)
        .def("is_minimized", &Window::is_minimized)
        .def("is_maximized", &Window::is_maximized)
        .def("is_fullscreen", &Window::is_fullscreen)
        .def("is_visible", &Window::is_visible)
        .def("gl_make_current", &Window::gl_make_current)
        .def("gl_swap_buffers", &Window::gl_swap_buffers)
        .def("gl_set_swap_interval", &Window::gl_set_swap_interval, py::arg("interval"))
        .def("close", &Window::close)
        .def("__enter__", [](Window &self) -> Window & { return self; })
        .def("__exit__", [](Window &self, const py::object &, const py::object &, const py::object &) {
            self.close();
            return false;
        });
}
