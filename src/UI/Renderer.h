/*
 * Renderer.h
 * ----------
 * All low-level OpenGL drawing lives here: rectangles, circles, lines,
 * text and the coordinate grid. Algorithm classes never call OpenGL;
 * screens describe WHAT to draw and the Renderer knows HOW.
 *
 * Coordinates are screen pixels with (0,0) at the top-left corner.
 */

#ifndef GLIDE_RENDERER_H
#define GLIDE_RENDERER_H

#include "Theme.h"
#include "../Graph/CoordinateSystem.h"
#include "../Common/Format.h"
#include "../Common/MathUtils.h"

#include <string>
#include <vector>
#include <sstream>
#include <cmath>

/* Simple axis-aligned rectangle in screen pixels */
struct Rect {
    float x, y, w, h;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), w(w), h(h) {}

    float right()  const { return x + w; }
    float bottom() const { return y + h; }
    float cx()     const { return x + w / 2.0f; }
    float cy()     const { return y + h / 2.0f; }

    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    Rect inset(float d) const { return Rect(x + d, y + d, w - 2 * d, h - 2 * d); }
};

/* Fonts (GLUT bitmap fonts - ASCII only) */
namespace Fonts {
    inline void* small() { return GLUT_BITMAP_HELVETICA_10; }
    inline void* body()  { return GLUT_BITMAP_HELVETICA_12; }
    inline void* title() { return GLUT_BITMAP_HELVETICA_18; }
    inline void* mono()  { return GLUT_BITMAP_8_BY_13; }      // fixed width: 8 px
}

class Renderer {
public:
    /* ---------------------------------------------------------------
     * TEXT
     * --------------------------------------------------------------- */

    static int lineHeight(void* font) {
        if (font == Fonts::title()) return 24;
        if (font == Fonts::small()) return 13;
        if (font == Fonts::mono())  return 15;
        return 16;
    }

    /* Width in pixels of a single line of text */
    static int textWidth(const std::string& text, void* font = Fonts::body()) {
        int w = 0, best = 0;
        for (char c : text) {
            if (c == '\n') { best = std::max(best, w); w = 0; continue; }
            w += glutBitmapWidth(font, c);
        }
        return std::max(best, w);
    }

    /* Draw text; (x, y) is the BASELINE of the first line. '\n' starts a new line. */
    static void drawText(float x, float y, const std::string& text, void* font = Fonts::body()) {
        glRasterPos2f(x, y);
        for (char c : text) {
            if (c == '\n') {
                y += lineHeight(font);
                glRasterPos2f(x, y);
            } else {
                glutBitmapCharacter(font, c);
            }
        }
    }

    /* Draw a single line of text vertically centred in a box */
    static void drawTextInBox(const Rect& r, const std::string& text, void* font,
                              bool centreX = true) {
        int tw = textWidth(text, font);
        float tx = centreX ? r.x + (r.w - tw) / 2.0f : r.x;
        float ty = r.y + r.h / 2.0f + (font == Fonts::title() ? 6.0f : 4.0f);
        drawText(std::floor(tx), std::floor(ty), text, font);
    }

    /* Split text into lines that fit 'maxWidth', honouring existing '\n' */
    static std::vector<std::string> wrapText(const std::string& text, void* font, float maxWidth) {
        std::vector<std::string> out;
        std::istringstream paragraphs(text);
        std::string para;
        while (std::getline(paragraphs, para)) {
            /* Preserve leading indentation of the paragraph */
            size_t indentLen = para.find_first_not_of(' ');
            if (indentLen == std::string::npos) { out.push_back(""); continue; }
            std::string indent = para.substr(0, indentLen);

            std::istringstream words(para.substr(indentLen));
            std::string word, line = indent;
            bool lineHasWord = false;
            while (words >> word) {
                std::string candidate = lineHasWord ? line + " " + word : line + word;
                if (lineHasWord && textWidth(candidate, font) > maxWidth) {
                    out.push_back(line);
                    line = indent + "  " + word;   // continuation lines indent a little
                } else {
                    line = candidate;
                }
                lineHasWord = true;
            }
            out.push_back(line);
        }
        return out;
    }

    /* Draw wrapped text; returns the y after the last line */
    static float drawWrapped(float x, float y, float maxWidth, const std::string& text,
                             void* font, float maxY = 1e9f) {
        for (const auto& line : wrapText(text, font, maxWidth)) {
            if (y > maxY) break;
            drawText(x, y, line, font);
            y += lineHeight(font);
        }
        return y;
    }

    /* ---------------------------------------------------------------
     * SHAPES
     * --------------------------------------------------------------- */

    static void drawRect(float x, float y, float w, float h) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    static void drawRect(const Rect& r) { drawRect(r.x, r.y, r.w, r.h); }

