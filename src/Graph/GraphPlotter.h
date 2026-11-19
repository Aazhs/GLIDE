/*
 * GraphPlotter.h
 * --------------
 * Connects the expression evaluator to the drawing algorithms.
 *
 * THE GRAPH RENDERING PIPELINE:
 *
 *   User enters function (e.g., "x*x")
 *        ↓
 *   GraphFunction evaluates at many x values
 *        ↓
 *   Generates sample points: P1, P2, P3, ...
 *        ↓
 *   Consecutive points are connected: P1→P2, P2→P3, P3→P4, ...
 *        ↓
 *   Each segment is drawn using the selected line algorithm (DDA or Bresenham)
 *        ↓
 *   Algorithm generates raster pixels
 *        ↓
 *   OpenGL renders the pixels with GL_POINTS
 *
 * This class manages the coordinate system transformation
 * between mathematical world coordinates and screen coordinates,
 * and orchestrates the drawing process.
 */

#ifndef GLIDE_GRAPH_PLOTTER_H
#define GLIDE_GRAPH_PLOTTER_H

#include "GraphFunction.h"
#include "CoordinateSystem.h"
#include "../Algorithms/DDALine.h"
#include "../Algorithms/BresenhamLine.h"
#include "../Common/Point.h"
#include "../Common/MathUtils.h"
#include <vector>
#include <string>

/*
 * Segment stores the algorithm steps for one line segment
 * connecting two consecutive sample points.
 */
struct GraphSegment {
    Point worldStart;       // Start point in world coordinates
    Point worldEnd;         // End point in world coordinates
    Point screenStart;      // Start point in screen coordinates
    Point screenEnd;        // End point in screen coordinates
    std::vector<AlgorithmStep> steps;    // Algorithm steps for this segment
    std::vector<Point> rasterPoints;     // Rasterized pixels
};

class GraphPlotter {
private:
    GraphFunction function;             // The mathematical function
    CoordinateSystem coordSystem;       // Coordinate transformation
    std::vector<Point> samplePoints;    // Sampled world-coordinate points
    std::vector<GraphSegment> segments; // All line segments
    std::vector<Point> allRasterPoints; // All rasterized pixels
    int sampleCount;                    // Number of sample points
    bool useDDA;                        // true = DDA, false = Bresenham
    bool hasGraph;                      // Whether a graph has been computed

    /*
     * Liang-Barsky line clipping against the plot rectangle (expanded by a
     * small margin). Modifies the endpoints in place.
     * Returns false if the segment lies completely outside.
     */
    bool clipToPlotArea(float& x1, float& y1, float& x2, float& y2) const {
        const float margin = 2.0f;
        float xmin = coordSystem.plotAreaX - margin;
        float ymin = coordSystem.plotAreaY - margin;
        float xmax = (float)(coordSystem.plotAreaX + coordSystem.plotAreaW) + margin;
        float ymax = (float)(coordSystem.plotAreaY + coordSystem.plotAreaH) + margin;

        float dx = x2 - x1, dy = y2 - y1;
        float p[4] = { -dx, dx, -dy, dy };
        float q[4] = { x1 - xmin, xmax - x1, y1 - ymin, ymax - y1 };
        float t0 = 0.0f, t1 = 1.0f;

        for (int i = 0; i < 4; i++) {
            if (p[i] == 0.0f) {
                if (q[i] < 0.0f) return false;      // parallel and outside
            } else {
                float t = q[i] / p[i];
                if (p[i] < 0.0f) { if (t > t1) return false; if (t > t0) t0 = t; }
                else             { if (t < t0) return false; if (t < t1) t1 = t; }
            }
        }

        float nx1 = x1 + t0 * dx, ny1 = y1 + t0 * dy;
        float nx2 = x1 + t1 * dx, ny2 = y1 + t1 * dy;
        x1 = nx1; y1 = ny1; x2 = nx2; y2 = ny2;
        return true;
    }

public:
    GraphPlotter() : sampleCount(200), useDDA(true), hasGraph(false) {}

    /* Set the mathematical function */
    void setFunction(const std::string& expr) {
        function.setExpression(expr);
    }

    /* Set the line drawing algorithm */
    void setAlgorithm(bool useDDAAlgorithm) {
        useDDA = useDDAAlgorithm;
    }

    /* Set sample count */
    void setSampleCount(int count) {
        if (count < 10) count = 10;
        if (count > 1000) count = 1000;
        sampleCount = count;
    }

    /* Get coordinate system (for rendering) */
    CoordinateSystem& getCoordSystem() { return coordSystem; }
    const CoordinateSystem& getCoordSystem() const { return coordSystem; }

