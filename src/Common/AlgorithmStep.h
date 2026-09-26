/*
 * AlgorithmStep.h
 * ---------------
 * Stores the data for a single step/iteration of a drawing algorithm.
 *
 * KEY DESIGN PRINCIPLE:
 * Each algorithm runs to completion first, generating a vector of these steps.
 * The animation/visualization system then reads through these steps one at a time.
 * This cleanly separates algorithm logic from rendering/animation timing.
 *
 * This structure holds all information needed to display what the algorithm
 * is doing at each iteration: current position, decision variables,
 * explanatory text, and the pixel(s) plotted.
 */

#ifndef GLIDE_ALGORITHM_STEP_H
#define GLIDE_ALGORITHM_STEP_H

#include "Point.h"
#include <string>
#include <vector>

struct AlgorithmStep {
    int iteration;              // Step number (1-indexed for display)

    float x;                    // Current x value
    float y;                    // Current y value

    float deltaX;               // Δx (meaning varies by algorithm)
    float deltaY;               // Δy (meaning varies by algorithm)

    float slope;                // Slope m (for line algorithms)
    float decisionVariable;     // Decision variable d/e (Bresenham algorithms)

    float dTheta;               // dθ (for DDA circle)
    float theta;                // Current θ (for DDA circle)
    float radius;               // Radius r (for circle algorithms)

    std::string explanation;    // Human-readable explanation of this step
    std::string decision;       // What decision was made (e.g., "e >= 0, increase y")

    /*
     * The pixel(s) plotted at this step.
     * For line algorithms: typically one point.
     * For Bresenham circle: up to 8 points due to symmetry.
     */
    std::vector<Point> plottedPoints;

    /*
     * For Bresenham circle: the 8 symmetric points generated from
     * the single calculated point. Stored separately so the visualizer
     * can highlight them individually.
     */
    std::vector<Point> symmetricPoints;

    AlgorithmStep() :
        iteration(0),
        x(0), y(0),
        deltaX(0), deltaY(0),
        slope(0), decisionVariable(0),
        dTheta(0), theta(0), radius(0)
    {}
};

#endif // GLIDE_ALGORITHM_STEP_H
