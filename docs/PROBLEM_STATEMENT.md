# Problem Statement

## Context
Computer Graphics is a core subject in engineering curricula. Students learn fundamental rasterization algorithms like DDA and Bresenham for lines and circles. However, understanding these algorithms from textbooks alone can be challenging because:

1. The step-by-step calculations are tedious to trace manually
2. The relationship between mathematical decisions (decision variables, slope comparisons) and visual output (which pixels light up) is not immediately obvious
3. Students often memorize formulas without understanding why each step matters

## Problem
There is a need for an **interactive, educational tool** that:

1. **Visualizes** the DDA Line, Bresenham Line, DDA Circle, and Bresenham Circle algorithms step by step
2. **Explains** every calculation, decision variable change, and pixel selection in real time
3. **Demonstrates** how these algorithms are applied in practice (specifically, for drawing function graphs)
4. Is **simple enough** for a college student to understand the source code and explain it in a viva

## Proposed Solution
GLIDE (Graphics Learning & Interactive Demonstration Environment) — an OpenGL application that combines:
- A simple graph plotter that uses DDA/Bresenham to render mathematical functions
- An interactive algorithm visualizer that animates each algorithm's internal workings
- Educational features like pixel grids, step-by-step explanations, and quiz mode

## Scope
This project is an **educational tool**, not a commercial-grade application. The priority is clarity and correctness over features and performance.
