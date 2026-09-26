/*
 * MathUtils.h
 * -----------
 * Simple math utility functions used throughout GLIDE.
 * Provides helper functions for common operations like sign,
 * clamping, and coordinate rounding.
 */

#ifndef GLIDE_MATH_UTILS_H
#define GLIDE_MATH_UTILS_H

#include <cmath>
#include <algorithm>

namespace MathUtils {

    /* Returns the sign of a number: -1, 0, or 1 */
    inline int sign(float value) {
        if (value > 0) return 1;
        if (value < 0) return -1;
        return 0;
    }

    /* Rounds a float to the nearest integer */
    inline int roundToInt(float value) {
        return static_cast<int>(roundf(value));
    }

    /* Clamp a value between min and max */
    inline float clamp(float value, float minVal, float maxVal) {
        return std::max(minVal, std::min(maxVal, value));
    }

    /* Check if a float value is valid (not NaN or infinity) */
    inline bool isValidFloat(float value) {
        return !std::isnan(value) && !std::isinf(value);
    }

    /* Convert degrees to radians */
    inline float degToRad(float degrees) {
        return degrees * 3.14159265358979f / 180.0f;
    }

    /* Convert radians to degrees */
    inline float radToDeg(float radians) {
        return radians * 180.0f / 3.14159265358979f;
    }

    /* Absolute value */
    inline float absf(float value) {
        return (value < 0) ? -value : value;
    }

    /* Linear interpolation */
    inline float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    const float PI = 3.14159265358979f;

} // namespace MathUtils

#endif // GLIDE_MATH_UTILS_H
