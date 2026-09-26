/*
 * DDACircle.h
 * -----------
 * DDA Circle Drawing Algorithm.
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
 * Start at θ = 0: x = r, y = 0
 * Continue until θ completes a full circle (2π).
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#ifndef GLIDE_DDA_CIRCLE_H
#define GLIDE_DDA_CIRCLE_H

#include "DrawingAlgorithm.h"
#include "../Common/MathUtils.h"
#include <cmath>
#include <sstream>

class DDACircle : public DrawingAlgorithm {
private:
    Point center;       // Center of the circle
    float radius;       // Radius r

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
               "y = r*sin(theta) and computes incremental changes "
               "dx = -y*dtheta, dy = x*dtheta with dtheta = 1/r "
               "to trace the circle one approximate pixel at a time.";
    }

    /*
     * Execute the DDA Circle algorithm.
     *
     * This directly implements the course notes:
     *   dθ = 1/r
     *   Starting at (r, 0)
     *   At each step: x_new = x - y*dθ, y_new = y + x*dθ
     *   Continue for 2πr steps (full circle)
     */
    void execute() override {
        clear();

        if (radius <= 0) return;

        /*
         * dθ = 1/r
         * This makes each step approximately 1 pixel along the arc,
         * since arc length = r * dθ = r * (1/r) = 1.
         */
        float dTheta = 1.0f / radius;

        /*
         * Number of steps to complete a full circle:
         * Full angle = 2π, step size = dθ = 1/r
         * Steps = 2π / dθ = 2π * r
         */
        int totalSteps = static_cast<int>(2.0f * MathUtils::PI * radius) + 1;

        /* Start at θ = 0: x = r, y = 0 */
        float x = radius;
        float y = 0.0f;
        float theta = 0.0f;

        for (int i = 0; i <= totalSteps; i++) {
            AlgorithmStep step;
            step.iteration = i + 1;
            step.x = x;
            step.y = y;
            step.radius = radius;
            step.theta = theta;
            step.dTheta = dTheta;

            /*
             * From the notes:
             *   dx = -y * dθ
             *   dy =  x * dθ
             */
            float dx = -y * dTheta;
            float dy =  x * dTheta;
            step.deltaX = dx;
            step.deltaY = dy;

            /* Plot the pixel at the rounded position, offset by center */
            Point pixel(
                (float)MathUtils::roundToInt(x + center.x),
                (float)MathUtils::roundToInt(y + center.y)
            );
            step.plottedPoints.push_back(pixel);

            /* Build explanation */
            std::ostringstream oss;
            if (i == 0) {
                oss << "DDA Circle: center = " << center.toString()
                    << ", radius = " << radius << "\n";
                oss << "dtheta = 1/r = 1/" << radius << " = " << dTheta << "\n";
                oss << "Start at theta=0: x = r = " << radius << ", y = 0\n";
                oss << "Total steps for full circle: " << totalSteps;
            } else {
                oss << "Step " << i << ": theta = "
                    << MathUtils::radToDeg(theta) << " deg\n";
                oss << "dx = -y * dtheta = " << -(y - dy) << " * " << dTheta
                    << " = " << dx << "\n";
                oss << "dy =  x * dtheta = " << (x - dx) << " * " << dTheta
                    << " = " << dy << "\n";
                oss << "x = " << (x - dx) << " + " << dx << " = " << x << "\n";
                oss << "y = " << (y - dy) << " + " << dy << " = " << y << "\n";
                oss << "Plot pixel " << pixel.toPixelString();
            }
            step.explanation = oss.str();
            step.decision = "Plot " + pixel.toPixelString();

            steps.push_back(step);
            allPoints.push_back(pixel);

            /*
             * Update position for next step:
             *   x_new = x + dx = x - y * dθ
             *   y_new = y + dy = y + x * dθ
             *
             * Important: use the CURRENT x to compute dy,
             * not the already-updated x_new. This is consistent
             * with the notes' derivation.
             */
            float newX = x + dx;
            float newY = y + dy;
            x = newX;
            y = newY;
            theta += dTheta;
        }
    }
};

#endif // GLIDE_DDA_CIRCLE_H