    static void drawRectOutline(float x, float y, float w, float h, float lineW = 1.0f) {
        glLineWidth(lineW);
        /* Offset by half a pixel so 1px outlines land exactly on pixels */
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 0.5f, y + 0.5f);
        glVertex2f(x + w - 0.5f, y + 0.5f);
        glVertex2f(x + w - 0.5f, y + h - 0.5f);
        glVertex2f(x + 0.5f, y + h - 0.5f);
        glEnd();
        glLineWidth(1.0f);
    }
    static void drawRectOutline(const Rect& r, float lineW = 1.0f) {
        drawRectOutline(r.x, r.y, r.w, r.h, lineW);
    }

    static void drawLine(float x1, float y1, float x2, float y2, float lineW = 1.0f) {
        glLineWidth(lineW);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
        glLineWidth(1.0f);
    }

    static void drawDashedLine(float x1, float y1, float x2, float y2, float lineW = 1.0f) {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0x00FF);
        drawLine(x1, y1, x2, y2, lineW);
        glDisable(GL_LINE_STIPPLE);
    }

    static void drawCircle(float cx, float cy, float radius, bool filled, int segments = 48) {
        glBegin(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
        if (filled) glVertex2f(cx, cy);
        for (int i = 0; i <= segments; i++) {
            float a = 2.0f * MathUtils::PI * i / segments;
            glVertex2f(cx + radius * cosf(a), cy + radius * sinf(a));
        }
        glEnd();
    }

    /* A round draggable handle: filled dot with a light ring */
    static void drawHandle(float cx, float cy, const Color& c, bool hot) {
        float r = hot ? 8.0f : 6.0f;
        setColor(Theme::PLOT_BG);
        drawCircle(cx, cy, r + 2.5f, true);
        setColor(c);
        drawCircle(cx, cy, r, true);
        setColor(hot ? Theme::TEXT : c.mix(Theme::TEXT, 0.5f));
        glLineWidth(1.5f);
        drawCircle(cx, cy, r + 2.0f, false);
        glLineWidth(1.0f);
    }

    /* Text with a dark backing box so it stays readable over pixels */
    static void drawLabel(float x, float y, const std::string& text, const Color& c,
                          void* font = Fonts::small()) {
        int tw = textWidth(text, font);
        int lh = lineHeight(font);
        setColor(Theme::PLOT_BG.withAlpha(0.85f));
        drawRect(x - 3, y - lh + 2, (float)tw + 6, (float)lh + 2);
        setColor(c);
        drawText(x, y, text, font);
    }

    /* ---------------------------------------------------------------
     * PIXEL CELLS
     * A raster pixel (i, j) is a unit square CENTRED on the integer
     * coordinate (i, j), i.e. it covers [i-0.5, i+0.5] x [j-0.5, j+0.5].
     * --------------------------------------------------------------- */

    static void drawPixelCell(const CoordinateSystem& cs, float wx, float wy, float inset = 1.0f) {
        float ppu = cs.pixelsPerUnitX();
        float size = std::max(ppu - 2.0f * inset, 3.0f);
        float sx = cs.worldToScreenX(wx);
        float sy = cs.worldToScreenY(wy);
        drawRect(sx - size / 2.0f, sy - size / 2.0f, size, size);
    }

    static void drawPixelCellOutline(const CoordinateSystem& cs, float wx, float wy,
                                     float lineW = 2.0f) {
        float ppu = cs.pixelsPerUnitX();
        float size = std::max(ppu, 5.0f);
        float sx = cs.worldToScreenX(wx);
        float sy = cs.worldToScreenY(wy);
        drawRectOutline(sx - size / 2.0f, sy - size / 2.0f, size, size, lineW);
    }

    /* ---------------------------------------------------------------
     * CLIPPING
     * --------------------------------------------------------------- */

    static void beginClip(const Rect& r, int windowHeight) {
        glEnable(GL_SCISSOR_TEST);
        glScissor((int)r.x, windowHeight - (int)(r.y + r.h), (int)r.w, (int)r.h);
    }
    static void endClip() { glDisable(GL_SCISSOR_TEST); }

    /* ---------------------------------------------------------------
     * GRID
     * --------------------------------------------------------------- */

    /* Choose a "nice" spacing (1, 2 or 5 x 10^n) at least minPixels apart */
    static float niceStep(float pixelsPerUnit, float minPixels) {
        float raw = minPixels / std::max(pixelsPerUnit, 1e-6f);
        float mag = powf(10.0f, floorf(log10f(raw)));
        float n = raw / mag;
        float step = (n <= 1.0f) ? 1.0f : (n <= 2.0f) ? 2.0f : (n <= 5.0f) ? 5.0f : 10.0f;
        return step * mag;
    }

    /*
     * Draw the coordinate grid, axes and tick labels for 'cs'.
     *
     * pixelCells = true  -> raster mode: thin lines on pixel BOUNDARIES
     *                       (n +- 0.5) so each cell is one pixel.
     * pixelCells = false -> graph mode: lines at nice round values.
     */
    static void drawGrid(const CoordinateSystem& cs, bool pixelCells) {
        float minX = cs.getVisibleMinX(), maxX = cs.getVisibleMaxX();
        float minY = cs.getVisibleMinY(), maxY = cs.getVisibleMaxY();
        float top = (float)cs.plotAreaY, bottom = (float)(cs.plotAreaY + cs.plotAreaH);
        float left = (float)cs.plotAreaX, right = (float)(cs.plotAreaX + cs.plotAreaW);
        float ppu = cs.pixelsPerUnitX();

        float major = niceStep(ppu, 55.0f);
        if (pixelCells && major < 1.0f) major = 1.0f;

        bool showCells = pixelCells && ppu >= 8.0f;

        glBegin(GL_LINES);
        if (showCells) {
            /* Pixel boundaries: x = i + 0.5 */
            setColor(Theme::PIXEL_GRID);
            for (int i = (int)floorf(minX - 0.5f); i <= (int)ceilf(maxX); i++) {
                float sx = floorf(cs.worldToScreenX(i + 0.5f)) + 0.5f;
                glVertex2f(sx, top); glVertex2f(sx, bottom);
            }
            for (int j = (int)floorf(minY - 0.5f); j <= (int)ceilf(maxY); j++) {
                float sy = floorf(cs.worldToScreenY(j + 0.5f)) + 0.5f;
                glVertex2f(left, sy); glVertex2f(right, sy);
            }
        } else {
            /* Minor lines (a fifth of the major spacing) */
            float minor = major / 5.0f;
            if (minor * ppu >= 9.0f && !pixelCells) {
                setColor(Theme::GRID_MINOR);
                for (long i = (long)ceilf(minX / minor); i <= (long)floorf(maxX / minor); i++) {
                    float sx = floorf(cs.worldToScreenX(i * minor)) + 0.5f;
                    glVertex2f(sx, top); glVertex2f(sx, bottom);
                }
                for (long j = (long)ceilf(minY / minor); j <= (long)floorf(maxY / minor); j++) {
                    float sy = floorf(cs.worldToScreenY(j * minor)) + 0.5f;
                    glVertex2f(left, sy); glVertex2f(right, sy);
                }
            }
            /* Major lines */
            setColor(Theme::GRID_MAJOR);
            for (long i = (long)ceilf(minX / major); i <= (long)floorf(maxX / major); i++) {
                float sx = floorf(cs.worldToScreenX(i * major)) + 0.5f;
                glVertex2f(sx, top); glVertex2f(sx, bottom);
            }
            for (long j = (long)ceilf(minY / major); j <= (long)floorf(maxY / major); j++) {
                float sy = floorf(cs.worldToScreenY(j * major)) + 0.5f;
                glVertex2f(left, sy); glVertex2f(right, sy);
            }
        }
        glEnd();

        /* Axes */
        float axisX = cs.worldToScreenX(0);   // screen x of the y-axis
        float axisY = cs.worldToScreenY(0);   // screen y of the x-axis
        setColor(Theme::AXIS);
        if (axisY >= top && axisY <= bottom) drawLine(left, axisY, right, axisY, 1.5f);
        if (axisX >= left && axisX <= right) drawLine(axisX, top, axisX, bottom, 1.5f);

        /* Tick labels: stick to the axis, or to the plot edge if the axis is off-screen */
        int decimals = major >= 1.0f ? 0 : (major >= 0.1f ? 1 : 2);
        float labelY = std::max(top + 14.0f, std::min(axisY + 14.0f, bottom - 5.0f));
        float labelXBase = std::max(left + 4.0f, std::min(axisX + 4.0f, right - 30.0f));

        setColor(Theme::TEXT_DIM);
        for (long i = (long)ceilf(minX / major); i <= (long)floorf(maxX / major); i++) {
            if (i == 0) continue;
            float v = i * major;
            float sx = cs.worldToScreenX(v);
            std::string s = Format::num(v, decimals);
            int tw = textWidth(s, Fonts::small());
            if (sx - tw / 2.0f < left + 2 || sx + tw / 2.0f > right - 2) continue;
            drawText(sx - tw / 2.0f, labelY, s, Fonts::small());
        }
        for (long j = (long)ceilf(minY / major); j <= (long)floorf(maxY / major); j++) {
            if (j == 0) continue;
            float v = j * major;
            float sy = cs.worldToScreenY(v);
            if (sy < top + 12 || sy > bottom - 4) continue;
            drawText(labelXBase, sy + 4, Format::num(v, decimals), Fonts::small());
        }
        if (axisX >= left && axisX <= right && axisY >= top && axisY <= bottom) {
            drawText(axisX + 4, axisY + 14, "0", Fonts::small());
        }
    }
};

#endif // GLIDE_RENDERER_H
