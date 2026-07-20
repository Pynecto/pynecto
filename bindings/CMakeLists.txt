# ==============================================================================
#  Pynecto — bindings/CMakeLists.txt
#
#  Defines the pybind11 extension module: pynecto._core
#
#  This is the L2 translation layer: type translation only, no business
#  logic. It links pynecto_core (which brings SDL3, glad, and core/include
#  transitively via its PUBLIC usage requirements).
# ==============================================================================

pybind11_add_module(_core
    pybind/module.cpp
    pybind/bind_context.cpp
    pybind/bind_window.cpp
    pybind/bind_event.cpp
)

target_link_libraries(_core PRIVATE pynecto_core)

target_compile_options(_core PRIVATE ${PYNECTO_C_WARNING_FLAGS})

set_target_properties(_core PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED  ON
    CXX_EXTENSIONS         OFF
)
