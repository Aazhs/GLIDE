/*
 * MidpointCircle.h
 * ----------------
 * Midpoint Circle Drawing Algorithm with 8-way symmetry.
 *
 * Algorithm based on the course notes:
 *
 * It uses the eight-way symmetry of the circle to generate it.
 * To draw 1/8th part of the circle we take unit steps in the
 * positive x direction and make use of a decision variable to
 * determine the next pixel positions.
 *
 * CIRCLE FUNCTION:
 *   f_circle(x, y) = x^2 + y^2 - r^2
 *
 * The decision variable d_i is evaluated at the midpoint between
 * the two candidate pixels:
 *   d_i = f_circle(x_i + 1, y_i - 1/2)
 *       = (x_i + 1)^2 + (y_i - 1/2)^2 - r^2
 *       = (x_i + 1)^2 + y_i^2 - y_i + 1/4 - r^2        ...(1)
 *
 * INITIAL VALUE:
 *   Start position: (x0, y0) = (0, r)
 *   d0 = f_circle(0 + 1, r - 1/2)
 *      = 1 + r^2 - r + 1/4 - r^2
 *      = 5/4 - r
 *      = 1.25 - r
 *
 * INCREMENTAL CALCULATION:
 *   For the next step, x_{i+1} = x_i + 1, so:
 *     d_{i+1} = f_circle(x_{i+1} + 1, y_{i+1} - 1/2)
 *
 *   Comparing eq (1) and (2):
 *     d_{i+1} = d_i + 2(x_i + 1) + (y_{i+1}^2 - y_i^2) - (y_{i+1} - y_i)
 *
 * CASE a: d_i < 0
 *   Midpoint is inside the circle — pixel on scanline y_i is closer.
 *   y_{i+1} = y_i
 *   d_{i+1} = d_i + 2(x_i + 1) + 1
 *           = d_i + 2*x_{i+1} + 1
 *
 * CASE b: d_i >= 0
 *   Midpoint is outside or on the circle — pixel y_i - 1 is closer.
 *   y_{i+1} = y_i - 1
 *   d_{i+1} = d_i + 2(x_i + 1) + 1 - 2*y_{i+1}
 *           = d_i + 2*x_{i+1} - 2*y_{i+1} + 1
 *   where: 2*x_{i+1} = 2*x_i + 2
 *          2*y_{i+1} = 2*y_i - 2
 *
 * 8-WAY SYMMETRY:
 * Standard ordering as per course notes:
 *   (-x, -y)  ( y, -x)  (-y,  x)  ( x,  y)
 *   (-x,  y)  (-y, -x)  ( y,  x)  ( x, -y)
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_MIDPOINT_CIRCLE_H
#define GLIDE_MIDPOINT_CIRCLE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include "../Common/Format.h"
#include "../Common/Benchmark.h"
#include <cmath>
#include <sstream>

class MidpointCircle : public DrawingAlgorithm {
private:
    Point center;       // Center of the circle
    float radiusVal;    // Radius r

    /*
     * Generate all 8 symmetric points from a single calculated point.
     *
     * Standard ordering as per course notes:
     *   (-x,-y)  (y,-x)  (-y,x)  (x,y)  (-x,y)  (-y,-x)  (y,x)  (x,-y)
     *
     * Each point is offset by the circle's center.
     */
    std::vector<Point> getSymmetricPoints(int x, int y) const {
        int cx = MathUtils::roundToInt(center.x);
        int cy = MathUtils::roundToInt(center.y);

        std::vector<Point> points;
        points.push_back(Point((float)(cx - x), (float)(cy - y)));   // (-x, -y)
        points.push_back(Point((float)(cx + y), (float)(cy - x)));   // ( y, -x)
        points.push_back(Point((float)(cx - y), (float)(cy + x)));   // (-y,  x)
        points.push_back(Point((float)(cx + x), (float)(cy + y)));   // ( x,  y)
        points.push_back(Point((float)(cx - x), (float)(cy + y)));   // (-x,  y)
        points.push_back(Point((float)(cx - y), (float)(cy - x)));   // (-y, -x)
        points.push_back(Point((float)(cx + y), (float)(cy + x)));   // ( y,  x)
        points.push_back(Point((float)(cx + x), (float)(cy - y)));   // ( x, -y)
        return points;
    }

