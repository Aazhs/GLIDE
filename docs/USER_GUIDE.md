# User Guide

## Starting GLIDE

When you launch GLIDE, you see the main menu with five options:

```
[1] Graph Plotter
[2] Algorithm Explorer (Lines)
[3] Circle Explorer
[4] About / Help
[5] Exit
```

Navigate using mouse clicks on buttons or keyboard shortcuts (1-5).

---

## Graph Plotter

### Entering a Function
1. Click the input field next to "y ="
2. Type a mathematical expression (e.g., `sin(x)`, `x*x`, `2*x+3`)
3. Press **Enter** or click **Plot**

### Supported Expressions
| Element | Examples |
|---------|----------|
| Numbers | `3`, `2.5`, `0.1` |
| Variable | `x` |
| Operators | `+`, `-`, `*`, `/`, `^` |
| Parentheses | `(x+1)*(x-1)` |
| Functions | `sin(x)`, `cos(x)`, `tan(x)`, `sqrt(x)`, `abs(x)` |
| Constants | `pi` |

### Preset Functions
Click one of the preset buttons at the bottom: `x*x`, `sin(x)`, `cos(x)`, `2*x+1`, `x^3`, `sqrt(abs(x))`

### Choosing the Algorithm
- Click **DDA** to draw using the DDA line algorithm
- Click **Bresenham** to draw using Bresenham's line algorithm

### View Modes
- **Normal Plot**: Shows the complete graph immediately
- **Step**: Steps through segment by segment
- **Explain**: Shows detailed algorithm calculations for each segment

### Navigation (Step/Explain modes)
- **< Prev**: Go back one step
- **Next >**: Advance one step
- **Reset**: Go back to the beginning

### How It Works
Click the **How It Works** button to see an explanation of how the graph is drawn using the rasterization algorithms.

### Zoom and Pan
| Action | Control |
|--------|---------|
| Zoom in | `+` key or scroll up |
| Zoom out | `-` key or scroll down |
| Pan | Arrow keys |
| Reset view | `R` key |
| Fewer samples | `[` key |
| More samples | `]` key |

---

## Algorithm Explorer (Lines)

### Selecting an Algorithm
Click **DDA Line** or **Bresenham Line** at the top.

### Setting Points
1. Click anywhere in the plot area to set the **start point** (green)
2. Click again to set the **end point** (red)
3. The algorithm runs automatically

### Animation Controls
| Button | Action |
|--------|--------|
| < Previous | Go back one step |
| Next > | Advance one step |
| Play | Auto-advance through steps |
| Pause | Stop auto-advancing |
| Reset | Go back to step 0 |
| Run Algo | Re-run the algorithm |

### Keyboard Shortcuts
| Key | Action |
|-----|--------|
| `N` | Next step |
| `P` | Previous step |
| `Space` | Play/Pause |
| `S` | Reset animation |
| `G` | Toggle pixel grid |

### Pixel Grid
Toggle the pixel grid with the **Grid: ON/OFF** button or `G` key. When zoomed in, each raster cell appears as a visible square with coordinate labels.

### Speed Control
- **Slower**: Increases time between steps
- **Faster**: Decreases time between steps

### Quiz Mode
Press `Q` or click **Quiz** to test your understanding. The quiz asks: "Given the current pixel, what is the next pixel?" Choose from three options.

---

## Circle Explorer

### Selecting an Algorithm
Click **DDA Circle** or **Bresenham Circle**.

### Setting Parameters
- Click in the plot area to set the **circle center**
- Click **R +** and **R -** to adjust the radius

### Understanding the Visualization

#### DDA Circle
Watch as the algorithm traces the circle using parametric increments. Each step shows:
- Current angle θ
- The dx and dy calculations
- The plotted pixel

#### Bresenham Circle
Watch the algorithm compute points in one octant, then see the 8-way symmetry:
- **Yellow** = calculated point (first octant)
- **Orange** = symmetric reflections (other 7 octants)
- The decision variable `d` and its updates

### Animation
Same controls as Algorithm Explorer: Previous, Next, Play, Pause, Reset.

---

## About / Help

Shows a complete reference of all keyboard and mouse controls.
