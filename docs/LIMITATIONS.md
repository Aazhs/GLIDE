# Limitations

## Algorithm Limitations

1. **DDA Circle floating-point drift**: The DDA circle algorithm accumulates floating-point errors over a full revolution. The circle may not close perfectly for large radii. This is inherent to the algorithm, not a bug.

2. **Bresenham for all octants**: The Bresenham line algorithm uses coordinate transformation to handle all octants. While the core algorithm matches the course notes exactly, the transformation code adds complexity.

3. **No anti-aliasing**: The algorithms produce jagged (aliased) output, as expected from basic rasterization algorithms. Anti-aliasing algorithms like Xiaolin Wu are outside the project scope.

## Expression Parser Limitations

4. **Limited function set**: Only supports sin, cos, tan, sqrt, abs. Does not support:
   - log, ln, exp
   - Implicit functions (e.g., x² + y² = r²)
   - Parametric equations
   - Piecewise functions
   - Multiple variables

5. **No error recovery**: If the expression is malformed, it simply reports an error. It does not suggest corrections.

6. **Integer overflow**: Very large numbers or deeply nested expressions may cause issues.

## Graph Plotter Limitations

7. **Fixed sampling**: Uses evenly-spaced x values. This can miss sharp features like the spike of y = 1/x near x = 0.

8. **No adaptive sampling**: Does not increase sample density in regions of high curvature.

9. **Discontinuity handling**: Simply skips NaN/infinity values. Does not detect and properly render asymptotes or jumps.

10. **Single function**: Can only plot one function at a time.

## UI Limitations

11. **Basic UI**: Simple OpenGL rectangles and text, not a polished GUI. No text cursor, no scroll bars, no dropdown menus.

12. **Fixed layout**: The UI layout adapts to window size to some extent but is not fully responsive.

13. **No undo/redo**: No way to undo actions.

14. **Text input**: The expression input field is basic — no cursor movement, no selection, no copy/paste.

## Performance Limitations

15. **Large sample counts**: Very high sample counts (>500) with the explain/step mode may be slow due to the large number of algorithm steps stored.

16. **No optimization**: The rendering redraws everything every frame. No display list caching or VBO usage.

## Platform Limitations

17. **GLUT deprecation on macOS**: GLUT is deprecated on macOS (though still functional). FreeGLUT is recommended on other platforms.

18. **Window management**: Limited to what GLUT provides — no custom window decorations, no proper dialog boxes.
