# Code Structure

## File Overview

| File | Purpose | Lines (approx) |
|------|---------|----------------|
| `src/main.cpp` | Entry point, GLUT setup | ~50 |
| `src/Application.h` | Main app, renderer, UI, state machine | ~2250 |
| `src/Common/Point.h` | 2D point structure | ~55 |
| `src/Common/AlgorithmStep.h` | Step data for visualization | ~60 |
| `src/Common/MathUtils.h` | Math utility functions | ~60 |
| `src/Algorithms/DrawingAlgorithm.h` | Base algorithm class | ~45 |
| `src/Algorithms/DDALine.h` | DDA line implementation | ~145 |
| `src/Algorithms/BresenhamLine.h` | Bresenham line implementation | ~175 |
| `src/Algorithms/DDACircle.h` | DDA circle implementation | ~125 |
| `src/Algorithms/BresenhamCircle.h` | Bresenham circle implementation | ~160 |
| `src/Graph/GraphFunction.h` | Expression parser | ~230 |
| `src/Graph/GraphPlotter.h` | Graph rendering pipeline | ~210 |

## Class Hierarchy

```
DrawingAlgorithm (abstract base)
    ├── DDALine        — implements execute() for DDA line
    ├── BresenhamLine  — implements execute() for Bresenham line
    ├── DDACircle      — implements execute() for DDA circle
    └── BresenhamCircle — implements execute() for Bresenham circle
```

## Key Classes

### Point (`Common/Point.h`)
Simple data holder for 2D coordinates. Provides `pixelX()`, `pixelY()` for rounding, and `toString()` for display.

### AlgorithmStep (`Common/AlgorithmStep.h`)
Stores everything about one algorithm iteration:
- Position (x, y)
- Deltas (Δx, Δy)
- Decision variable
- Human-readable explanation string
- List of plotted pixels
- Symmetric points (for circles)

### DrawingAlgorithm (`Algorithms/DrawingAlgorithm.h`)
Abstract base class. Defines the interface:
- `execute()` — run the algorithm, populate steps
- `getSteps()` — return computed steps
- `getAllPoints()` — return all plotted pixels

### GraphFunction (`Graph/GraphFunction.h`)
Recursive descent expression parser. Key methods:
- `setExpression(string)` — parse and validate
- `evaluate(float x)` — compute y for a given x
- `generatePoints(xMin, xMax, count)` — sample the function

### GraphPlotter (`Graph/GraphPlotter.h`)
Orchestrates the graph rendering pipeline:
- Stores the function and coordinate system
- Samples the function
- Connects consecutive points with line algorithm
- Collects all rasterized pixels

### CoordinateSystem (`Graph/GraphPlotter.h`)
Handles world ↔ screen coordinate conversion:
- `worldToScreenX/Y()` — convert math coords to pixels
- `screenToWorldX/Y()` — convert pixels to math coords
- Zoom, pan, scale management

### Renderer (`Application.h`)
Static methods for all OpenGL drawing:
- `drawText()`, `drawLargeText()`, `drawSmallText()`
- `drawRect()`, `drawRectOutline()`
- `drawButton()`
- `drawPixel()`, `drawPixelSquare()`
- `drawGrid()`
- `drawPixelGrid()`

### Application (`Application.h`)
Central state machine. Manages:
- Application state (menu, plotter, explorer, etc.)
- All GLUT callbacks (display, keyboard, mouse)
- Button system with hit-testing
- Algorithm instances
- Animation timing
- Quiz mode

## Design Patterns Used

| Pattern | Where | Why |
|---------|-------|-----|
| Template Method | DrawingAlgorithm.execute() | Common interface, different implementations |
| Observer (implicit) | GLUT callbacks | Event-driven UI |
| Strategy | DDA vs Bresenham selection | Swappable algorithms |
| State Machine | AppState enum | Clean screen transitions |

## No Complex Patterns
The project deliberately avoids:
- Factory pattern
- Singleton pattern
- Observer pattern (formal)
- MVC architecture
- Event bus systems

Simple, direct code is preferred over architectural elegance.
