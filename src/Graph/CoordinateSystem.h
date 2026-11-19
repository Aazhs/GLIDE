/*
 * CoordinateSystem.h
 * ------------------
 * Converts between mathematical WORLD coordinates and SCREEN pixels.
 *
 *   World space : x grows to the right, y grows UPWARDS (normal maths).
 *   Screen space: x grows to the right, y grows DOWNWARDS (OpenGL ortho
 *                 projection with (0,0) at the top-left of the window).
 *
 * Because the two y-axes point in opposite directions, the y conversion
 * has a minus sign:
 *
 *   screenX = centreX + (worldX - panX) * zoom * scaleX
 *   screenY = centreY - (worldY - panY) * zoom * scaleY
 *
 * where (centreX, centreY) is the middle of the plot area and
 * (panX, panY) is the world point shown at that centre.
 */

#ifndef GLIDE_COORDINATE_SYSTEM_H
#define GLIDE_COORDINATE_SYSTEM_H

#include <algorithm>
#include <cmath>

struct CoordinateSystem {
    float panX;         // World x shown at the centre of the plot area
    float panY;         // World y shown at the centre of the plot area
    float zoom;         // Magnification factor (1.0 = default)
    float scaleX;       // Pixels per world unit at zoom 1 (x)
    float scaleY;       // Pixels per world unit at zoom 1 (y)
    int screenWidth;    // Window width in pixels
    int screenHeight;   // Window height in pixels
    int plotAreaX;      // Plot area left edge
    int plotAreaY;      // Plot area top edge
    int plotAreaW;      // Plot area width
    int plotAreaH;      // Plot area height

    static constexpr float MIN_ZOOM = 0.05f;
    static constexpr float MAX_ZOOM = 50.0f;

    CoordinateSystem() :
        panX(0), panY(0), zoom(1.0f),
        scaleX(40.0f), scaleY(40.0f),
        screenWidth(1200), screenHeight(800),
        plotAreaX(0), plotAreaY(100),
        plotAreaW(850), plotAreaH(540)
    {}

    /* Pixels covered by one world unit at the current zoom */
    float pixelsPerUnitX() const { return zoom * scaleX; }
    float pixelsPerUnitY() const { return zoom * scaleY; }

    float centreScreenX() const { return plotAreaX + plotAreaW / 2.0f; }
    float centreScreenY() const { return plotAreaY + plotAreaH / 2.0f; }

    /* World -> screen */
    float worldToScreenX(float worldX) const {
        return centreScreenX() + (worldX - panX) * pixelsPerUnitX();
    }
    float worldToScreenY(float worldY) const {
        return centreScreenY() - (worldY - panY) * pixelsPerUnitY();
    }

    /* Screen -> world (exact inverse of the above) */
    float screenToWorldX(float screenX) const {
        return (screenX - centreScreenX()) / pixelsPerUnitX() + panX;
    }
    float screenToWorldY(float screenY) const {
        return panY - (screenY - centreScreenY()) / pixelsPerUnitY();
    }

    /* Visible world range (min is always <= max) */
    float getVisibleMinX() const { return screenToWorldX((float)plotAreaX); }
    float getVisibleMaxX() const { return screenToWorldX((float)(plotAreaX + plotAreaW)); }
    float getVisibleMinY() const { return screenToWorldY((float)(plotAreaY + plotAreaH)); }
    float getVisibleMaxY() const { return screenToWorldY((float)plotAreaY); }

    bool containsScreen(int sx, int sy) const {
        return sx >= plotAreaX && sx < plotAreaX + plotAreaW &&
               sy >= plotAreaY && sy < plotAreaY + plotAreaH;
    }

    void setPlotArea(int x, int y, int w, int h) {
        plotAreaX = x; plotAreaY = y;
        plotAreaW = std::max(w, 1); plotAreaH = std::max(h, 1);
    }

    /* Reset to default view */
    void reset() {
        panX = 0;
        panY = 0;
        zoom = 1.0f;
    }

    /* Zoom about the centre of the plot area */
    void zoomIn()  { zoomAt(1.2f, centreScreenX(), centreScreenY()); }
    void zoomOut() { zoomAt(1.0f / 1.2f, centreScreenX(), centreScreenY()); }

    /*
     * Zoom by 'factor' while keeping the world point under the screen
     * position (sx, sy) fixed - this is how zooming at the mouse cursor works.
     */
    void zoomAt(float factor, float sx, float sy) {
        float wx = screenToWorldX(sx);
        float wy = screenToWorldY(sy);
        zoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, zoom * factor));
        /* Re-solve pan so that (wx, wy) maps back onto (sx, sy) */
        panX = wx - (sx - centreScreenX()) / pixelsPerUnitX();
        panY = wy + (sy - centreScreenY()) / pixelsPerUnitY();
    }

    /*
     * Move the drawn content by (dx, dy) SCREEN pixels.
     * Dragging the mouse right by 10px calls panPixels(10, 0) and the
     * picture follows the mouse.
     */
    void panPixels(float dx, float dy) {
        panX -= dx / pixelsPerUnitX();
        panY += dy / pixelsPerUnitY();
    }

    /* Kept for compatibility with older code: same as panPixels */
    void pan(float dx, float dy) { panPixels(dx, dy); }

    /*
     * Adjust pan and zoom so that the world rectangle
     * [minX, maxX] x [minY, maxY] is fully visible, with 'marginPx'
     * pixels of breathing room on each side.
     */
    void fitTo(float minX, float minY, float maxX, float maxY, float marginPx = 40.0f) {
        float w = std::max(maxX - minX, 1.0f);
        float h = std::max(maxY - minY, 1.0f);
        float availW = std::max(plotAreaW - 2.0f * marginPx, 40.0f);
        float availH = std::max(plotAreaH - 2.0f * marginPx, 40.0f);
        float zx = availW / (w * scaleX);
        float zy = availH / (h * scaleY);
        zoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, std::min(zx, zy)));
        panX = (minX + maxX) / 2.0f;
        panY = (minY + maxY) / 2.0f;
    }

    /* True if the world rectangle is completely inside the plot area */
    bool showsWorldRect(float minX, float minY, float maxX, float maxY,
                        float marginPx = 10.0f) const {
        return worldToScreenX(minX) >= plotAreaX + marginPx &&
               worldToScreenX(maxX) <= plotAreaX + plotAreaW - marginPx &&
               worldToScreenY(maxY) >= plotAreaY + marginPx &&
               worldToScreenY(minY) <= plotAreaY + plotAreaH - marginPx;
    }
};

#endif // GLIDE_COORDINATE_SYSTEM_H
