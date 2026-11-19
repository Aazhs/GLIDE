/*
 * DrawingAlgorithm.h
 * ------------------
 * Base class for all drawing algorithms in GLIDE.
 *
 * Every algorithm follows the same pattern:
 *   1. Set parameters (endpoints for lines, center+radius for circles)
 *   2. Call execute() to run the algorithm
 *   3. Call getSteps() to retrieve the computed steps
 *   4. The renderer/animator reads through steps one at a time
 *
 * This base class ensures all algorithms share a common interface,
 * making it easy to switch between them in the graph plotter.
 */

#ifndef GLIDE_DRAWING_ALGORITHM_H
#define GLIDE_DRAWING_ALGORITHM_H

#include "../Common/AlgorithmStep.h"
#include "../Common/Point.h"
#include "../Common/OpCounts.h"
#include <vector>
#include <string>
#include <set>
#include <cmath>
#include <algorithm>

/*
 * How good is the rasterised result? Filled in by each algorithm after
 * execute(), so the explorers can compare accuracy, not just speed.
 */
struct Quality {
    float maxError;     // largest distance of a pixel centre from the ideal shape
    float avgError;     // average of that distance
    int uniquePixels;   // distinct pixels produced
    int duplicates;     // pixels plotted more than once
    int gaps;           // breaks in the pixel chain (neighbours more than 1 apart)
    float drift;        // DDA circle only: how far the radius drifted by the end
    bool hasDrift;

    Quality() : maxError(0), avgError(0), uniquePixels(0), duplicates(0),
                gaps(0), drift(0), hasDrift(false) {}
};

/*
 * Write the 8 symmetric points of octant point (x, y) around (cx, cy)
 * as 16 ints. Used by the lean circle rasterizers that are benchmarked.
 */
inline void writeMirrors8(int cx, int cy, int x, int y, int* o) {
    o[0]  = cx - x; o[1]  = cy - y;   o[2]  = cx + y; o[3]  = cy - x;
    o[4]  = cx - y; o[5]  = cy + x;   o[6]  = cx + x; o[7]  = cy + y;
    o[8]  = cx - x; o[9]  = cy + y;   o[10] = cx - y; o[11] = cy - x;
    o[12] = cx + y; o[13] = cy + x;   o[14] = cx + x; o[15] = cy - y;
}

class DrawingAlgorithm {
protected:
    std::vector<AlgorithmStep> steps;       // All computed steps
    std::vector<Point> allPoints;           // All rasterized points (final result)
    std::string algorithmName;              // Name for display
    std::vector<std::string> setupLines;    // Initial values (dx, dy, e0 ...) for the panel
    OpCounts setupOps;                      // Arithmetic done before the main loop
    Quality quality;                        // Accuracy of the result

    /* Count distinct pixels / duplicates in allPoints */
    void countPixels() {
        std::set<std::pair<int, int>> seen;
        for (const auto& p : allPoints) seen.insert({ p.pixelX(), p.pixelY() });
        quality.uniquePixels = (int)seen.size();
        quality.duplicates = (int)allPoints.size() - quality.uniquePixels;
    }

    /* Accuracy against the ideal line through a and b */
    void measureLineQuality(const Point& a, const Point& b) {
        countPixels();
        float dx = b.x - a.x, dy = b.y - a.y;
        float len = sqrtf(dx * dx + dy * dy);
        float sum = 0, mx = 0;
        for (const auto& p : allPoints) {
            float d = len < 1e-6f ? p.distanceTo(a) : fabsf(dy * (p.x - a.x) - dx * (p.y - a.y)) / len;
            sum += d;
            mx = std::max(mx, d);
        }
        quality.maxError = mx;
        quality.avgError = allPoints.empty() ? 0 : sum / allPoints.size();
        quality.gaps = 0;
        for (size_t i = 1; i < allPoints.size(); i++) {
            if (std::max(fabsf(allPoints[i].x - allPoints[i - 1].x),
                         fabsf(allPoints[i].y - allPoints[i - 1].y)) > 1.5f) quality.gaps++;
        }
    }

    /* Accuracy against the ideal circle; octant points are step.x / step.y */
    void measureCircleQuality(const Point& c, float r) {
        countPixels();
        float sum = 0, mx = 0;
        for (const auto& p : allPoints) {
            float d = fabsf(p.distanceTo(c) - r);
            sum += d;
            mx = std::max(mx, d);
        }
        quality.maxError = mx;
        quality.avgError = allPoints.empty() ? 0 : sum / allPoints.size();
        quality.gaps = 0;
        for (size_t i = 1; i < steps.size(); i++) {
            if (std::max(fabsf(steps[i].x - steps[i - 1].x),
                         fabsf(steps[i].y - steps[i - 1].y)) > 1.5f) quality.gaps++;
        }
    }

public:
    DrawingAlgorithm() {}
    virtual ~DrawingAlgorithm() {}

    /* Execute the algorithm and populate the steps vector */
    virtual void execute() = 0;

    /* Get the name of this algorithm */
    virtual std::string getName() const { return algorithmName; }

    /* Get a short description of the algorithm */
    virtual std::string getDescription() const = 0;

    /* Get all computed steps for animation/visualization */
    const std::vector<AlgorithmStep>& getSteps() const { return steps; }

    /* Get all rasterized points (the final drawn result) */
    const std::vector<Point>& getAllPoints() const { return allPoints; }

    /* Initial calculations performed before the main loop */
    const std::vector<std::string>& getSetupLines() const { return setupLines; }

    /* Column headings for the iteration table (matches AlgorithmStep::tableCells) */
    virtual std::vector<std::string> getTableColumns() const = 0;

    /* Operation counts */
    const OpCounts& getSetupOps() const { return setupOps; }
    OpCounts getTotalOps() const {
        OpCounts t = setupOps;
        for (const auto& s : steps) t += s.ops;
        return t;
    }
    const Quality& getQuality() const { return quality; }

    /*
     * Measured time for one complete run of the lean rasterize() version
     * of this algorithm on the current parameters (nanoseconds).
     */
    virtual double benchmarkNs() const = 0;

    /* Get the number of steps */
    int getStepCount() const { return static_cast<int>(steps.size()); }

    /* Clear all computed data */
    virtual void clear() {
        steps.clear();
        allPoints.clear();
        setupLines.clear();
        setupOps = OpCounts();
        quality = Quality();
    }
};

#endif // GLIDE_DRAWING_ALGORITHM_H
