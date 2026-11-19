/*
 * DDALine.h
 * ---------
 * DDA (Digital Differential Analyzer) Line Drawing Algorithm.
 *
 * Algorithm based on the course notes:
 *
 * Given two endpoints (x1,y1) and (x2,y2):
 *   1. Calculate Δx = x2 - x1 and Δy = y2 - y1
 *   2. Calculate slope m = Δy / Δx
 *   3. Compare |Δx| and |Δy| to find the major axis
 *
 *   If |Δx| >= |Δy| (x is the major axis):
 *     Step along x by 1 (or -1 for negative direction)
 *     x(i+1) = x(i) + 1
 *     y(i+1) = y(i) + m
 *
 *   If |Δx| < |Δy| (y is the major axis):
 *     Step along y by 1 (or -1 for negative direction)
 *     y(i+1) = y(i) + 1
 *     x(i+1) = x(i) + 1/m
 *
 * The key insight: by stepping 1 unit along the major axis,
 * the increment along the minor axis is always <= 1,
 * so no pixels are skipped.
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_DDA_LINE_H
#define GLIDE_DDA_LINE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include "../Common/Format.h"
#include "../Common/Benchmark.h"
#include <cmath>
#include <sstream>

class DDALine : public DrawingAlgorithm {
private:
    Point startPoint;   // Starting endpoint (x1, y1)
    Point endPoint;     // Ending endpoint (x2, y2)

public:
    DDALine() {
        algorithmName = "DDA Line";
    }

    DDALine(Point start, Point end) : startPoint(start), endPoint(end) {
        algorithmName = "DDA Line";
    }

    /* Set the line endpoints */
    void setEndpoints(Point start, Point end) {
        startPoint = start;
        endPoint = end;
    }

    Point getStart() const { return startPoint; }
    Point getEnd() const { return endPoint; }

    std::string getDescription() const override {
        return "DDA (Digital Differential Analyzer) uses incremental "
               "calculation to find successive points on a line. "
               "It steps along the major axis by 1 unit and calculates "
               "the corresponding minor axis increment using the slope.";
    }

    /*
     * Execute the DDA Line algorithm.
     *
     * This follows the course notes exactly:
     * - Find slope m = Δy/Δx
     * - Determine major axis by comparing |Δx| and |Δy|
     * - Step along major axis by 1, increment minor axis accordingly
     * - Round to nearest integer for pixel position
     *
     * The direction handling (negative slopes, reversed endpoints)
     * uses the sign of Δx/Δy to determine step direction, but the
     * core algorithm concept remains unchanged from the notes.
     */
    void execute() override {
        clear();

        float dx = endPoint.x - startPoint.x;  // dx = x2 - x1
        float dy = endPoint.y - startPoint.y;  // dy = y2 - y1

        /*
         * Determine the number of steps needed.
         * We step along whichever axis has the greater change,
         * so that we never skip a pixel.
         */
        int numSteps = static_cast<int>(
            std::max(MathUtils::absf(dx), MathUtils::absf(dy))
        );

        /* Setup cost: two subtractions, one comparison to find the major axis */
        setupOps.floatAdd += 2;
        setupOps.compare += 1;

        bool verticalLine = (MathUtils::absf(dx) < 0.0001f);
        float slope = verticalLine ? 0.0f : dy / dx;
        std::string majorAxis = (MathUtils::absf(dx) >= MathUtils::absf(dy)) ? "x" : "y";

        setupLines.push_back("P1 = " + Format::ptf(startPoint.x, startPoint.y) +
                             "   P2 = " + Format::ptf(endPoint.x, endPoint.y));
        setupLines.push_back("dx = x2 - x1 = " + Format::num(dx) +
                             ",  dy = y2 - y1 = " + Format::num(dy));
        setupLines.push_back(verticalLine ? std::string("m = undefined (vertical line)")
                                          : "m = dy / dx = " + Format::num(slope));
        setupLines.push_back("|dx| " + std::string(majorAxis == "x" ? ">=" : "<") +
                             " |dy|  ->  major axis is " + majorAxis);
        setupLines.push_back("steps = max(|dx|, |dy|) = " + std::to_string(numSteps));

        /* Handle degenerate case: start == end */
        if (numSteps == 0) {
            AlgorithmStep step;
            step.iteration = 1;
            step.x = startPoint.x;
            step.y = startPoint.y;
            Point pixel((float)MathUtils::roundToInt(startPoint.x),
                        (float)MathUtils::roundToInt(startPoint.y));
            step.explanation = "P1 equals P2, so the line is a\nsingle pixel: plot " +
                               Format::px(pixel);
            step.decision = "Plot " + Format::px(pixel);
            step.plottedPoints.push_back(pixel);
            step.tableCells = { "1", Format::num(step.x), Format::num(step.y), Format::px(pixel) };
            step.ops.rounds = 2;
            steps.push_back(step);
            allPoints.push_back(pixel);
            measureLineQuality(startPoint, endPoint);
            return;
        }

        /*
         * Calculate the increment per step.
         *
         * From the notes:
         *   If |dx| >= |dy|: step x by 1, y increment = m = dy/dx
         *   If |dx| <  |dy|: step y by 1, x increment = 1/m = dx/dy
         *
         * We generalize slightly: the increment is dx/numSteps and dy/numSteps.
         * When |dx| >= |dy|, numSteps = |dx|, so xIncrement = +-1 and
         * yIncrement = dy/|dx| = +-m. This matches the notes exactly.
         * The sign is preserved naturally from dx and dy.
         */
        float xIncrement = dx / static_cast<float>(numSteps);
        float yIncrement = dy / static_cast<float>(numSteps);
        setupOps.floatDiv += 2;
        setupLines.push_back("x_inc = dx / steps = " + Format::num(xIncrement));
        setupLines.push_back("y_inc = dy / steps = " + Format::num(yIncrement));

        float x = startPoint.x;
        float y = startPoint.y;
        float prevX = x, prevY = y;

        /*
         * Generate points: iterate from step 0 to numSteps.
         * At each step, round (x, y) to the nearest pixel and record it.
         */
        for (int i = 0; i <= numSteps; i++) {
            AlgorithmStep step;
            step.iteration = i + 1;
            step.x = x;
            step.y = y;
            step.deltaX = dx;
            step.deltaY = dy;
            step.slope = slope;

            /* The plotted pixel is the rounded position */
            Point pixel(
                (float)MathUtils::roundToInt(x),
                (float)MathUtils::roundToInt(y)
            );
            step.plottedPoints.push_back(pixel);

            /* Cost of this iteration: round x and y, loop test, then x += , y += */
            step.ops.rounds = 2;
            step.ops.compare = 1;
            if (i < numSteps) step.ops.floatAdd = 2;

            /* Build explanation text */
            std::ostringstream oss;
            if (i == 0) {
                oss << "Start at P1:\n";
                oss << "  x = " << Format::num(x) << ",  y = " << Format::num(y) << "\n";
            } else {
                oss << "Add the increments:\n";
                oss << "  x = " << Format::num(prevX) << " + " << Format::num(xIncrement)
                    << " = " << Format::num(x) << "\n";
                oss << "  y = " << Format::num(prevY) << " + " << Format::num(yIncrement)
                    << " = " << Format::num(y) << "\n";
            }
            oss << "Round to nearest pixel:\n";
            oss << "  (round(" << Format::num(x) << "), round(" << Format::num(y)
                << ")) = " << Format::px(pixel);
            if (i == numSteps) {
                oss << "\nReached P2 - line complete.";
            }
            step.explanation = oss.str();
            step.decision = "Plot " + Format::px(pixel);
            step.tableCells = { std::to_string(i + 1), Format::num(x), Format::num(y),
                                Format::px(pixel) };

            steps.push_back(step);
            allPoints.push_back(pixel);

            /* Increment for next step */
            prevX = x;
            prevY = y;
            x += xIncrement;
            y += yIncrement;
        }

        measureLineQuality(startPoint, endPoint);
    }

    /*
     * Lean version of the same algorithm, used only for timing.
     * Writes (x, y) pairs to 'out' and returns the number of pixels.
     */
    static int rasterize(float x1, float y1, float x2, float y2, int* out) {
        float dx = x2 - x1, dy = y2 - y1;
        int n = (int)std::max(fabsf(dx), fabsf(dy));
        if (n == 0) {
            out[0] = (int)roundf(x1); out[1] = (int)roundf(y1);
            return 1;
        }
        float xi = dx / n, yi = dy / n;
        float x = x1, y = y1;
        for (int i = 0; i <= n; i++) {
            out[2 * i]     = (int)roundf(x);
            out[2 * i + 1] = (int)roundf(y);
            x += xi;
            y += yi;
        }
        return n + 1;
    }

    double benchmarkNs() const override {
        std::vector<int> buf(2 * (getStepCount() + 2));
        Point a = startPoint, b = endPoint;
        return Bench::nsPerCall([&]() {
            int n = rasterize(a.x, a.y, b.x, b.y, buf.data());
            return n + buf[2 * (n - 1)];
        });
    }

    std::vector<std::string> getTableColumns() const override {
        return { "#", "x", "y", "plot" };
    }
};

#endif // GLIDE_DDA_LINE_H
