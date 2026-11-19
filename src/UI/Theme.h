/*
 * Theme.h
 * -------
 * The colour palette used throughout GLIDE (dark theme, so that plotted
 * pixels stand out).
 *
 * Every colour has a single job:
 *   ACCENT   (cyan)    - DDA, primary actions, the "active" state
 *   ACCENT2  (orange)  - Bresenham, secondary information
 *   PURPLE             - Midpoint circle
 *   YELLOW             - the CURRENT step being explained
 *   GREEN / RED        - start / end point, correct / wrong
 *   MIRROR   (pink)    - points produced by 8-way symmetry
 */

#ifndef GLIDE_THEME_H
#define GLIDE_THEME_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

struct Color {
    float r, g, b, a;
    Color(float r = 0, float g = 0, float b = 0, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

    Color withAlpha(float alpha) const { return Color(r, g, b, alpha); }

    /* Blend towards another colour (t = 0 -> this, t = 1 -> other) */
    Color mix(const Color& o, float t) const {
        return Color(r + (o.r - r) * t, g + (o.g - g) * t, b + (o.b - b) * t, a + (o.a - a) * t);
    }
};

inline void setColor(const Color& c) { glColor4f(c.r, c.g, c.b, c.a); }

namespace Theme {
    /* Surfaces */
    const Color BG          (0.090f, 0.094f, 0.110f);
    const Color PLOT_BG     (0.075f, 0.078f, 0.092f);
    const Color HEADER      (0.060f, 0.063f, 0.075f);
    const Color PANEL       (0.125f, 0.130f, 0.152f);
    const Color PANEL_ALT   (0.155f, 0.160f, 0.188f);
    const Color BORDER      (0.230f, 0.240f, 0.280f);

    /* Grid */
    const Color GRID_MINOR  (0.150f, 0.155f, 0.180f);
    const Color GRID_MAJOR  (0.210f, 0.218f, 0.255f);
    const Color PIXEL_GRID  (0.185f, 0.192f, 0.225f);
    const Color AXIS        (0.480f, 0.490f, 0.540f);

    /* Text */
    const Color TEXT        (0.920f, 0.925f, 0.940f);
    const Color TEXT_DIM    (0.580f, 0.595f, 0.640f);
    const Color TEXT_FAINT  (0.400f, 0.410f, 0.450f);

    /* Accents */
    const Color ACCENT      (0.000f, 0.800f, 0.920f);
    const Color ACCENT2     (1.000f, 0.600f, 0.220f);
    const Color PURPLE      (0.700f, 0.520f, 1.000f);
    const Color GREEN       (0.330f, 0.880f, 0.450f);
    const Color YELLOW      (1.000f, 0.880f, 0.300f);
    const Color RED         (1.000f, 0.360f, 0.360f);
    const Color MIRROR      (0.900f, 0.420f, 0.640f);

    /* Buttons */
    const Color BTN         (0.185f, 0.192f, 0.228f);
    const Color BTN_HOVER   (0.250f, 0.260f, 0.310f);
    const Color BTN_ACTIVE  (0.000f, 0.420f, 0.500f);
    const Color BTN_BORDER  (0.300f, 0.312f, 0.365f);
}

#endif // GLIDE_THEME_H
