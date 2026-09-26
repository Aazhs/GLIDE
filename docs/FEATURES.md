# Features

## Core Features

### 1. Algorithm Visualization
- Step-by-step animation of DDA Line, Bresenham Line, DDA Circle, and Bresenham Circle
- Display of all internal variables (decision variables, slopes, coordinates)
- Human-readable explanations at every step
- Play, pause, next, previous, and reset controls
- Adjustable animation speed

### 2. Graph Plotter
- Mathematical function input with expression parser
- Support for: +, -, *, /, ^, parentheses, sin, cos, tan, sqrt, abs
- Graph rendering using DDA or Bresenham line algorithms
- Configurable sample density (10-1000 points)
- Preset function buttons for quick demonstrations

### 3. Pixel Grid Mode
- Toggleable raster grid overlay
- Individual pixel cells highlighted when selected
- Coordinate labels on selected pixels
- Visual connection between algorithm decisions and pixel output

### 4. Circle Explorer
- DDA circle visualization with parametric increment display
- Bresenham circle visualization with 8-way symmetry highlighting
- Calculated points shown in primary color
- Symmetric reflections shown in secondary color
- Adjustable radius and center position

### 5. Coordinate System
- Zoom in/out (keyboard, scroll wheel)
- Pan in all directions (arrow keys)
- Dynamic grid spacing that adapts to zoom level
- Axis labels and tick marks
- World-to-screen coordinate transformation

## Educational Features

### 6. "How It Works" Panel
Explains the complete rendering pipeline:
1. Function evaluation
2. Point sampling
3. Coordinate conversion
4. Rasterization algorithm application
5. OpenGL pixel rendering

### 7. Explain Mode (Graph Plotter)
Shows algorithm calculations for each segment of the graph curve, making it visible that the graph is built from many small rasterized line segments.

### 8. Quiz Mode
Interactive quiz that asks "What is the next pixel?" with three choices, testing understanding of algorithm decisions.

## User Interface Features

### 9. Main Menu
Clean navigation with keyboard shortcuts (1-5) and mouse-clickable buttons.

### 10. Interactive Controls
- Mouse click for point/center selection
- Keyboard shortcuts for all common operations
- Button-based UI for all features
- Hover effects on buttons

### 11. Information Panel
Right-side panel displaying:
- Algorithm name and description
- Current parameters
- Step counter
- Detailed step explanation
- Decision variable state

### 12. About / Help Screen
Complete reference of all keyboard and mouse controls.
