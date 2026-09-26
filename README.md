# GLIDE - Graphics Learning & Interactive Demonstration Environment

An educational OpenGL application that combines a simple graph plotting tool with interactive visualization of classical computer graphics algorithms.

![C++](https://img.shields.io/badge/C++-17-blue) ![OpenGL](https://img.shields.io/badge/OpenGL-GLUT-green)

## What is GLIDE?

GLIDE demonstrates how classical rasterization algorithms work by:

1. **Plotting mathematical functions** (like a simple Desmos) using DDA or Bresenham line algorithms to connect sample points
2. **Visualizing algorithms step-by-step** with detailed explanations of every decision variable, pixel selection, and calculation

### The Central Idea

> GLIDE does not merely draw graphics. It shows **how** classical computer graphics algorithms produce the graphics, and then applies those same algorithms to build a simple interactive graphing environment.

## Algorithms Implemented

| Algorithm | Type | Key Feature |
|-----------|------|-------------|
| DDA Line | Line | Incremental slope-based calculation |
| Bresenham Line | Line | Integer-only decision variable |
| DDA Circle | Circle | Parametric derivatives dθ = 1/r |
| Bresenham Circle | Circle | 8-way symmetry with integer decisions |

Algorithm implementations are based on the algorithms taught in the provided course notes.

## Quick Start

```bash
# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run
./glide
```

## Features

- **Graph Plotter**: Enter mathematical functions, plot using DDA or Bresenham
- **Algorithm Explorer**: Step-by-step visualization of line algorithms  
- **Circle Explorer**: Step-by-step visualization of circle algorithms
- **Pixel Grid Mode**: See individual raster cells being selected
- **Quiz Mode**: Test your understanding of algorithm decisions
- **How It Works**: Explains the rendering pipeline

## Controls

| Key | Action |
|-----|--------|
| 1-5 | Menu navigation |
| +/- | Zoom in/out |
| Arrow keys | Pan view |
| R | Reset view |
| N / P | Next / Previous step |
| Space | Play/Pause animation |
| G | Toggle pixel grid |
| Enter | Plot / Run algorithm |
| Escape | Back / Close |

## Documentation

- [Project Overview](docs/PROJECT_OVERVIEW.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Algorithms](docs/ALGORITHMS.md)
- [User Guide](docs/USER_GUIDE.md)
- [Build & Run](docs/BUILD_AND_RUN.md)
- [Viva Notes](docs/VIVA_NOTES.md)
- [Code Structure](docs/CODE_STRUCTURE.md)

## Requirements

- C++ compiler (C++17)
- OpenGL
- GLUT / FreeGLUT
- CMake 3.10+
