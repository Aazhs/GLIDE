# Project Overview

## GLIDE - Graphics Learning & Interactive Demonstration Environment

GLIDE is an educational OpenGL application designed for college-level Computer Graphics courses. It serves two interconnected purposes:

### 1. Graph Plotting Tool
A simple graphing environment where users can enter mathematical functions (like `y = x*x`, `y = sin(x)`) and see them plotted. The key difference from conventional plotters: **the graph is drawn using the actual DDA or Bresenham line algorithm**, not OpenGL's built-in line primitives.

### 2. Algorithm Visualizer
An interactive system that explains how DDA Line, Bresenham Line, DDA Circle, and Bresenham Circle algorithms work internally. Users can step through each algorithm iteration, seeing the calculations, decision variables, and pixel selections in real time.

### The Connection
The graph plotter and algorithm visualizer are connected by a central idea:

```
User enters function → Evaluate at sample points → Convert to screen coordinates
     → Connect consecutive points using DDA or Bresenham → Render pixels via OpenGL
```

Every curve on the graph is built from many small rasterized line segments. GLIDE makes this visible and understandable.

## Target Audience
- Second/third-year engineering students studying Computer Graphics
- Students preparing for viva examinations on rasterization algorithms
- Anyone learning how classical drawing algorithms work

## Technology Stack
- **Language**: C++ (C++17)
- **Graphics**: OpenGL with GLUT/FreeGLUT
- **Build**: CMake
- **Dependencies**: Standard library + OpenGL + GLUT only

## Design Philosophy
- Simple code that can be explained line-by-line in a viva
- Algorithm logic completely separated from rendering
- Precomputed steps for clean animation
- No complex design patterns or unnecessary abstractions

Algorithm implementations are based on the algorithms taught in the provided course notes.
