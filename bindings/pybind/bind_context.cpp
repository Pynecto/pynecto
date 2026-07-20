#include <pynecto/context.h>

#include "common.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_context(py::module_ &m) {
    py::class_<PncContextConfig>(m, "ContextConfig")
        .def(py::init([]() { return pnc_context_config_default(); }))
        .def_readwrite("headless", &PncContextConfig::headless)
        .def_readwrite("event_queue_capacity", &PncContextConfig::event_queue_capacity);

    m.def(
        "context_init",
        [](const PncContextConfig *config) { pnc::check(pnc_context_init(config)); },
        py::arg("config") = nullptr);

    m.def("context_shutdown", &pnc_context_shutdown);
    m.def("context_is_initialized", &pnc_context_is_initialized);
    m.def("time_ms", &pnc_time_ms);
    m.def("time_delta", &pnc_time_delta);
}
