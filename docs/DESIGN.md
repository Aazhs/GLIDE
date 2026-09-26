# GLIDE - Design Document

## Architecture Overview

GLIDE follows a simple layered architecture that separates concerns cleanly:

```
┌─────────────────────────────────────┐
│          Application Layer          │
│  (main.cpp, Application)            │
├─────────────────────────────────────┤
│            UI Layer                 │
│  (UI, InputHandler, Menus)          │
├─────────────────────────────────────┤
│         Rendering Layer             │
│  (Renderer, CoordinateSystem)       │
├──────────────┬──────────────────────┤
│  Algorithm   │   Graph              │
│  Layer       │   Layer              │
│  (DDA,       │   (GraphFunction,    │
│  Bresenham)  │    GraphPlotter,     │
│              │    ExprParser)       │
├──────────────┴──────────────────────┤
│         Common Layer                │
│  (Point, AlgorithmStep, MathUtils)  │
└─────────────────────────────────────┘
```

## Key Design Decisions

### 1. Algorithm-Rendering Separation
Algorithm classes compute steps and store them in `AlgorithmStep` vectors.
They contain ZERO OpenGL code. The Renderer reads these steps and displays them.
This is the most important architectural decision.

### 2. Precomputed Steps
All algorithms run to completion first, storing every step.
The animation system simply iterates through stored steps.
This avoids mixing timing logic with algorithm logic.

### 3. Simple Expression Parser
A recursive-descent parser handles: numbers, x, +, -, *, /, ^, parentheses,
sin, cos, tan, sqrt, abs. No external libraries.

### 4. Coordinate System
Mathematical world coordinates are transformed to screen coordinates via:
- screenX = (worldX - panX) * zoom * scale + originScreenX
- screenY = (worldY - panY) * zoom * scale + originScreenY

### 5. Graph Rendering Pipeline
```
User enters function
  → Parse expression
  → Sample x values at configured density
  → Evaluate y = f(x) for each sample
  → Convert (x,y) pairs to screen coordinates
  → Connect consecutive points using selected line algorithm
  → Each algorithm call generates raster pixels
  → OpenGL renders the pixels via GL_POINTS
```

### 6. Application States
```
MAIN_MENU → GRAPH_PLOTTER
           → ALGORITHM_EXPLORER
           → CIRCLE_EXPLORER
           → ABOUT_HELP
```

### 7. UI Approach
Simple OpenGL rectangles for buttons, GLUT bitmap fonts for text,
mouse hit-testing for interaction. No GUI framework.

## Class Responsibilities

| Class | Responsibility |
|-------|---------------|
| Point | Simple (x,y) data holder |
| AlgorithmStep | Stores one iteration's data: position, decision var, explanation |
| DrawingAlgorithm | Base class with virtual execute() and getSteps() |
| DDALine | DDA line algorithm from course notes |
| BresenhamLine | Bresenham line algorithm from course notes |
| DDACircle | DDA circle algorithm from course notes |
| BresenhamCircle | Bresenham circle algorithm from course notes |
| GraphFunction | Expression parser + evaluator |
| GraphPlotter | Samples function, connects points via algorithms |
| CoordinateSystem | World ↔ Screen coordinate conversion |
| Renderer | All OpenGL drawing: grid, axes, pixels, text, UI elements |
| UI | Button definitions, layout, hit-testing, state management |
| Application | GLUT callbacks, main loop, state machine |
