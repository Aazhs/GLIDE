/*
 * BresenhamLine.h
 * ---------------
 * Bresenham Line Drawing Algorithm.
 *
 * Algorithm based on the course notes:
 *
 * Given two endpoints (x1,y1) and (x2,y2):
 *   Δx = |x2 - x1|
 *   Δy = |y2 - y1|
 *
 * Starting point: x = x1, y = y1
 *
 * Initial decision variable:
 *   e = 2Δy - Δx
 *
 * For each step:
 *   While e >= 0:
 *     y = y + 1      (move in y direction)
 *     e = e - 2Δx    (adjust decision variable)
 *
 *   x = x + 1        (always move in x direction)
 *   e = e + 2Δy      (adjust decision variable)
 *
 * Continue until x reaches x2.
 *
 * KEY INSIGHT: Bresenham's algorithm uses only integer arithmetic
 * (additions and comparisons), making it very efficient compared
 * to DDA which requires floating-point operations.
 *
 * The decision variable 'e' represents the accumulated error.
 * When e >= 0, the true line has moved far enough from the
 * current y position that we need to step in y.
 *
 * For directions other than the primary octant (slope 0 to 1,
 * left-to-right), we use coordinate transformation/reflection
 * to map the line into the primary octant, run the core algorithm,
 * then transform back. The core algorithm stays unchanged.
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_BRESENHAM_LINE_H
#define GLIDE_BRESENHAM_LINE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include <cmath>
#include <sstream>
#include <algorithm>

class BresenhamLine : public DrawingAlgorithm {
private:
    Point startPoint;
    Point endPoint;

public:
    BresenhamLine() {
        algorithmName = "Bresenham Line";
    }

    BresenhamLine(Point start, Point end) : startPoint(start), endPoint(end) {
        algorithmName = "Bresenham Line";
    }

    void setEndpoints(Point start, Point end) {
        startPoint = start;
        endPoint = end;
    }

    Point getStart() const { return startPoint; }
    Point getEnd() const { return endPoint; }

    std::string getDescription() const override {
        return "Bresenham's line algorithm uses only integer arithmetic "
               "(additions and comparisons) to determine which pixels to "
               "plot. A decision variable 'e' tracks accumulated error and "
               "decides whether to step in the y-direction.";
    }

    /*
     * Execute the Bresenham Line algorithm.
     *
     * The core algorithm from the notes handles the case where
     * 0 <= slope <= 1 and x1 < x2. For other cases, we use
     * coordinate reflection to map into this primary case,
     * run the algorithm, then map back.
     *
     * The coordinate transformation is clearly separated from
     * the core algorithm logic.
     */
    void execute() override {
        clear();

        int x1 = MathUtils::roundToInt(startPoint.x);
        int y1 = MathUtils::roundToInt(startPoint.y);
        int x2 = MathUtils::roundToInt(endPoint.x);
        int y2 = MathUtils::roundToInt(endPoint.y);

        /* Handle single point case */
        if (x1 == x2 && y1 == y2) {
            AlgorithmStep step;
            step.iteration = 1;
            step.x = (float)x1;
            step.y = (float)y1;
            step.deltaX = 0;
            step.deltaY = 0;
            step.explanation = "Single point. Plot pixel.";
            step.decision = "Plot (" + std::to_string(x1) + ", " + std::to_string(y1) + ")";
            step.plottedPoints.push_back(Point((float)x1, (float)y1));
            steps.push_back(step);
            allPoints.push_back(Point((float)x1, (float)y1));
            return;
        }

        /*
         * COORDINATE TRANSFORMATION
         * -------------------------
         * The core Bresenham algorithm from the notes works for:
         *   - x increasing left to right
         *   - slope between 0 and 1 (|Δx| >= |Δy|)
         *
         * To handle all octants, we track two flags:
         *   steep: true if |Δy| > |Δx| (swap x and y roles)
         *   reversed: true if we need to go right-to-left
         *
         * These transformations do NOT change the algorithm.
         * They just remap coordinates so the algorithm always
         * operates in its natural octant.
         */

        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);

        /* Determine step directions */
        int sx = (x1 < x2) ? 1 : -1;   // x step direction
        int sy = (y1 < y2) ? 1 : -1;   // y step direction

        bool steep = (dy > dx);

        /*
         * If the line is steep (|Δy| > |Δx|), swap the roles of x and y.
         * This maps the steep case into the gentle case so the core
         * algorithm works unchanged.
         */
        if (steep) {
            std::swap(dx, dy);
        }

        /*
         * CORE BRESENHAM ALGORITHM (from the notes)
         *
         * Initial decision variable: e = 2Δy - Δx
         *
         * At each step:
         *   Plot current point
         *   While e >= 0: move in minor axis, e = e - 2Δx
         *   Move in major axis, e = e + 2Δy
         */
        int e = 2 * dy - dx;   // Decision variable: e = 2Δy - Δx
        int x = x1;
        int y = y1;

        for (int i = 0; i <= dx; i++) {
            AlgorithmStep step;
            step.iteration = i + 1;
            step.deltaX = (float)dx;
            step.deltaY = (float)dy;
            step.decisionVariable = (float)e;

            /* The actual pixel position depends on whether we swapped axes */
            int plotX = steep ? y : x;
            int plotY = steep ? x : y;

            step.x = (float)plotX;
            step.y = (float)plotY;

            Point pixel((float)plotX, (float)plotY);
            step.plottedPoints.push_back(pixel);

            /* Build explanation */
            std::ostringstream oss;
            if (i == 0) {
                oss << "Bresenham Line: (" << x1 << "," << y1
                    << ") to (" << x2 << "," << y2 << ")\n";
                oss << "dx = " << dx << ", dy = " << dy << "\n";
                oss << "Initial e = 2*dy - dx = 2*" << dy << " - " << dx
                    << " = " << e << "\n";
                oss << "Plot starting pixel " << pixel.toPixelString();
                if (steep) {
                    oss << "\n(Steep line: x and y roles swapped)";
                }
            } else {
                oss << "Step " << i << ": e = " << e << "\n";
            }

            /* Decision logic */
            std::ostringstream decOss;
            bool yChanged = false;

            if (e >= 0) {
                decOss << "e = " << e << " >= 0: move in y direction\n";
                if (steep) {
                    x += sx;  // In steep mode, "y" is actually x
                } else {
                    y += sy;
                }
                yChanged = true;
                e -= 2 * dx;
                decOss << "e = e - 2*dx = " << (e + 2*dx) << " - " << (2*dx)
                       << " = " << e << "\n";
            } else {
                if (i > 0) {
                    decOss << "e = " << (e - 2*dy) << " < 0: no y change\n";
                }
            }

            /* Always move in major axis */
            if (i < dx) {  // Don't step past the end
                if (steep) {
                    y += sy;
                } else {
                    x += sx;
                }
                e += 2 * dy;
                if (i > 0 || e >= 0) {
                    decOss << "Move in x direction\n";
                    decOss << "e = e + 2*dy = " << (e - 2*dy) << " + " << (2*dy)
                           << " = " << e;
                }
            }

            if (i > 0) {
                oss << decOss.str() << "\n";
                oss << "Plot pixel " << pixel.toPixelString();
            }

            step.explanation = oss.str();
            step.decision = yChanged ? "Y changed" : "Y unchanged";

            steps.push_back(step);
            allPoints.push_back(pixel);
        }
    }
};

#endif // GLIDE_BRESENHAM_LINE_H
