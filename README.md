# Pynecto
Modern Python GUI framework that enable you to build virtually any type of application from simple tools to production-grade systems.


## Framework Architecture
```
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
```

### Phase 1: The C Core Foundation
<p> Window creation, event loop, input handling. This is the skeleton everything else sits on. We'll use SDL3 to handle OS-level windowing so there is no need of reimplementing Win32/Cocoa/X11 for every platform. </p>
