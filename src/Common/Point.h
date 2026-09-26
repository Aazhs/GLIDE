/*
 * Point.h
 * -------
 * Simple 2D point structure used throughout GLIDE.
 * Stores floating-point coordinates for mathematical precision
 * and integer coordinates for rasterized pixel positions.
 */

#ifndef GLIDE_POINT_H
#define GLIDE_POINT_H

#include <string>
#include <sstream>
#include <cmath>

/*
 * Point represents a 2D coordinate.
 * Used for both mathematical world coordinates (floating point)
 * and rasterized pixel coordinates (integer).
 */
struct Point {
    float x;
    float y;

    Point() : x(0.0f), y(0.0f) {}
    Point(float x, float y) : x(x), y(y) {}

    /* Get the integer-rounded pixel coordinate */
    int pixelX() const { return static_cast<int>(roundf(x)); }
    int pixelY() const { return static_cast<int>(roundf(y)); }

    /* String representation for display in the UI */
    std::string toString() const {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ")";
        return oss.str();
    }

    /* String with integer coordinates for pixel display */
    std::string toPixelString() const {
        std::ostringstream oss;
        oss << "(" << pixelX() << ", " << pixelY() << ")";
        return oss.str();
    }

    /* Distance between two points */
    float distanceTo(const Point& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        return sqrtf(dx * dx + dy * dy);
    }

    bool operator==(const Point& other) const {
        return (pixelX() == other.pixelX()) && (pixelY() == other.pixelY());
    }
};

#endif // GLIDE_POINT_H
