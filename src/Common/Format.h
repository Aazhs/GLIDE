/*
 * Format.h
 * --------
 * Small helpers for turning numbers and points into short, readable text
 * for the explanation panels and step tables.
 *
 * NOTE: GLUT bitmap fonts only contain ASCII characters, so all text
 * shown on screen must avoid symbols like the Greek delta or theta.
 */

#ifndef GLIDE_FORMAT_H
#define GLIDE_FORMAT_H

#include "Point.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

namespace Format {

    /* Format a float with up to 'maxDecimals' decimals, trimming zeros:
     * 2.500 -> "2.5", 3.000 -> "3", -0.000 -> "0" */
    inline std::string num(float v, int maxDecimals = 3) {
        if (std::isnan(v)) return "nan";
        if (std::isinf(v)) return v > 0 ? "inf" : "-inf";
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(maxDecimals) << v;
        std::string s = oss.str();
        if (s.find('.') != std::string::npos) {
            while (!s.empty() && s.back() == '0') s.pop_back();
            if (!s.empty() && s.back() == '.') s.pop_back();
        }
        if (s == "-0") s = "0";
        return s;
    }

    /* Integer pair as "(x, y)" */
    inline std::string pt(int x, int y) {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    /* Float pair as "(x, y)" */
    inline std::string ptf(float x, float y, int maxDecimals = 2) {
        return "(" + num(x, maxDecimals) + ", " + num(y, maxDecimals) + ")";
    }

    /* Point rounded to its pixel */
    inline std::string px(const Point& p) {
        return pt(p.pixelX(), p.pixelY());
    }

    /* Signed step such as "+1" / "-1" */
    inline std::string sgn(int s) {
        return s >= 0 ? "+" + std::to_string(s) : std::to_string(s);
    }

    /* Lay out a list of points 'perLine' at a time, one line per row */
    inline std::string pointRows(const std::vector<Point>& pts, int perLine) {
        std::string out;
        for (size_t i = 0; i < pts.size(); i++) {
            if (i > 0) out += (i % perLine == 0) ? "\n" : " ";
            out += px(pts[i]);
        }
        return out;
    }

} // namespace Format

#endif // GLIDE_FORMAT_H
