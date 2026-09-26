# Architecture

## Layered Architecture

```
┌─────────────────────────────────────┐
│      Application + GLUT Callbacks   │   ← State machine, event handling
├─────────────────────────────────────┤
│        Renderer (OpenGL)            │   ← All GL_* calls live here
├──────────────┬──────────────────────┤
│  Algorithm   │  Graph Plotter       │   ← Pure computation, no GL
│  Classes     │  + Expression Parser │
├──────────────┴──────────────────────┤
│  Common (Point, AlgorithmStep)      │   ← Data structures
└─────────────────────────────────────┘
```

## Key Design Decision: Algorithm-Rendering Separation

The most important architectural decision in GLIDE is that **algorithm classes contain zero OpenGL code**.

Each algorithm class:
1. Takes input parameters (endpoints, center, radius)
2. Runs the algorithm to completion
3. Stores every step as an `AlgorithmStep` object
4. Returns a vector of steps

The Renderer reads these steps and displays them using OpenGL. This separation:
- Makes algorithms easy to understand (pure math)
- Makes rendering easy to understand (pure display)
- Allows animation by simply iterating through steps
- Is easy to explain in a viva

## Precomputed Steps Pattern

```
Algorithm.execute()  →  vector<AlgorithmStep>  →  Renderer reads step N
                                                    ↓
                                               Timer advances N
                                                    ↓
                                               Renderer reads step N+1
```

## Application State Machine

```
                    ┌─── GRAPH_PLOTTER
                    │
MAIN_MENU ──────────┼─── ALGORITHM_EXPLORER (Lines)
                    │
                    ├─── CIRCLE_EXPLORER
                    │
                    └─── ABOUT_HELP
```

## Class Diagram

```
DrawingAlgorithm (base)
    ├── DDALine
    ├── BresenhamLine
    ├── DDACircle
    └── BresenhamCircle

GraphFunction (expression parser)
GraphPlotter (connects functions to algorithms)
CoordinateSystem (world ↔ screen transformation)
Renderer (all OpenGL drawing - static methods)
Application (state machine + GLUT callbacks)
```

## Data Flow: Graph Plotting

```
"sin(x)" → GraphFunction.evaluate(x) → [(x₁,y₁), (x₂,y₂), ...]
    → CoordinateSystem.worldToScreen() → [(sx₁,sy₁), (sx₂,sy₂), ...]
    → DDALine/BresenhamLine.execute() for each pair → [pixel, pixel, ...]
    → Renderer draws GL_POINTS
```

## Data Flow: Algorithm Explorer

```
User clicks points (start, end)
    → Algorithm.execute() → vector<AlgorithmStep>
    → User presses Next/Play
    → Renderer draws step[N].plottedPoints as highlighted squares
    → Renderer displays step[N].explanation as text
```

## File Organization

```
GLIDE/
├── src/
│   ├── main.cpp              # Entry point
│   ├── Application.h         # Main app + Renderer + UI
│   ├── Common/
│   │   ├── Point.h           # 2D point
│   │   ├── AlgorithmStep.h   # Step data
│   │   └── MathUtils.h       # Math helpers
│   ├── Algorithms/
│   │   ├── DrawingAlgorithm.h  # Base class
│   │   ├── DDALine.h           # DDA line
│   │   ├── BresenhamLine.h     # Bresenham line
│   │   ├── DDACircle.h         # DDA circle
│   │   └── BresenhamCircle.h   # Bresenham circle
│   └── Graph/
│       ├── GraphFunction.h     # Expression parser
│       └── GraphPlotter.h      # Graph rendering pipeline
├── docs/                       # Documentation
├── CMakeLists.txt             # Build configuration
└── README.md
```
