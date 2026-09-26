# Project Flow

## High-Level System Flow

```
┌──────────────────┐
│   USER INPUT     │
│  (function /     │
│   coordinates /  │
│   parameters)    │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  PROCESSING      │
│  • Parse expression│
│  • Validate input │
│  • Sample function│
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  ALGORITHM       │
│  • DDA Line      │
│  • Bresenham Line│
│  • DDA Circle    │
│  • Bresenham Circ│
│                  │
│  Generates:      │
│  • Raster points │
│  • Step data     │
│  • Explanations  │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  OPENGL RENDERER │
│  • Draw pixels   │
│  • Draw grid     │
│  • Draw UI       │
│  • Display text  │
│  • Animate steps │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  SCREEN OUTPUT   │
│  • Visualization │
│  • Information   │
│  • Controls      │
└──────────────────┘
```

## Graph Plotter Flow

```
User types "sin(x)"
    │
    ▼
GraphFunction.setExpression("sin(x)")
    │
    ├── Recursive descent parser validates syntax
    │
    ▼
GraphPlotter.computeGraph()
    │
    ├── Get visible x range from CoordinateSystem
    │
    ├── GraphFunction.generatePoints(xMin, xMax, sampleCount)
    │   │
    │   ├── For each sample x: evaluate y = sin(x)
    │   │
    │   └── Returns: [(x₁,y₁), (x₂,y₂), ..., (xₙ,yₙ)]
    │
    ├── For each consecutive pair (Pᵢ, Pᵢ₊₁):
    │   │
    │   ├── Convert to screen coordinates
    │   │
    │   ├── Create line algorithm (DDA or Bresenham)
    │   │
    │   ├── Algorithm.execute()
    │   │   │
    │   │   └── Generates raster pixels + step data
    │   │
    │   └── Store as GraphSegment
    │
    └── Collect all raster points
         │
         ▼
    Renderer draws GL_POINTS
```

## Algorithm Explorer Flow

```
User clicks start point → User clicks end point
    │
    ▼
Algorithm.setEndpoints(start, end)
Algorithm.execute()
    │
    ├── Compute all steps
    │   │
    │   └── For each iteration:
    │       ├── Calculate position
    │       ├── Make decision (e.g., e >= 0?)
    │       ├── Update decision variable
    │       ├── Generate explanation text
    │       └── Store as AlgorithmStep
    │
    └── Store all steps in vector
         │
         ▼
User presses Next / Play
    │
    ├── Increment step counter
    │
    ├── Renderer reads step[counter]
    │   │
    │   ├── Draw all previous pixels (completed)
    │   ├── Draw current pixel (highlighted)
    │   └── Display step explanation
    │
    └── Timer auto-advances (if playing)
```

## Application State Flow

```
                         Launch
                           │
                           ▼
                    ┌──────────────┐
                    │  MAIN MENU   │
                    └──────┬───────┘
                           │
            ┌──────────────┼──────────────┬─────────────┐
            │              │              │             │
            ▼              ▼              ▼             ▼
    ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌─────────┐
    │  GRAPH    │  │ ALGORITHM │  │  CIRCLE   │  │  ABOUT  │
    │  PLOTTER  │  │ EXPLORER  │  │ EXPLORER  │  │  HELP   │
    └───────────┘  └───────────┘  └───────────┘  └─────────┘
            │              │              │             │
            └──────────────┴──────────────┴─────────────┘
                           │
                    (Escape / Back)
                           │
                           ▼
                    ┌──────────────┐
                    │  MAIN MENU   │
                    └──────────────┘
```