public:
    MidpointCircle() : radiusVal(0) {
        algorithmName = "Midpoint Circle";
    }

    MidpointCircle(Point center, float radius) : center(center), radiusVal(radius) {
        algorithmName = "Midpoint Circle";
    }

    void setParameters(Point c, float r) {
        center = c;
        radiusVal = r;
    }

    Point getCenter() const { return center; }
    float getRadius() const { return radiusVal; }

    std::string getDescription() const override {
        return "Midpoint Circle uses f_circle(x,y) = x^2+y^2-r^2 evaluated "
               "at the midpoint between candidate pixels. Initial d = 1.25 - r. "
               "If d < 0: move x only, d += 2x+1. "
               "If d >= 0: move x and decrement y, d += 2x-2y+1. "
               "Uses 8-way symmetry for the full circle.";
    }

    /*
     * Execute the Midpoint Circle algorithm.
     *
     * Directly implements the course notes:
     *   Step 1: Read radius r
     *   Step 2: Initialize x = 0, y = r
     *   Step 3: Calculate initial d = 1.25 - r
     *   Step 4: do {
     *             plot (x, y)
     *             if (d < 0):
     *               x = x + 1; y = y;
     *               d = d + 2x + 1
     *             else:
     *               x = x + 1; y = y - 1;
     *               d = d + 2x - 2y + 1
     *           } while (x < y)
     *   Step 5: Determine symmetry points
     *   Step 6: STOP
     */
    void execute() override {
        clear();

        int r = MathUtils::roundToInt(radiusVal);
        if (r <= 0) return;

        /*
         * Step 2: Initialize starting position
         *   x = 0, y = r
         */
        int x = 0;
        int y = r;

        /*
         * Step 3: Calculate initial decision parameter
         *   d = 1.25 - r  (derived from f_circle(1, r - 0.5))
         *
         * We store as float to keep the 1.25 accurate. All later
         * increments are integers, so the .25 offset is preserved.
         */
        float d = 1.25f - (float)r;
        setupOps.floatAdd += 1;

        setupLines.push_back("Centre = " + Format::px(center) + ",  r = " + std::to_string(r));
        setupLines.push_back("f(x,y) = x^2 + y^2 - r^2 at the midpoint");
        setupLines.push_back("Start: x = 0, y = r = " + std::to_string(r));
        setupLines.push_back("d0 = 1.25 - r = " + Format::num(d));
        setupLines.push_back("d < 0 :  x+1,      d += 2x + 1");
        setupLines.push_back("d >= 0:  x+1, y-1, d += 2x - 2y + 1");
        setupLines.push_back("(x, y in the update are the NEW values)");

        int iteration = 0;

        /*
         * Step 4: Loop while x < y (first octant)
         * At each iteration: plot, decide, update.
         */
        do {
            iteration++;

            AlgorithmStep step;
            step.iteration = iteration;
            step.x = (float)x;
            step.y = (float)y;
            step.radius = (float)r;
            step.decisionVariable = d;

            /* Generate all 8 symmetric points */
            std::vector<Point> symPoints = getSymmetricPoints(x, y);
            step.symmetricPoints = symPoints;
            step.plottedPoints = symPoints;
            for (const auto& p : symPoints) {
                allPoints.push_back(p);
            }

            std::ostringstream oss;
            oss << "Octant point " << Format::pt(x, y) << "   (d = " << Format::num(d) << ")\n";
            oss << "Plot it + 7 mirrors (centre added):\n";
            oss << Format::pointRows(symPoints, 4) << "\n";

            float oldD = d;
            int oldX = x, oldY = y;
            std::string action;

            /* Cost: 16 adds for the 8 mirrors, loop test, d < 0 test, x + 1.
             * d is a float here because d0 = 1.25 - r (as in the notes). */
            step.ops.intAdd = 16 + 1;
            step.ops.compare = 2;

            if (d < 0) {
                step.ops.intShift += 1;   // 2x
                step.ops.floatAdd += 2;   // d + 2x + 1
                /*
                 * Case a: d < 0 - midpoint is inside the circle.
                 * Pixel on scanline y_i is closer to the boundary.
                 *   x = x + 1; y stays
                 *   d = d + 2x + 1    (where x is new x = old x + 1)
                 */
                x = x + 1;
                d = d + 2.0f * (float)x + 1.0f;
                oss << "d = " << Format::num(oldD) << " < 0: midpoint inside\n";
                oss << "  x = " << x << ", y stays " << y << "\n";
                oss << "  d = d + 2x + 1 = " << Format::num(oldD) << " + "
                    << (2 * x) << " + 1 = " << Format::num(d) << "\n";
                step.decision = "d < 0: x only (y stays)";
                action = "x+1";
            } else {
                /*
                 * Case b: d >= 0 - midpoint is outside or on the circle.
                 * Pixel y_i - 1 is closer to the boundary.
                 *   x = x + 1; y = y - 1
                 *   d = d + 2x - 2y + 1  (using new x and new y)
                 */
                x = x + 1;
                y = y - 1;
                d = d + 2.0f * (float)x - 2.0f * (float)y + 1.0f;
                step.ops.intAdd += 1;     // y - 1
                step.ops.intShift += 2;   // 2x, 2y
                step.ops.floatAdd += 3;   // d + 2x - 2y + 1
                oss << "d = " << Format::num(oldD) << " >= 0: midpoint outside\n";
                oss << "  x = " << x << ", y = " << y << "\n";
                oss << "  d = d + 2x - 2y + 1\n";
                oss << "    = " << Format::num(oldD) << " + " << (2 * x) << " - "
                    << (2 * y) << " + 1 = " << Format::num(d) << "\n";
                step.decision = "d >= 0: x and y-1";
                action = "x+1 y-1";
            }

            if (x < y) {
                oss << "Next: " << Format::pt(x, y);
            } else {
                oss << "Now x >= y: octant complete.";
            }

            step.explanation = oss.str();
            step.tableCells = { std::to_string(iteration), std::to_string(oldX),
                                std::to_string(oldY), Format::num(oldD), action };
            steps.push_back(step);

        } while (x < y);

        measureCircleQuality(Point((float)MathUtils::roundToInt(center.x),
                                   (float)MathUtils::roundToInt(center.y)), (float)r);
    }

    /* Lean version of the same algorithm, used only for timing */
    static int rasterize(int cx, int cy, int r, int* out) {
        int x = 0, y = r, n = 0;
        float d = 1.25f - (float)r;
        do {
            writeMirrors8(cx, cy, x, y, out + 16 * n);
            n++;
            x++;
            if (d < 0) {
                d += 2.0f * x + 1.0f;
            } else {
                y--;
                d += 2.0f * x - 2.0f * y + 1.0f;
            }
        } while (x < y);
        return n;
    }

    double benchmarkNs() const override {
        std::vector<int> buf(16 * (getStepCount() + 4));
        int cx = MathUtils::roundToInt(center.x), cy = MathUtils::roundToInt(center.y);
        int r = MathUtils::roundToInt(radiusVal);
        return Bench::nsPerCall([&]() {
            int n = rasterize(cx, cy, r, buf.data());
            return n + buf[16 * n - 1];
        });
    }

    std::vector<std::string> getTableColumns() const override {
        return { "#", "x", "y", "d", "next" };
    }
};

#endif // GLIDE_MIDPOINT_CIRCLE_H
