/*
 * DDACircle.h
 * -----------
 * DDA Circle Drawing Algorithm with 8-way symmetry.
 *
 * Algorithm based on the course notes:
 *
 * A circle of radius r centered at origin is parametrically:
 *   x = r cos θ
 *   y = r sin θ
 *
 * Taking derivatives with respect to θ:
 *   dx/dθ = -r sin θ = -y
 *   dy/dθ =  r cos θ =  x
 *
 * Therefore the incremental changes are:
 *   dx = -y * dθ
 *   dy =  x * dθ
 *
 * Choosing dθ = 1/r ensures that each step moves approximately
 * one pixel along the circle arc (since arc length = r * dθ = 1).
 *
 * At each step:
 *   x_new = x + dx = x + (-y * dθ) = x - y/r
 *   y_new = y + dy = y + ( x * dθ) = y + x/r
 *
 * 8-WAY SYMMETRY:
 * The circle is symmetric across both axes and both diagonals.
 * We only need to trace the first octant (θ from 0 to π/4,
 * where x > y >= 0) and reflect each computed point to obtain
 * all 8 symmetric points. The standard ordering starting from
 * the top-left quadrant moving clockwise:
 *
 *   (-x, -y)   ( y, -x)   (-y,  x)   ( x,  y)
 *   (-x,  y)   (-y, -x)   ( y,  x)   ( x, -y)
 *
 * Start at θ = 0: x = r, y = 0
 * Step until x < y (end of first octant, θ > π/4)
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_DDA_CIRCLE_H
#define GLIDE_DDA_CIRCLE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include "../Common/Format.h"
#include "../Common/Benchmark.h"
#include <cmath>
#include <sstream>

class DDACircle : public DrawingAlgorithm {
private:
    Point center;       // Center of the circle
    float radius;       // Radius r

    /*
     * Generate all 8 symmetric points from a single first-octant point.
     * Each point is offset by the circle center.
     *
     * Ordering follows the standard 8-way symmetry as per course notes:
     *   (-x, -y)  top-left quadrant
     *   ( y, -x)  upper-right reflected
     *   (-y,  x)  upper-left reflected
     *   ( x,  y)  computed point (first octant)
     *   (-x,  y)  left reflection
     *   (-y, -x)  lower-left reflected
     *   ( y,  x)  diagonal reflection
     *   ( x, -y)  lower-right reflection
     */
    std::vector<Point> getSymmetricPoints(int x, int y) const {
        int cx = MathUtils::roundToInt(center.x);
        int cy = MathUtils::roundToInt(center.y);

        std::vector<Point> pts;
        pts.push_back(Point((float)(cx - x), (float)(cy - y)));   // (-x, -y)
        pts.push_back(Point((float)(cx + y), (float)(cy - x)));   // ( y, -x)
        pts.push_back(Point((float)(cx - y), (float)(cy + x)));   // (-y,  x)
        pts.push_back(Point((float)(cx + x), (float)(cy + y)));   // ( x,  y)
        pts.push_back(Point((float)(cx - x), (float)(cy + y)));   // (-x,  y)
        pts.push_back(Point((float)(cx - y), (float)(cy - x)));   // (-y, -x)
        pts.push_back(Point((float)(cx + y), (float)(cy + x)));   // ( y,  x)
        pts.push_back(Point((float)(cx + x), (float)(cy - y)));   // ( x, -y)
        return pts;
    }

