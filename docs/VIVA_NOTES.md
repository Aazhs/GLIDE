# Viva Notes

Prepared questions and answers for viva examination. Algorithm implementations are based on the algorithms taught in the provided course notes.

---

### Q1: What is rasterization?
**A:** Rasterization is the process of converting a geometric shape (defined by mathematical equations like y = mx + c) into a set of discrete pixel positions on a screen. Since screens are made of pixels arranged in a grid, we cannot display perfect mathematical shapes — we must choose which pixels to turn on to best approximate them.

---

### Q2: What is DDA?
**A:** DDA stands for Digital Differential Analyzer. It is a line drawing algorithm that uses incremental calculation. Starting from one endpoint, it steps along the major axis by 1 unit and calculates the corresponding position on the minor axis using the slope. It generates successive points by simple additions at each step.

---

### Q3: Why does DDA use increments?
**A:** Instead of recalculating y = mx + c for every x value (which involves multiplication), DDA uses the fact that consecutive points differ by a constant increment. If x increases by 1, y increases by m (the slope). This makes each step just an addition, which is computationally simpler.

---

### Q4: What is the slope?
**A:** The slope m = Δy/Δx represents the steepness of the line. It tells us how many units y changes for every unit change in x. A slope of 0.5 means y increases by 0.5 for every 1 unit increase in x.

---

### Q5: Why compare |Δx| and |Δy| in DDA?
**A:** We choose the axis with the larger change as the "major axis" and step along it by 1. This ensures we never skip pixels. If we stepped along the smaller axis, the larger axis might jump by more than 1 pixel, leaving gaps in the line. By stepping along the major axis, the minor axis increment is always ≤ 1, so every pixel is covered.

---

### Q6: What is Bresenham's decision variable?
**A:** The decision variable `e` (initially e = 2Δy - Δx) represents the accumulated error — the distance between the true mathematical line position and the current pixel. When e ≥ 0, the true line has moved more than halfway to the next row of pixels, so we step in y. When e < 0, we stay at the same y. This allows the algorithm to make optimal pixel choices using only integer arithmetic.

---

### Q7: Why is Bresenham considered efficient?
**A:** Bresenham uses only integer additions and comparisons — no floating-point multiplication or division. The initial setup computes 2Δx and 2Δy (integer multiplications), and then each step only adds or subtracts these values. This makes it significantly faster than DDA, which uses floating-point operations. On early hardware, this difference was substantial.

---

### Q8: What does the circle decision variable represent?
**A:** In Bresenham's circle algorithm, the decision variable d (initially d = 3 - 2r) represents whether the ideal circle passes closer to the current pixel or the one below it. When d < 0, the circle is still within the current y row, so only x moves. When d ≥ 0, the circle has moved to the next row, so both x increases and y decreases.

---

### Q9: Why does Bresenham circle use 8-way symmetry?
**A:** A circle has symmetry across the x-axis, y-axis, and both diagonal lines (y=x and y=-x). This means any point (x, y) on the circle produces 7 other points that are also on the circle: (y,x), (-x,y), (-y,x), (-x,-y), (-y,-x), (x,-y), (y,-x). By exploiting this, we only need to compute one-eighth of the circle (0° to 45°) and can generate the rest through reflection. This makes the algorithm 8 times faster.

---

### Q10: Why do we need coordinate transformation for graph plotting?
**A:** Mathematical coordinates (where x=2 means "two units right of origin") are different from screen coordinates (where x=400 means "400 pixels from the left edge"). We need a mapping function that converts between these two systems, accounting for the origin position, scale (pixels per unit), zoom level, and panning offset. Without this transformation, we couldn't display mathematical graphs on a pixel screen.

---

### Q11: Why separate algorithm logic from rendering?
**A:** Separating them makes both easier to understand:
- The algorithm class contains pure math — it calculates positions and decision variables without any OpenGL code
- The renderer takes those calculated positions and draws them on screen
- This separation also makes animation easy: the algorithm runs once and stores all steps, then the renderer simply reads them one at a time
- In a viva, you can explain the algorithm independently of how it's displayed

---

### Q12: Why use GL_POINTS?
**A:** GL_POINTS tells OpenGL to draw individual pixels at specific positions. Since our algorithms generate individual pixel coordinates (the rasterized output), GL_POINTS is the natural way to display them. Using GL_LINES would defeat the purpose — we're showing that our algorithms create the line pixel by pixel, not using OpenGL's built-in line drawing.

---

### Q13: Why are graph functions converted into line segments?
**A:** A mathematical function like y = sin(x) is continuous — it has a value at every x. But we can only plot a finite number of points. We sample the function at many x values, getting points like P1, P2, P3, etc. Then we connect consecutive points with straight line segments (using DDA or Bresenham). If we sample densely enough, these short line segments approximate the smooth curve.

---

### Q14: What happens when zooming?
**A:** Zooming changes the scale — the number of screen pixels per mathematical unit. Zooming in means more pixels per unit, so the graph appears larger and more detailed. The coordinate system transformation (worldToScreen) multiplies by the zoom factor. When we recompute the graph after zooming, the line segments are longer (in pixel terms), so the algorithms generate more pixels per segment.

---

### Q15: What is the role of OpenGL in this project?
**A:** OpenGL serves purely as a display layer. It provides:
- A window to draw in (via GLUT)
- Pixel plotting (GL_POINTS)
- Simple shapes for the UI (GL_QUADS for buttons)
- Text rendering (GLUT bitmap fonts)
- Input handling (keyboard and mouse callbacks via GLUT)

OpenGL does NOT perform the actual line/circle drawing — that's done by our DDA and Bresenham implementations. We plot the pixels they generate using OpenGL.

---

### Q16: How does the DDA circle algorithm work?
**A:** It uses the parametric form: x = r cos θ, y = r sin θ. By differentiating, we get dx = -y dθ and dy = x dθ. Choosing dθ = 1/r makes each step approximately 1 pixel along the arc. Starting at (r, 0), each step computes x_new = x - y/r and y_new = y + x/r. We continue for about 2πr steps to complete the full circle.

---

### Q17: What is the expression parser?
**A:** The expression parser is a simple recursive descent parser that converts a text string like "sin(x*x) + 2" into a computed numerical value for any given x. It handles operator precedence (multiplication before addition), parentheses, and mathematical functions. The grammar rules are implemented as nested function calls: parseExpression calls parseTerm calls parsePower calls parseUnary calls parsePrimary.

---

### Q18: What is the precomputed steps pattern?
**A:** Instead of mixing algorithm computation with animation timing, we:
1. Run the complete algorithm once, storing every step in a vector
2. Each step contains the position, decision variable, explanation text, etc.
3. The animation timer simply increments a counter
4. The renderer reads step[counter] and displays it

This makes the code much simpler and the algorithm easier to verify.

---

### Q19: How is Bresenham's line algorithm adapted for all directions?
**A:** The notes describe the algorithm for the primary octant (slope 0 to 1, moving left to right). For other octants, we use coordinate transformation: if the line is steep (|Δy| > |Δx|), we swap x and y roles. We also track the sign of the direction (positive or negative) for stepping. The core algorithm remains unchanged — only the coordinate mapping changes.

---

### Q20: What are the limitations of this project?
**A:** 
- The expression parser handles basic functions only (no implicit functions, no parametric curves)
- Graph plotting uses a fixed set of sample points (may miss sharp features)
- The UI is simple OpenGL rectangles (not a polished GUI toolkit)
- DDA circle accumulates floating-point errors over a full circle
- Performance is not optimized for very high sample counts
- Only 2D visualization (no 3D graphics)