    /* Get function object */
    const GraphFunction& getFunction() const { return function; }
    bool isUsingDDA() const { return useDDA; }
    int getSampleCount() const { return sampleCount; }
    bool graphExists() const { return hasGraph; }

    /* Get segments for visualization */
    const std::vector<GraphSegment>& getSegments() const { return segments; }
    const std::vector<Point>& getAllRasterPoints() const { return allRasterPoints; }
    const std::vector<Point>& getSamplePoints() const { return samplePoints; }

    /*
     * Compute the graph.
     *
     * This implements the rendering pipeline:
     *   1. Sample the function at many x values
     *   2. Convert each sample point to screen coordinates
     *   3. Connect consecutive valid points with the selected line algorithm
     *   4. Collect all rasterized pixels
     */
    void computeGraph() {
        segments.clear();
        allRasterPoints.clear();
        hasGraph = false;

        if (!function.isValid()) return;

        /* Step 1: Sample the function across the visible x range */
        float xMin = coordSystem.getVisibleMinX();
        float xMax = coordSystem.getVisibleMaxX();
        samplePoints = function.generatePoints(xMin, xMax, sampleCount);

        if (samplePoints.size() < 2) return;

        /*
         * Step 2 & 3: Connect consecutive valid points.
         * Skip segments where either point has an invalid y (NaN).
         */
        for (size_t i = 0; i + 1 < samplePoints.size(); i++) {
            Point& p1 = samplePoints[i];
            Point& p2 = samplePoints[i + 1];

            /* Skip if either point is invalid */
            if (!MathUtils::isValidFloat(p1.y) || !MathUtils::isValidFloat(p2.y)) {
                continue;
            }

            /* Convert world coordinates to screen coordinates */
            float sx1 = coordSystem.worldToScreenX(p1.x);
            float sy1 = coordSystem.worldToScreenY(p1.y);
            float sx2 = coordSystem.worldToScreenX(p2.x);
            float sy2 = coordSystem.worldToScreenY(p2.y);

            /*
             * Asymptote check: tan(x), 1/x etc. jump from +huge to -huge
             * between two samples. Joining them would draw a false vertical
             * line, so treat a jump taller than the whole plot as a break.
             */
            if (fabsf(sy2 - sy1) > 2.0f * coordSystem.plotAreaH &&
                ((sy1 < coordSystem.plotAreaY && sy2 > coordSystem.plotAreaY + coordSystem.plotAreaH) ||
                 (sy2 < coordSystem.plotAreaY && sy1 > coordSystem.plotAreaY + coordSystem.plotAreaH))) {
                continue;
            }

            /*
             * Clip the segment to the plot area (Liang-Barsky).
             * Without this, a point far off-screen (e.g. x^9 at x = 10)
             * would make the line algorithm walk millions of pixels.
             */
            if (!clipToPlotArea(sx1, sy1, sx2, sy2)) continue;

            Point screenP1(sx1, sy1);
            Point screenP2(sx2, sy2);

            /* Skip very short segments (would generate 0-1 pixels) */
            if (screenP1.distanceTo(screenP2) < 0.5f) continue;

            GraphSegment seg;
            seg.worldStart = p1;
            seg.worldEnd = p2;
            seg.screenStart = screenP1;
            seg.screenEnd = screenP2;

            /*
             * Step 4: Use the selected line algorithm to rasterize this segment.
             * This is the core idea of GLIDE: the graph is built from
             * many small rasterized line segments.
             */
            if (useDDA) {
                DDALine algo(screenP1, screenP2);
                algo.execute();
                seg.steps = algo.getSteps();
                seg.rasterPoints = std::vector<Point>(
                    algo.getAllPoints().begin(), algo.getAllPoints().end()
                );
            } else {
                BresenhamLine algo(screenP1, screenP2);
                algo.execute();
                seg.steps = algo.getSteps();
                seg.rasterPoints = std::vector<Point>(
                    algo.getAllPoints().begin(), algo.getAllPoints().end()
                );
            }

            /* Collect all raster points */
            for (const auto& pt : seg.rasterPoints) {
                allRasterPoints.push_back(pt);
            }

            segments.push_back(seg);
        }

        hasGraph = true;
    }

    /* Evaluate the current function at x (NaN if undefined) */
    float evaluateAt(float x) {
        if (!function.isValid()) return std::numeric_limits<float>::quiet_NaN();
        try {
            return function.evaluate(x);
        } catch (...) {
            return std::numeric_limits<float>::quiet_NaN();
        }
    }

    /* Clear the graph */
    void clearGraph() {
        segments.clear();
        allRasterPoints.clear();
        samplePoints.clear();
        hasGraph = false;
    }
};

#endif // GLIDE_GRAPH_PLOTTER_H
