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
#include <vector>
#include <string>

class DrawingAlgorithm {
protected:
    std::vector<AlgorithmStep> steps;       // All computed steps
    std::vector<Point> allPoints;           // All rasterized points (final result)
    std::string algorithmName;              // Name for display

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

    /* Get the number of steps */
    int getStepCount() const { return static_cast<int>(steps.size()); }

    /* Clear all computed data */
    virtual void clear() {
        steps.clear();
        allPoints.clear();
    }
};

#endif // GLIDE_DRAWING_ALGORITHM_H
