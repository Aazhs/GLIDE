# Algorithms

Algorithm implementations are based on the algorithms taught in the provided course notes.

---

## 1. DDA Line Drawing Algorithm

### Purpose
Draw a straight line between two points by computing successive pixel positions using incremental calculation.

### Input
- Start point (x₁, y₁)
- End point (x₂, y₂)

### Variables
| Variable | Meaning |
|----------|---------|
| Δx | x₂ - x₁ (horizontal distance) |
| Δy | y₂ - y₁ (vertical distance) |
| m | Slope = Δy / Δx |
| steps | max(|Δx|, |Δy|) — number of pixels to generate |
| xIncrement | How much x changes per step |
| yIncrement | How much y changes per step |

### Logic

1. Calculate Δx = x₂ - x₁ and Δy = y₂ - y₁
2. Calculate slope m = Δy / Δx
3. Compare |Δx| and |Δy| to determine the major axis:
   - **If |Δx| ≥ |Δy|** (x is major): step x by 1, y increments by m
   - **If |Δx| < |Δy|** (y is major): step y by 1, x increments by 1/m
4. Starting from (x₁, y₁), generate successive points
5. Round each (x, y) to the nearest integer for the pixel position

### Step-by-Step Flow

```
Given: (x₁, y₁) = (2, 3) and (x₂, y₂) = (7, 6)

Δx = 7 - 2 = 5
Δy = 6 - 3 = 3
m = 3/5 = 0.6

|Δx| ≥ |Δy| → x is major axis

xIncrement = 1 (step along x)
yIncrement = m = 0.6

Step 0: x=2.0, y=3.0  → pixel (2, 3)
Step 1: x=3.0, y=3.6  → pixel (3, 4)
Step 2: x=4.0, y=4.2  → pixel (4, 4)
Step 3: x=5.0, y=4.8  → pixel (5, 5)
Step 4: x=6.0, y=5.4  → pixel (6, 5)
Step 5: x=7.0, y=6.0  → pixel (7, 6)
```

### What the Visualizer Displays
- Starting and ending points
- Δx, Δy, slope
- Chosen major axis
- Current x and y values
- Each increment calculation
- Generated pixel position
- Complete list of all generated points

### Relation to Implementation
See `src/Algorithms/DDALine.h`. The `execute()` method directly implements this flow, storing each iteration as an `AlgorithmStep` object.

---

## 2. Bresenham Line Drawing Algorithm

### Purpose
Draw a line using only **integer arithmetic** (no floating-point operations), making it more efficient than DDA.

### Input
- Start point (x₁, y₁)
- End point (x₂, y₂)

### Variables
| Variable | Meaning |
|----------|---------|
| Δx | \|x₂ - x₁\| (absolute horizontal distance) |
| Δy | \|y₂ - y₁\| (absolute vertical distance) |
| e | Decision variable — tracks accumulated error |

### Logic

From the course notes:

1. Set Δx = |x₂ - x₁|, Δy = |y₂ - y₁|
2. Starting point: x = x₁, y = y₁
3. Initial decision variable: **e = 2Δy - Δx**
4. For each step:
   - **While e ≥ 0**: y = y + 1, e = e - 2Δx
   - **Always**: x = x + 1, e = e + 2Δy
5. Continue until all Δx steps are completed

### The Decision Variable Explained

The decision variable `e` represents the distance between the true line position and the current pixel. When e ≥ 0, the true line has crossed the halfway point to the next y row, so we step y.

### Step-by-Step Flow

```
Given: (1, 3) → (5, 6)

Δx = 4, Δy = 3
Initial e = 2(3) - 4 = 2

Step 0: Plot (1, 3), e = 2
  e = 2 ≥ 0 → y = 4, e = 2 - 8 = -6
  x = 2, e = -6 + 6 = 0

Step 1: Plot (2, 4), e = 0
  e = 0 ≥ 0 → y = 5, e = 0 - 8 = -8
  x = 3, e = -8 + 6 = -2

Step 2: Plot (3, 5), e = -2
  e = -2 < 0 → no y change
  x = 4, e = -2 + 6 = 4

Step 3: Plot (4, 5), e = 4
  e = 4 ≥ 0 → y = 6, e = 4 - 8 = -4
  x = 5, e = -4 + 6 = 2

Step 4: Plot (5, 6)
```

### Handling All Directions
For lines not in the primary octant (slope 0-1, left-to-right), the implementation uses coordinate reflection to map into the primary case, runs the core algorithm unchanged, then maps back.

### What the Visualizer Displays
- x, y at each step
- Δx, Δy
- Current decision variable e
- Whether y changed (and why)
- How e was updated
- Iteration number
- Example explanations like:
  - "e = 4 ≥ 0: increase y"
  - "e = e - 2Δx = 4 - 8 = -4"
  - "x = x + 1 = 5"
  - "e = e + 2Δy = -4 + 6 = 2"

