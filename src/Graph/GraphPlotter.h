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
#include "../Algorithms/DDALine.h"
#include "../Algorithms/BresenhamLine.h"
#include "../Common/Point.h"
#include "../Common/MathUtils.h"
#include <vector>
#include <string>

/*
 * CoordinateSystem handles the conversion between:
 *   - Mathematical world coordinates (where x=2 means x equals 2)
 *   - Screen/pixel coordinates (where x=400 means 400 pixels from left)
 *
 * The transformation is:
 *   screenX = (worldX - panX) * zoom * scaleX + originScreenX
 *   screenY = (worldY - panY) * zoom * scaleY + originScreenY
 *
 * panX/panY: the world coordinate at the center of the screen
 * zoom: magnification factor
 * scaleX/scaleY: pixels per unit
 * originScreenX/Y: screen position of the origin
 */
struct CoordinateSystem {
    float panX;         // World x at screen center
    float panY;         // World y at screen center
    float zoom;         // Zoom level (1.0 = default)
    float scaleX;       // Pixels per world unit (x)
    float scaleY;       // Pixels per world unit (y)
    int screenWidth;    // Screen width in pixels
    int screenHeight;   // Screen height in pixels
    int plotAreaX;      // Plot area left edge
    int plotAreaY;      // Plot area top edge
    int plotAreaW;      // Plot area width
    int plotAreaH;      // Plot area height

    CoordinateSystem() :
        panX(0), panY(0), zoom(1.0f),
        scaleX(40.0f), scaleY(40.0f),
        screenWidth(1200), screenHeight(800),
        plotAreaX(0), plotAreaY(60),
        plotAreaW(850), plotAreaH(540)
    {}

    /* Convert world coordinate to screen coordinate */
    float worldToScreenX(float worldX) const {
        return plotAreaX + plotAreaW / 2.0f + (worldX - panX) * zoom * scaleX;
    }

    float worldToScreenY(float worldY) const {
        return plotAreaY + plotAreaH / 2.0f + (worldY - panY) * zoom * scaleY;
    }

    /* Convert screen coordinate to world coordinate */
    float screenToWorldX(float screenX) const {
        return (screenX - plotAreaX - plotAreaW / 2.0f) / (zoom * scaleX) + panX;
    }

    float screenToWorldY(float screenY) const {
        return (screenY - plotAreaY - plotAreaH / 2.0f) / (zoom * scaleY) + panY;
    }

    /* Get the visible world coordinate range */
    float getVisibleMinX() const { return screenToWorldX((float)plotAreaX); }
    float getVisibleMaxX() const { return screenToWorldX((float)(plotAreaX + plotAreaW)); }
    float getVisibleMinY() const { return screenToWorldY((float)(plotAreaY + plotAreaH)); }
    float getVisibleMaxY() const { return screenToWorldY((float)plotAreaY); }

    /* Reset to default view */
    void reset() {
        panX = 0;
        panY = 0;
        zoom = 1.0f;
    }

    /* Zoom in/out by a factor */
    void zoomIn()  { zoom *= 1.2f; if (zoom > 50.0f)  zoom = 50.0f; }
    void zoomOut() { zoom /= 1.2f; if (zoom < 0.05f) zoom = 0.05f; }

    /* Pan by a fraction of the visible area */
    void pan(float dx, float dy) {
        panX -= dx / (zoom * scaleX);
        panY -= dy / (zoom * scaleY);
    }
};

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

    /* Clear the graph */
    void clearGraph() {
        segments.clear();
        allRasterPoints.clear();
        samplePoints.clear();
        hasGraph = false;
    }
};

#endif // GLIDE_GRAPH_PLOTTER_H
