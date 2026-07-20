#ifndef PYNECTO_BINDINGS_COMMON_H
#define PYNECTO_BINDINGS_COMMON_H

#include <pynecto/window.h> /* pnc_get_error() */

#include <stdexcept>

namespace pnc {

/* Registered as pynecto._core.PyNectoError in module.cpp; every bind_*.cpp
 * throws this so a single py::register_exception call handles translation. */
class PncError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline void check(bool ok) {
    if (!ok) {
        throw PncError(pnc_get_error());
    }
}

inline void check_ptr(const void *ptr) {
    if (ptr == nullptr) {
        throw PncError(pnc_get_error());
    }
}

} // namespace pnc

#endif // PYNECTO_BINDINGS_COMMON_H
