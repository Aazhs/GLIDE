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
 *   Plot current pixel
 *   If e >= 0:
 *     y = y + sy    (move in minor axis direction)
 *     e = e - 2Δx
 *   x = x + sx      (always move in major axis direction)
 *   e = e + 2Δy
 *
 * Continue until x reaches x2 (dx+1 total pixels).
 *
 * KEY INSIGHT: Bresenham's algorithm uses only integer arithmetic
 * (additions and comparisons), making it very efficient compared
 * to DDA which requires floating-point operations.
 *
 * ALL-OCTANT HANDLING:
 * The core algorithm always walks along the major axis (the one
 * with the larger delta). We determine:
 *   - steep: whether |Δy| > |Δx| (y is major axis)
 *   - sx, sy: step directions (+1 or -1) for each axis
 *
 * When steep, we swap the roles of x and y so the core loop
 * always increments the major axis. After swapping dx/dy, the
 * variables 'x' and 'y' in the loop represent the major and minor
 * axes respectively (not screen x and y). We track actual screen
 * coordinates separately via (curX, curY).
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_BRESENHAM_LINE_H
#define GLIDE_BRESENHAM_LINE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include "../Common/Format.h"
#include "../Common/Benchmark.h"
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
               "decides whether to step in the minor-axis direction.";
    }

    /*
     * Execute the Bresenham Line algorithm.
     *
     * Handles all 8 octants via:
     *   - sx, sy: per-axis step directions
     *   - steep flag: swap major/minor roles when |dy| > |dx|
     *
     * The actual screen pixel is always (curX, curY) which is
     * updated directly using sx/sy — no reverse-mapping needed.
     */
    void execute() override {
        clear();

        int x1 = MathUtils::roundToInt(startPoint.x);
        int y1 = MathUtils::roundToInt(startPoint.y);
        int x2 = MathUtils::roundToInt(endPoint.x);
        int y2 = MathUtils::roundToInt(endPoint.y);

        setupLines.push_back("P1 = " + Format::pt(x1, y1) + "   P2 = " + Format::pt(x2, y2));

        /* Handle single point */
        if (x1 == x2 && y1 == y2) {
            AlgorithmStep step;
            step.iteration = 1;
            step.x = (float)x1;
            step.y = (float)y1;
            step.explanation = "P1 equals P2, so the line is a\nsingle pixel: plot " +
                               Format::pt(x1, y1);
            step.decision = "Plot " + Format::pt(x1, y1);
            step.plottedPoints.push_back(Point((float)x1, (float)y1));
            step.tableCells = { "1", Format::pt(x1, y1), "-", "done" };
            step.ops.compare = 2;
            steps.push_back(step);
            allPoints.push_back(Point((float)x1, (float)y1));
            measureLineQuality(startPoint, endPoint);
            return;
        }

        int adx = abs(x2 - x1);
        int ady = abs(y2 - y1);

        /*
         * Step directions for each screen axis.
         */
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;

        /*
         * Determine major axis.
         * If steep (|dy| > |dx|), y is the major axis.
         * We swap adx/ady so the loop variable 'dx' always
         * counts steps along the major axis.
         */
        bool steep = (ady > adx);
        int dx = steep ? ady : adx;   // major-axis delta (loop count)
        int dy = steep ? adx : ady;   // minor-axis delta

        /* Names used in the explanation text */
        const char* majName = steep ? "y" : "x";
        const char* minName = steep ? "x" : "y";
        int majStep = steep ? sy : sx;
        int minStep = steep ? sx : sy;
        std::string majD = std::string("d") + majName;   // e.g. "dx"
        std::string minD = std::string("d") + minName;   // e.g. "dy"

        /*
         * Initial decision variable: e = 2*dy - dx
         * (using the major-axis-normalised dx/dy)
         */
        int e = 2 * dy - dx;

        /* Setup cost: 2 subtractions, abs/sign/steep tests, e0 = 2dy - dx,
         * and the constants 2dy and 2dx (shifts) used inside the loop. */
        setupOps.intAdd += 3;
        setupOps.compare += 3;
        setupOps.intShift += 3;

        setupLines.push_back("dx = |x2 - x1| = " + std::to_string(adx) +
                             ",  dy = |y2 - y1| = " + std::to_string(ady));
        if (steep) {
            setupLines.push_back("dy > dx: steep line, so y is the major");
            setupLines.push_back("axis and the roles of x and y swap.");
        } else {
            setupLines.push_back("dx >= dy: x is the major axis.");
        }
        setupLines.push_back("Step signs: sx = " + Format::sgn(sx) + ", sy = " + Format::sgn(sy));
        setupLines.push_back("e0 = 2*" + minD + " - " + majD + " = 2*" + std::to_string(dy) +
                             " - " + std::to_string(dx) + " = " + std::to_string(e));
        setupLines.push_back("2*" + minD + " = " + std::to_string(2 * dy) +
                             ",  2*" + majD + " = " + std::to_string(2 * dx));

        /* Current screen position */
        int curX = x1;
        int curY = y1;

        for (int i = 0; i <= dx; i++) {
            AlgorithmStep step;
            step.iteration = i + 1;
            step.deltaX = (float)adx;
            step.deltaY = (float)ady;
            step.decisionVariable = (float)e;
            step.x = (float)curX;
            step.y = (float)curY;

            Point pixel((float)curX, (float)curY);
            step.plottedPoints.push_back(pixel);

            std::ostringstream oss;
            oss << "Plot " << pixel.toPixelString() << "   (e = " << e << ")\n";

            std::string action = "done";
            bool minorStepped = false;
            step.ops.compare = 1;   // loop test

            /* Update e and position for next pixel */
            if (i < dx) {
                int eBefore = e;
                step.ops.compare += 1;      // e >= 0 ?
                if (e >= 0) {
                    step.ops.intAdd += 2;   // minor step, e -= 2dx
                    /* Step minor axis */
                    if (steep) {
                        curX += sx;   // minor axis is x when steep
                    } else {
                        curY += sy;   // minor axis is y when not steep
                    }
                    minorStepped = true;
                    e -= 2 * dx;
                    oss << "e = " << eBefore << " >= 0  ->  step " << minName
                        << " by " << Format::sgn(minStep) << "\n";
                    oss << "  e = e - 2*" << majD << " = " << eBefore << " - "
                        << (2 * dx) << " = " << e << "\n";
                } else {
                    oss << "e = " << eBefore << " < 0  ->  " << minName << " stays\n";
                }

                /* Always step major axis */
                if (steep) {
                    curY += sy;   // major axis is y when steep
                } else {
                    curX += sx;   // major axis is x when not steep
                }
                int eMid = e;
                e += 2 * dy;
                step.ops.intAdd += 2;       // major step, e += 2dy
                oss << "Always step " << majName << " by " << Format::sgn(majStep) << "\n";
                oss << "  e = e + 2*" << minD << " = " << eMid << " + "
                    << (2 * dy) << " = " << e << "\n";
                oss << "Next pixel: " << Format::pt(curX, curY);

                action = std::string(majName) + Format::sgn(majStep);
                if (minorStepped) action += " " + std::string(minName) + Format::sgn(minStep);
            } else {
                oss << "Reached P2 - line complete.";
            }

            step.explanation = oss.str();
            step.decision = minorStepped ? "Minor axis stepped" : "Major axis only";
            step.tableCells = { std::to_string(i + 1), pixel.toPixelString(),
                                std::to_string((int)step.decisionVariable), action };

            steps.push_back(step);
            allPoints.push_back(pixel);
        }

        measureLineQuality(startPoint, endPoint);
    }

    /*
     * Lean version of the same algorithm, used only for timing.
     * Integer additions and comparisons only.
     */
    static int rasterize(int x1, int y1, int x2, int y2, int* out) {
        int adx = abs(x2 - x1), ady = abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
        bool steep = ady > adx;
        int dx = steep ? ady : adx, dy = steep ? adx : ady;
        int twoDx = 2 * dx, twoDy = 2 * dy;
        int e = twoDy - dx;
        int x = x1, y = y1;
        for (int i = 0; i <= dx; i++) {
            out[2 * i] = x;
            out[2 * i + 1] = y;
            if (e >= 0) {
                if (steep) x += sx; else y += sy;
                e -= twoDx;
            }
            if (steep) y += sy; else x += sx;
            e += twoDy;
        }
        return dx + 1;
    }

    double benchmarkNs() const override {
        std::vector<int> buf(2 * (getStepCount() + 2));
        int x1 = MathUtils::roundToInt(startPoint.x), y1 = MathUtils::roundToInt(startPoint.y);
        int x2 = MathUtils::roundToInt(endPoint.x), y2 = MathUtils::roundToInt(endPoint.y);
        return Bench::nsPerCall([&]() {
            int n = rasterize(x1, y1, x2, y2, buf.data());
            return n + buf[2 * (n - 1)];
        });
    }

    std::vector<std::string> getTableColumns() const override {
        return { "#", "plot", "e", "next move" };
    }
};

#endif // GLIDE_BRESENHAM_LINE_H
