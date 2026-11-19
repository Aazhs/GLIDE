/*
 * HelpScreen.h
 * ------------
 * Controls reference and colour legend.
 */

#ifndef GLIDE_HELP_SCREEN_H
#define GLIDE_HELP_SCREEN_H

#include "Screen.h"

class HelpScreen : public Screen {
private:
    static float drawList(float x, float y, const char* heading,
                          const std::vector<std::pair<const char*, const char*>>& items) {
        setColor(Theme::ACCENT2);
        Renderer::drawText(x, y, heading, Fonts::body());
        y += 22;
        for (const auto& it : items) {
            setColor(Theme::YELLOW);
            Renderer::drawText(x, y, it.first, Fonts::mono());
            setColor(Theme::TEXT);
            Renderer::drawText(x + 130, y, it.second, Fonts::body());
            y += 19;
        }
        return y + 16;
    }

public:
    explicit HelpScreen(Settings& s) : Screen(s) {}

    void render(UI& ui) override {
        drawHeader(ui, "Help & Controls", "everything you can do in GLIDE");
        float w = (float)ui.width();
        float colW = std::min(520.0f, (w - 60) / 2);
        float x1 = std::max(24.0f, (w - colW * 2 - 30) / 2), x2 = x1 + colW + 30;
        float y = 90;

        float yl = drawList(x1, y, "Explorers (lines and circles)", {
            { "Click",        "Lines: start a new line / Circles: move centre" },
            { "Drag handle",  "Move P1 / P2, centre or radius (live)" },
            { "Drag empty",   "Pan the view (right-drag also pans)" },
            { "Space",        "Play / pause the animation" },
            { "N  or  .",     "Next step" },
            { "P  or  ,",     "Previous step" },
            { "S / E",        "Jump to first / last step" },
            { "Table row",    "Click a row to jump to that step" },
            { "1 / 2 / 3",    "Choose algorithm / compare mode" },
            { "Q",            "Quiz: predict the next pixel" },
            { "G / I",        "Toggle pixel grid / ideal shape" },
            { "+ / -",        "Zoom in / out" },
            { "R or F",       "Fit the shape in view" },
            { "Arrow keys",   "Pan the view" },
            { "[ / ]",        "Circle radius - / +" },
            { "Esc",          "Cancel / close popup / back to menu" },
        });

        float yr = drawList(x2, y, "Graph plotter", {
            { "Click y = box", "Type a function, Enter to plot" },
            { "Examples",      "One-click sample functions" },
            { "D / B",         "Use DDA / Bresenham" },
            { "T",             "Step through the rasterisation" },
            { "[ / ]",         "Fewer / more samples" },
            { "Drag",          "Pan       Hover: trace the curve" },
            { "+ / -   R",     "Zoom, reset view" },
        });

        yr = drawList(x2, yr, "Polygon Explorer", {
            { "1 - 5", "Choose a fill method" },
            { "C / D", "Load a concave polygon or diamond" },
            { "Click", "Set the starting cell for seed fill" },
            { "Space / N / P", "Play or step through the fill" },
        });

        yr = drawList(x2, yr, "Functions you can type", {
            { "+ - * / ^",     "operators ( 2x and 3sin(x) also work )" },
            { "sin cos tan",   "trigonometry (radians)" },
            { "sqrt abs",      "square root, absolute value" },
            { "pi",            "the constant 3.14159..." },
        });

        /* Colour legend */
        setColor(Theme::ACCENT2);
        Renderer::drawText(x2, yr, "Colours", Fonts::body());
        yr += 22;
        struct Swatch { Color c; const char* text; };
        const Swatch sw[] = {
            { Theme::ACCENT,  "DDA pixels" },
            { Theme::ACCENT2, "Bresenham pixels" },
            { Theme::PURPLE,  "Midpoint circle pixels" },
            { Theme::YELLOW,  "the current step" },
            { Theme::MIRROR,  "points copied by 8-way symmetry" },
            { Theme::RED,     "pixels where algorithms disagree" },
        };
        for (const auto& s : sw) {
            setColor(s.c);
            Renderer::drawRect(x2, yr - 10, 12, 12);
            setColor(Theme::TEXT);
            Renderer::drawText(x2 + 22, yr, s.text, Fonts::body());
            yr += 19;
        }
        (void)yl;
    }

    bool onKey(unsigned char key) override {
        if (key == 27) { navigate(SCREEN_MENU); return true; }
        return false;
    }
};

#endif // GLIDE_HELP_SCREEN_H
