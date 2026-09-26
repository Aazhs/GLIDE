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

        float dx = endPoint.x - startPoint.x;  // Δx
        float dy = endPoint.y - startPoint.y;  // Δy

        /*
         * Determine the number of steps needed.
         * We step along whichever axis has the greater change,
         * so that we never skip a pixel.
         */
        int numSteps = static_cast<int>(
            std::max(MathUtils::absf(dx), MathUtils::absf(dy))
        );

        /* Handle degenerate case: start == end */
        if (numSteps == 0) {
            AlgorithmStep step;
            step.iteration = 1;
            step.x = startPoint.x;
            step.y = startPoint.y;
            step.deltaX = 0;
            step.deltaY = 0;
            step.slope = 0;
            step.explanation = "Start point equals end point. Plot single pixel.";
            step.decision = "Single point";
            step.plottedPoints.push_back(Point(
                (float)MathUtils::roundToInt(startPoint.x),
                (float)MathUtils::roundToInt(startPoint.y)
            ));
            steps.push_back(step);
            allPoints.push_back(step.plottedPoints[0]);
            return;
        }

        /*
         * Calculate the increment per step.
         *
         * From the notes:
         *   If |Δx| >= |Δy|: step x by 1, y increment = m = Δy/Δx
         *   If |Δx| <  |Δy|: step y by 1, x increment = 1/m = Δx/Δy
         *
         * We generalize slightly: the increment is Δx/numSteps and Δy/numSteps.
         * When |Δx| >= |Δy|, numSteps = |Δx|, so xIncrement = ±1 and
         * yIncrement = Δy/|Δx| = ±m. This matches the notes exactly.
         * The sign is preserved naturally from dx and dy.
         */
        float xIncrement = dx / static_cast<float>(numSteps);
        float yIncrement = dy / static_cast<float>(numSteps);

        /* Calculate slope for display purposes */
        float slope = 0.0f;
        bool verticalLine = (MathUtils::absf(dx) < 0.0001f);
        if (!verticalLine) {
            slope = dy / dx;
        }

        /* Determine which axis is major for explanation */
        std::string majorAxis = (MathUtils::absf(dx) >= MathUtils::absf(dy))
                                ? "X" : "Y";

        float x = startPoint.x;
        float y = startPoint.y;

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

            /* Build explanation text */
            std::ostringstream oss;
            if (i == 0) {
                oss << "Starting point: (" << x << ", " << y << ")\n";
                oss << "dx = " << dx << ", dy = " << dy << "\n";
                if (verticalLine) {
                    oss << "Vertical line (slope undefined)\n";
                } else {
                    oss << "Slope m = " << slope << "\n";
                }
                oss << "Major axis: " << majorAxis << "\n";
                oss << "Steps needed: " << numSteps << "\n";
                oss << "x increment = " << xIncrement << "\n";
                oss << "y increment = " << yIncrement;
            } else {
                oss << "Step " << i << ":\n";
                oss << "x = " << (x - xIncrement) << " + " << xIncrement
                    << " = " << x << "\n";
                oss << "y = " << (y - yIncrement) << " + " << yIncrement
                    << " = " << y << "\n";
                oss << "Round to pixel: " << pixel.toPixelString();
            }
            step.explanation = oss.str();

            std::ostringstream decOss;
            decOss << "Plot pixel " << pixel.toPixelString();
            step.decision = decOss.str();

            steps.push_back(step);
            allPoints.push_back(pixel);

            /* Increment for next step */
            x += xIncrement;
            y += yIncrement;
        }
    }
};

#endif // GLIDE_DDA_LINE_H