public:
    DDACircle() : radius(0) {
        algorithmName = "DDA Circle";
    }

    DDACircle(Point center, float radius) : center(center), radius(radius) {
        algorithmName = "DDA Circle";
    }

    void setParameters(Point c, float r) {
        center = c;
        radius = r;
    }

    Point getCenter() const { return center; }
    float getRadius() const { return radius; }

    std::string getDescription() const override {
        return "DDA Circle uses the parametric form x = r*cos(theta), "
               "y = r*sin(theta) with incremental steps dx = -y*dtheta, "
               "dy = x*dtheta (dtheta = 1/r) to trace the first octant, "
               "then exploits 8-way symmetry to fill the full circle.";
    }

    /*
     * Execute the DDA Circle algorithm with 8-way symmetry.
     *
     * Trace only the first octant (x >= y >= 0, θ from 0 to π/4)
     * using the DDA incremental step:
     *   dθ = 1/r
     *   dx = -y * dθ
     *   dy =  x * dθ
     *
     * At each step, reflect the computed (x, y) to 8 symmetric points.
     */
    void execute() override {
        clear();

        if (radius <= 0) return;

        /*
         * dtheta = 1/r
         * One radian step that advances approximately 1 pixel along the arc.
         */
        float dTheta = 1.0f / radius;
        setupOps.floatDiv += 1;

        setupLines.push_back("Centre = " + Format::px(center) + ",  r = " + Format::num(radius));
        setupLines.push_back("dtheta = 1 / r = " + Format::num(dTheta, 4) + " rad");
        setupLines.push_back("Start at theta = 0:  x = r, y = 0");
        setupLines.push_back("Update:  x' = x - y*dtheta");
        setupLines.push_back("         y' = y + x*dtheta");
        setupLines.push_back("Repeat while x >= y (first octant),");
        setupLines.push_back("mirror each point into all 8 octants.");

        /*
         * Start at theta = 0: x = r, y = 0
         * Step through the first octant only (stop when x < y, i.e. theta > pi/4)
         */
        float x = radius;
        float y = 0.0f;
        float theta = 0.0f;

        int iteration = 0;
        float lastX = x, lastY = y;   // last plotted (unrounded) point, for drift

        /*
         * First octant: x >= y >= 0
         * We stop when the rounded x drops below the rounded y,
         * which marks the end of the first octant (45 degrees).
         */
        while (MathUtils::roundToInt(x) >= MathUtils::roundToInt(y)) {
            iteration++;

            int ix = MathUtils::roundToInt(x);
            int iy = MathUtils::roundToInt(y);

            AlgorithmStep step;
            step.iteration = iteration;
            step.x = (float)ix;
            step.y = (float)iy;
            step.radius = radius;
            step.theta = theta;
            step.dTheta = dTheta;

            /*
             * DDA incremental step (computed before updating x, y):
             *   dx = -y * dtheta
             *   dy =  x * dtheta
             */
            float dx = -y * dTheta;
            float dy =  x * dTheta;
            step.deltaX = dx;
            step.deltaY = dy;
            lastX = x;
            lastY = y;

            /* Cost: round x and y, loop test x >= y, two multiplies, two adds,
             * and 16 integer adds to place the 8 mirrored points */
            step.ops.rounds = 2;
            step.ops.compare = 1;
            step.ops.floatMul = 2;
            step.ops.floatAdd = 2;
            step.ops.intAdd = 16;

            /* Generate and store the 8 symmetric points */
            std::vector<Point> symPoints = getSymmetricPoints(ix, iy);
            step.symmetricPoints = symPoints;
            step.plottedPoints   = symPoints;
            for (const auto& p : symPoints) {
                allPoints.push_back(p);
            }

            /*
             * Advance to next point:
             *   x_new = x + dx = x - y * dtheta
             *   y_new = y + dy = y + x * dtheta
             *
             * Use current x to compute dy (not updated x) - per course notes.
             */
            float newX = x + dx;
            float newY = y + dy;

            /* Build explanation */
            std::ostringstream oss;
            oss << "theta = " << Format::num(MathUtils::radToDeg(theta), 2) << " deg\n";
            oss << "x = " << Format::num(x) << ",  y = " << Format::num(y) << "\n";
            oss << "Round -> octant point " << Format::pt(ix, iy) << "\n";
            oss << "Plot it + 7 mirrors (centre added):\n";
            oss << Format::pointRows(symPoints, 4) << "\n";
            if (MathUtils::roundToInt(newX) >= MathUtils::roundToInt(newY)) {
                oss << "x' = " << Format::num(x) << " - " << Format::num(y) << "*"
                    << Format::num(dTheta, 4) << " = " << Format::num(newX) << "\n";
                oss << "y' = " << Format::num(y) << " + " << Format::num(x) << "*"
                    << Format::num(dTheta, 4) << " = " << Format::num(newY);
            } else {
                oss << "Next point would have x < y:\noctant complete.";
            }

            step.explanation = oss.str();
            step.decision = "Plot 8 symmetric points from (" +
                            std::to_string(ix) + ", " + std::to_string(iy) + ")";
            step.tableCells = { std::to_string(iteration),
                                Format::num(MathUtils::radToDeg(theta), 1),
                                Format::num(x, 2), Format::num(y, 2), Format::pt(ix, iy) };

            steps.push_back(step);

            x = newX;
            y = newY;
            theta += dTheta;
        }

        /*
         * x' = x - y*dtheta, y' = y + x*dtheta grows the radius slightly at
         * every step (by a factor sqrt(1 + dtheta^2)), so DDA slowly spirals
         * outwards. Report how far it drifted by the end of the octant.
         */
        quality.hasDrift = true;
        quality.drift = sqrtf(lastX * lastX + lastY * lastY) - radius;
        measureCircleQuality(Point((float)MathUtils::roundToInt(center.x),
                                   (float)MathUtils::roundToInt(center.y)), radius);
    }

    /* Lean version of the same algorithm, used only for timing */
    static int rasterize(int cx, int cy, float r, int* out) {
        float dt = 1.0f / r;
        float x = r, y = 0.0f;
        int n = 0;
        while (true) {
            int ix = (int)roundf(x), iy = (int)roundf(y);
            if (ix < iy) break;
            writeMirrors8(cx, cy, ix, iy, out + 16 * n);
            n++;
            float nx = x - y * dt;
            float ny = y + x * dt;
            x = nx;
            y = ny;
        }
        return n;
    }

    double benchmarkNs() const override {
        std::vector<int> buf(16 * (getStepCount() + 4));
        int cx = MathUtils::roundToInt(center.x), cy = MathUtils::roundToInt(center.y);
        float r = radius;
        return Bench::nsPerCall([&]() {
            int n = rasterize(cx, cy, r, buf.data());
            return n + buf[16 * n - 1];
        });
    }

    std::vector<std::string> getTableColumns() const override {
        return { "#", "deg", "x", "y", "point" };
    }
};

#endif // GLIDE_DDA_CIRCLE_H