### Relation to Implementation
See `src/Algorithms/BresenhamLine.h`. The core algorithm loop directly matches the notes, with coordinate transformation for other octants.

---

## 3. DDA Circle Drawing Algorithm

### Purpose
Draw a circle using parametric differentiation and small incremental steps.

### Input
- Center (cx, cy)
- Radius r

### Variables
| Variable | Meaning |
|----------|---------|
| r | Radius |
| θ | Current angle |
| dθ | Angular increment = 1/r |
| x, y | Current position on circle |
| dx | Increment in x = -y × dθ |
| dy | Increment in y = x × dθ |

### Derivation (from the notes)

A circle is defined parametrically as:
```
x = r cos θ
y = r sin θ
```

Taking derivatives:
```
dx/dθ = -r sin θ = -y
dy/dθ =  r cos θ =  x
```

Therefore:
```
dx = -y × dθ
dy =  x × dθ
```

Choosing **dθ = 1/r** makes each step approximately 1 pixel along the arc, since arc length = r × dθ = r × (1/r) = 1.

### Step-by-Step Flow

```
Given: center (0, 0), radius 5

dθ = 1/5 = 0.2
Start: x = 5, y = 0 (θ = 0)
Total steps ≈ 2π × 5 ≈ 31

Step 1: x=5.0, y=0.0
  dx = -0.0 × 0.2 = 0.0
  dy =  5.0 × 0.2 = 1.0
  New: x=5.0, y=1.0 → pixel (5, 1)

Step 2: x=5.0, y=1.0
  dx = -1.0 × 0.2 = -0.2
  dy =  5.0 × 0.2 = 1.0
  New: x=4.8, y=2.0 → pixel (5, 2)

... continues around the circle
```

### What the Visualizer Displays
- Radius r
- Current θ (in degrees)
- Current x, y
- dx, dy calculations
- dθ value
- Generated pixel
- Step number

### Relation to Implementation
See `src/Algorithms/DDACircle.h`. The `execute()` method directly implements the parametric increment approach from the notes.

---

## 4. Bresenham Circle Drawing Algorithm

### Purpose
Draw a circle using only integer arithmetic and 8-way symmetry for maximum efficiency.

### Input
- Center (cx, cy)
- Radius r

### Variables
| Variable | Meaning |
|----------|---------|
| r | Radius (integer) |
| x, y | Current position (starting at x=0, y=r) |
| d | Decision variable (starting at d = 3 - 2r) |

### 8-Way Symmetry

From a single calculated point (x, y), we generate all 8 reflections:

```
  ( x,  y)    ( y,  x)
  (-x,  y)    (-y,  x)
  (-x, -y)    (-y, -x)
  ( x, -y)    ( y, -x)
```

This means we only compute one octant (0° to 45°) and get the full circle for free.

### Logic (from the notes)

1. Start: x = 0, y = r
2. Initial decision variable: **d = 3 - 2r**
3. At each step:
   - Plot (x, y) and its 7 symmetric points
   - **If d < 0**: d = d + 4x + 6 (move only in x)
   - **If d ≥ 0**: d = d + 4(x - y) + 10, y = y - 1 (move in x and y)
   - x = x + 1
4. Continue while x ≤ y

### Step-by-Step Flow

```
Given: center (0, 0), radius 5

Start: x=0, y=5, d = 3 - 2(5) = -7

Step 1: Plot (0,5) and 7 symmetric points
  d = -7 < 0
  d = -7 + 4(0) + 6 = -1
  x = 1

Step 2: Plot (1,5) and 7 symmetric points
  d = -1 < 0
  d = -1 + 4(1) + 6 = 9
  x = 2

Step 3: Plot (2,5) and 7 symmetric points
  d = 9 ≥ 0
  d = 9 + 4(2-5) + 10 = 7
  y = 4
  x = 3

Step 4: Plot (3,4) and 7 symmetric points
  d = 7 ≥ 0
  d = 7 + 4(3-4) + 10 = 13
  y = 3
  x = 4

(x > y, stop)
```

### What the Visualizer Displays
- Radius r
- Current x, y
- Decision variable d
- Decision taken (d < 0 or d ≥ 0)
- Updated d calculation
- All 8 symmetric points (highlighted differently)
- Current iteration
- The symmetry is made visually obvious: when one point appears in the first octant, the corresponding 7 symmetric points are animated/highlighted

### Relation to Implementation
See `src/Algorithms/BresenhamCircle.h`. The `execute()` method directly implements the notes, with `getSymmetricPoints()` computing all 8 reflections.
