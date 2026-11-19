# GLIDE - Graphics Learning & Interactive Demonstration Environment

An educational OpenGL application for exploring line drawing, circle drawing, polygon filling, and graph plotting.

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
| Midpoint Circle | Circle | Chooses each pixel using a midpoint test |
| Boundary Fill | Polygon | Stops at the boundary colour, with 4 or 8 neighbours |
| Flood Fill | Polygon | Replaces the seed colour, with 4 or 8 neighbours |
| Scanline Fill | Polygon | Fills between edge intersections on each row |

Algorithm implementations are based on the algorithms taught in the provided course notes.

## Quick Start

Download GLIDE for Windows, macOS, or Linux from the [GLIDE homepage](https://aazhs.github.io/GLIDE/) or [GitHub Releases](https://github.com/Aazhs/GLIDE/releases).

- **Windows x64** — Extract the ZIP and open `glide.exe`.
- **macOS 12 or later** — Extract the ZIP and open `GLIDE.app` on Apple Silicon or Intel.
- **Linux x64** — Extract the archive and run `./glide` from the extracted folder. The build targets Ubuntu 22.04 or later and needs an X11 or XWayland desktop, OpenGL, and GLU.

### Build from source

#### macOS / Linux
```bash
mkdir build && cd build
cmake ..
cmake --build .
./glide
```

#### Windows (Visual Studio)
```cmd
vcpkg install freeglut:x64-windows
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release
.\Release\glide.exe
```

#### Windows (MinGW)
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
glide.exe
```

> See [BUILD_AND_RUN.md](docs/BUILD_AND_RUN.md) for detailed setup instructions and troubleshooting.

## Features

- **Graph Plotter**: Enter mathematical functions, plot using DDA or Bresenham
- **Algorithm Explorer**: Step-by-step visualization of line algorithms  
- **Circle Explorer**: Step-by-step visualization of circle algorithms
- **Polygon Explorer**: Pick a fill method and watch pixels or rows fill step by step.
- **Pixel Grid Mode**: See individual raster cells being selected
- **Quiz Mode**: Test your understanding of algorithm decisions
- **How It Works**: Explains the rendering pipeline

Open **Polygon Explorer** from the main menu or press **5**. Choose a fill method, click the grid to move the seed for boundary or flood fill, and use **Next** or **Play** to watch it fill. Scanline fill uses the polygon edges to fill one row at a time.

Select **Custom** to draw your own polygon. Click at least three corners in order, use **Undo** if needed, then select **Close shape**. Choose any fill method and press **Play**.

## Controls

| Key | Action |
|-----|--------|
| 1–5 | Open a tool or Help from the main menu |
| 6 | Exit from the main menu |
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
