#include "common.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_context(py::module_ &m);
void bind_window(py::module_ &m);
void bind_event(py::module_ &m);

PYBIND11_MODULE(_core, m) {
    m.doc() = "Pynecto native core — internal extension module, use the pynecto package instead";

    py::register_exception<pnc::PncError>(m, "PyNectoError");

    bind_context(m);
    bind_window(m);
    bind_event(m);
}
