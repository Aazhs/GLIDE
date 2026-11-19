/*
 * MenuScreen.h
 * ------------
 * Title page with one card per tool. Keys 1-5 open a card, Esc / 6 quits.
 */

#ifndef GLIDE_MENU_SCREEN_H
#define GLIDE_MENU_SCREEN_H

#include "Screen.h"

class MenuScreen : public Screen {
private:
    enum Ids { ID_M_CARD = 500, ID_M_EXIT = 510 };

    struct Card {
        ScreenId target;
        const char* title;
        const char* body;
        Color color;
    };

    static const std::vector<Card>& cards() {
        static const std::vector<Card> c = {
            { SCREEN_LINES, "Line Explorer",
              "DDA vs Bresenham, step by step. Place endpoints, watch every decision, "
              "then race them to see the cost difference.", Theme::ACCENT },
            { SCREEN_CIRCLES, "Circle Explorer",
              "DDA, Bresenham and Midpoint circles with 8-way symmetry. Compare speed, "
              "accuracy and pixel quality side by side.", Theme::ACCENT2 },
            { SCREEN_GRAPH, "Graph Plotter",
              "Plot y = f(x) where every pixel is produced by DDA or Bresenham. "
              "Step through the rasterisation of a real curve.", Theme::GREEN },
            { SCREEN_HELP, "Help & Controls",
              "Keyboard shortcuts, mouse controls and what each colour on screen means.",
              Theme::PURPLE },
            { SCREEN_POLYGONS, "Polygon Explorer",
              "Watch boundary fill, flood fill and scanline filling on a pixel grid.",
              Theme::YELLOW },
        };
        return c;
    }

public:
    explicit MenuScreen(Settings& s) : Screen(s) {}

    void render(UI& ui) override {
        float w = (float)ui.width(), h = (float)ui.height();

        /* Title block */
        setColor(Theme::HEADER);
        Renderer::drawRect(0, 0, w, 150);
        setColor(Theme::BORDER);
        Renderer::drawLine(0, 149.5f, w, 149.5f);

        std::string t = "G  L  I  D  E";
        setColor(Theme::ACCENT);
        Renderer::drawText((w - Renderer::textWidth(t, Fonts::title())) / 2, 62, t, Fonts::title());
        std::string sub = "Graphics Learning & Interactive Demonstration Environment";
        setColor(Theme::TEXT_PRIMARY);
        Renderer::drawText((w - Renderer::textWidth(sub, Fonts::body())) / 2, 90, sub, Fonts::body());
        std::string sub2 = "See how classic rasterisation algorithms decide every single pixel";
        setColor(Theme::TEXT_DIM);
        Renderer::drawText((w - Renderer::textWidth(sub2, Fonts::small())) / 2, 114, sub2, Fonts::small());

        const auto& cs = cards();
        const size_t columns = w >= 990 ? 3 : 2;
        const size_t rows = (cs.size() + columns - 1) / columns;
        constexpr float gap = 18;
        const float cardW = std::min(420.0f, (w - 48 - gap * (columns - 1)) / columns);
        const float cardH = std::clamp((h - 257 - gap * (rows - 1)) / rows, 70.0f, 126.0f);
        const float gridW = cardW * columns + gap * (columns - 1);
        const float gridH = cardH * rows + gap * (rows - 1);
        const float x0 = (w - gridW) / 2, y0 = std::max(170.0f, (h - gridH) / 2 + 35);
        for (size_t i = 0; i < cs.size(); i++) {
            Rect r(x0 + (i % columns) * (cardW + gap), y0 + (i / columns) * (cardH + gap),
                   cardW, cardH);
            bool hover = ui.region(r, ID_M_CARD + (int)i);
            setColor(hover ? Theme::PANEL_ALT : Theme::PANEL);
            Renderer::drawRect(r);
            setColor(hover ? cs[i].color : Theme::BORDER);
            Renderer::drawRectOutline(r, hover ? 1.5f : 1.0f);
            setColor(cs[i].color);
            Renderer::drawRect(r.x, r.y, 4, r.h);

            setColor(cs[i].color);
            Renderer::drawText(r.x + 22, r.y + 36, cs[i].title, Fonts::title());
            std::string key = "press " + std::to_string(i + 1);
            setColor(Theme::TEXT_FAINT);
            Renderer::drawText(r.right() - 16 - Renderer::textWidth(key, Fonts::small()), r.y + 34,
                               key, Fonts::small());
            setColor(Theme::TEXT_DIM);
            Renderer::drawWrapped(r.x + 22, r.y + 62, r.w - 44, cs[i].body, Fonts::body(), r.bottom() - 8);
        }

        float by = y0 + gridH + 20;
        ui.button(Rect((w - 120) / 2, by, 120, 32), "Exit", ID_M_EXIT);

        setColor(Theme::TEXT_FAINT);
        std::string foot = "Algorithm implementations are based on the algorithms taught in the course notes.";
        Renderer::drawText((w - Renderer::textWidth(foot, Fonts::small())) / 2, h - 14, foot, Fonts::small());
    }

    void onWidget(int id) override {
        if (id == ID_M_EXIT) { navigate(SCREEN_EXIT); return; }
        int i = id - ID_M_CARD;
        if (i >= 0 && i < (int)cards().size()) navigate(cards()[i].target);
    }

    bool onKey(unsigned char key) override {
        if (key >= '1' && key <= '5') { navigate(cards()[key - '1'].target); return true; }
        if (key == '6' || key == 27) { navigate(SCREEN_EXIT); return true; }
        return false;
    }

    int cursorFor(int, int) const override { return GLUT_CURSOR_LEFT_ARROW; }
};

#endif // GLIDE_MENU_SCREEN_H
