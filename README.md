# Pynecto
Modern Python GUI framework that enable you to build virtually any type of application from simple tools to production-grade systems.


## Framework Architecture
┌──────────────────────────────────────────────────┐
│               APPLICATION LAYER                  │
│         (User's Python code lives here)          │
├──────────────────────────────────────────────────┤
│              PYTHON PUBLIC API                   │
│   Widgets · Layouts · Themes · Data Binding      │
│         Animations · Event Handlers              │
├──────────────────────────────────────────────────┤
│           PYTHON ↔ C BINDING LAYER               │
│              (pybind11 / cffi)                   │
├────────────────┬─────────────────────────────────┤
│   EVENT CORE   │        LAYOUT ENGINE            │
│  - Event loop  │  - Flexbox / Constraint solver  │
│  - Dispatcher  │  - Virtual DOM / tree diffing   │
│  - Input mgmt  │  - Dirty region tracking        │
├────────────────┼─────────────────────────────────┤
│ RENDER BACKEND │      WINDOWING LAYER            │
│  - Draw calls  │  - OS window creation           │
│  - GPU buffers │  - Input capture                │
│  - Text/fonts  │  - Multi-monitor / DPI          │
├────────────────┴─────────────────────────────────┤
│              OS / GPU / HARDWARE                 │
└──────────────────────────────────────────────────┘
