#ifndef GLIDE_POLYGON_EXPLORER_H
#define GLIDE_POLYGON_EXPLORER_H

#include "Screen.h"
#include "../Algorithms/PolygonFill.h"

class PolygonExplorer : public Screen {
    using Cell = PolygonFill::Cell;
    using Colour = PolygonFill::Colour;
    using Method = PolygonFill::Method;

    struct Choice {
        Method value;
        const char* label;
        const char* explanation;
    };

    static constexpr std::array<Choice, 5> methods{{
        {Method::Boundary4, "Boundary 4", "Moves through side neighbours and stops at the orange boundary."},
        {Method::Boundary8, "Boundary 8", "Moves through side and corner neighbours and stops at orange cells. Try the diamond to see how the fill can slip between diagonal boundary cells."},
        {Method::Flood4, "Flood 4", "Replaces cells that match the seed colour and share a side with the growing region."},
        {Method::Flood8, "Flood 8", "Replaces cells that match the seed colour and touch the growing region along a side or at a corner."},
        {Method::Scanline, "Scanline", "Finds where each row crosses the polygon edges and fills the empty cells between each pair of crossings."}
    }};

    enum Ids { ID_METHOD = 600, ID_CONCAVE = 605, ID_DIAMOND, ID_CUSTOM, ID_UNDO, ID_CLOSE };
    enum class Preset { Concave, Diamond, Custom };

    Preset preset = Preset::Concave;
    size_t selected = 0;
    std::vector<Cell> vertices;
    PolygonFill::Grid canvas{};
    PolygonFill::Steps steps;
    StepAnimator animator;
    Cell seed{};
    Rect gridRect;
    float cellSize = 0;
    bool rejectedSeed = false;
    bool editing = false;
    const char* draftError = nullptr;

    bool scanline() const { return methods[selected].value == Method::Scanline; }

    void recalculate() {
        steps = scanline() ? PolygonFill::scanlineFill(vertices, canvas)
                           : PolygonFill::seedFill(canvas, seed, methods[selected].value);
        animator.reset(static_cast<int>(steps.size()), false);
        rejectedSeed = false;
    }

    void loadPreset(Preset next) {
        preset = next;
        canvas = {};
        editing = preset == Preset::Custom;
        draftError = nullptr;
        if (editing) {
            vertices.clear();
            steps.clear();
            animator.reset(0, false);
            return;
        }
        if (preset == Preset::Concave) {
            vertices = {{4, 3}, {19, 3}, {19, 14}, {14, 14}, {14, 8}, {9, 8}, {9, 14}, {4, 14}};
            seed = {11, 5};
            for (int y = 9; y < 13; ++y) {
                for (int x = 6; x < 8; ++x) canvas[y][x] = Colour::Patch;
            }
        } else {
            seed = {12, 9};
            constexpr int radius = 7;
            vertices = {{seed.x, seed.y - radius}, {seed.x + radius, seed.y},
                        {seed.x, seed.y + radius}, {seed.x - radius, seed.y}};
            // The inside and outside have different colours for flood fill.
            for (int y = 0; y < PolygonFill::HEIGHT; ++y) {
                for (int x = 0; x < PolygonFill::WIDTH; ++x) {
                    if (std::abs(x - seed.x) + std::abs(y - seed.y) > radius)
                        canvas[y][x] = Colour::Exterior;
                }
            }
        }
        PolygonFill::drawOutline(canvas, vertices);
        recalculate();
    }

    void closeCustom() {
        if (!editing) return;
        if (vertices.size() < 3) {
            draftError = "Place at least three corners.";
            return;
        }

        PolygonFill::Grid outline{};
        PolygonFill::drawOutline(outline, vertices, true);
        const auto interior = PolygonFill::scanlineFill(vertices, outline);
        if (interior.empty()) {
            draftError = "Leave more space inside the polygon.";
            return;
        }

        for (auto& row : outline)
            for (auto& cell : row)
                if (cell == Colour::Background) cell = Colour::Exterior;
        for (const auto& step : interior)
            for (const Cell cell : step.cells)
                outline[cell.y][cell.x] = Colour::Background;

        canvas = outline;
        seed = interior[interior.size() / 2].cells.front();
        editing = false;
        recalculate();
    }

    void undoCorner() {
        if (!editing || vertices.empty()) return;
        vertices.pop_back();
        draftError = nullptr;
    }

    Rect boundsFor(Cell cell) const {
        return Rect(gridRect.x + cell.x * cellSize, gridRect.y + cell.y * cellSize, cellSize, cellSize);
    }

    static void drawSwatch(float x, float y, const Color& color, const char* label) {
        setColor(color);
        Renderer::drawRect(x, y - 10, 10, 10);
        setColor(Theme::TEXT_DIM);
        Renderer::drawText(x + 17, y, label, Fonts::small());
    }

    void renderGrid(const Rect& plot) {
        setColor(Theme::PLOT_BG);
        Renderer::drawRect(plot);
        setColor(Theme::BORDER);
        Renderer::drawRectOutline(plot);

        cellSize = std::max(0.0f, std::floor(std::min((plot.w - 20) / PolygonFill::WIDTH,
                                                     (plot.h - 20) / PolygonFill::HEIGHT)));
        const float w = cellSize * PolygonFill::WIDTH, h = cellSize * PolygonFill::HEIGHT;
        gridRect = Rect(plot.cx() - w / 2, plot.cy() - h / 2, w, h);
        if (cellSize < 2) return;
        setColor(Theme::PIXEL_GRID);
        Renderer::drawRect(gridRect);
        for (int y = 0; y < PolygonFill::HEIGHT; ++y) {
            for (int x = 0; x < PolygonFill::WIDTH; ++x) {
                const auto color = canvas[y][x];
                setColor(color == Colour::Boundary ? Theme::ACCENT2 :
                         color == Colour::Patch ? Theme::PURPLE :
                         color == Colour::Exterior ? Theme::BG : Theme::PLOT_BG);
                Renderer::drawRect(boundsFor({x, y}).inset(0.5f));
            }
        }
        if (editing) {
            setColor(Theme::ACCENT2);
            for (size_t i = 1; i < vertices.size(); ++i) {
                const Rect a = boundsFor(vertices[i - 1]), b = boundsFor(vertices[i]);
                Renderer::drawLine(a.cx(), a.cy(), b.cx(), b.cy());
            }
            for (const Cell vertex : vertices) {
                const Rect bounds = boundsFor(vertex);
                Renderer::drawCircle(bounds.cx(), bounds.cy(), cellSize * 0.22f, true, 16);
            }
            return;
        }
        const int current = animator.currentIndex();
        for (int i = 0; i < animator.getRevealed(); ++i) {
            setColor(i == current ? Theme::YELLOW : Theme::ACCENT);
            for (Cell cell : steps[i].cells) Renderer::drawRect(boundsFor(cell).inset(0.5f));
        }
        if (scanline() && current >= 0) {
            setColor(Theme::YELLOW);
            Renderer::drawRectOutline(Rect(gridRect.x, boundsFor({0, steps[current].row}).y,
                                            gridRect.w, cellSize));
        }
        if (!scanline()) {
            setColor(Theme::GREEN);
            const Rect marker = boundsFor(seed);
            Renderer::drawCircle(marker.cx(), marker.cy(), cellSize * 0.24f, true, 16);
        }
    }

    void renderDetails(const Rect& panel) {
        setColor(Theme::PANEL);
        Renderer::drawRect(panel);
        setColor(Theme::BORDER);
        Renderer::drawRectOutline(panel);

        const float x = panel.x + 14;
        const float width = panel.w - 28;
        const float limit = panel.bottom() - 12;
        float y = panel.y + 27;
        setColor(Theme::ACCENT);
        if (editing) {
            Renderer::drawText(x, y, "Draw a polygon", Fonts::title());
            y = Renderer::drawWrapped(x, y + 25, width,
                "Click grid cells to place corners in order. Use Undo to remove the last corner. Select Close shape when done, then choose a fill method.",
                Fonts::body(), limit);
            setColor(Theme::YELLOW);
            Renderer::drawWrapped(x, y + 12, width,
                draftError ? draftError : std::to_string(vertices.size()) + " corners placed.",
                Fonts::body(), limit);
            return;
        }
        Renderer::drawText(x, y, methods[selected].label, Fonts::title());
        y = Renderer::drawWrapped(x, y + 25, width, methods[selected].explanation, Fonts::body(), limit);
        const char* hint = preset == Preset::Concave
            ? "The purple patch has a different colour. Compare Boundary Fill and Flood Fill."
            : "The inside and outside use different colours. Flood fill follows the seed colour.";
        setColor(Theme::TEXT_DIM);
        y = Renderer::drawWrapped(x, y + 9, width, hint, Fonts::body(), limit);
        if (!scanline()) {
            const std::string seedLabel = "Seed (" + std::to_string(seed.x) + ", " +
                                          std::to_string(seed.y) + "). Click a cell to move it.";
            y = Renderer::drawWrapped(x, y + 9, width, seedLabel, Fonts::body(), limit);
        }

        std::string status = "Press Next or Play to start.";
        const int current = animator.currentIndex();
        if (rejectedSeed) {
            status = preset == Preset::Custom ? "Choose a cell inside the polygon."
                                             : "Choose a cell away from the boundary.";
        } else if (current >= 0 && scanline()) {
            status = "Row " + std::to_string(steps[current].row) + ": " +
                     std::to_string(steps[current].cells.size()) + " cells filled.";
        } else if (current >= 0) {
            const Cell cell = steps[current].cells.front();
            status = "Filled (" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + ").";
        }
        setColor(Theme::YELLOW);
        y = Renderer::drawWrapped(x, y + 12, width, status, Fonts::body(), limit);

        y += 17;
        if (y + 80 > limit) return;
        drawSwatch(x, y, Theme::ACCENT2, "Boundary");
        drawSwatch(x, y + 20, Theme::PURPLE, "Different colour");
        drawSwatch(x, y + 40, Theme::ACCENT, "Filled");
        drawSwatch(x, y + 60, Theme::YELLOW, "Current step");
        if (!scanline()) drawSwatch(x, y + 80, Theme::GREEN, "Seed");
    }

public:
    explicit PolygonExplorer(Settings& s) : Screen(s) { loadPreset(Preset::Concave); }

    void onEnter() override { animator.pause(); }

    void render(UI& ui) override {
        const float w = static_cast<float>(ui.width()), h = static_cast<float>(ui.height());
        drawHeader(ui, "Polygon Explorer", w >= 690 ? "Boundary fill, flood fill and scanline" : "");

        const float top = Layout::HEADER_H + 2 * Layout::TOOLBAR_H;
        drawBand(Rect(0, Layout::HEADER_H, w, 2 * Layout::TOOLBAR_H));
        const float methodW = std::min(125.0f, (w - 6 * Layout::GAP) / methods.size());
        for (size_t i = 0; i < methods.size(); ++i) {
            ui.button(Rect(Layout::GAP + i * (methodW + Layout::GAP), Layout::HEADER_H + 7, methodW, 30),
                      methods[i].label, ID_METHOD + static_cast<int>(i), selected == i,
                      true, w < 510 ? Fonts::small() : Fonts::body());
        }
        const float presetY = Layout::HEADER_H + Layout::TOOLBAR_H + 5;
        const float presetW = std::clamp((w - 6 * Layout::GAP) / 5.0f, 48.0f, 120.0f);
        const auto presetButton = [&](int column, const char* label, int id, bool active, bool enabled = true) {
            const float x = Layout::GAP + column * (presetW + Layout::GAP);
            ui.button(Rect(x, presetY, presetW, 30), label, id, active, enabled,
                      w < 510 ? Fonts::small() : Fonts::body());
        };
        presetButton(0, "Concave", ID_CONCAVE, preset == Preset::Concave);
        presetButton(1, "Diamond", ID_DIAMOND, preset == Preset::Diamond);
        presetButton(2, preset == Preset::Custom ? "New shape" : "Custom", ID_CUSTOM, preset == Preset::Custom);
        if (editing) {
            presetButton(3, "Undo", ID_UNDO, false, !vertices.empty());
            presetButton(4, w < 400 ? "Close" : "Close shape", ID_CLOSE, false, vertices.size() >= 3);
        }

        const float panelW = std::min(310.0f, w * 0.36f);
        const float contentY = top + Layout::GAP;
        const float contentH = std::max(0.0f, h - Layout::NAV_H - 2 * Layout::GAP - top);
        renderGrid(Rect(Layout::GAP, contentY, w - panelW - 3 * Layout::GAP, contentH));
        renderDetails(Rect(w - panelW - Layout::GAP, contentY, panelW, contentH));

        const Rect bar(0, h - Layout::NAV_H, w, Layout::NAV_H);
        if (w >= 620) {
            drawPlaybackBar(ui, bar, animator, 0, scanline() ? "Row" : "Step");
        } else {
            drawBand(bar);
            ui.button(Rect(8, bar.y + 11, 64, 32), "< Prev", ID_PB_PREV);
            ui.button(Rect(80, bar.y + 11, 72, 32), animator.isPlaying() ? "Pause" : "Play", ID_PB_PLAY);
            ui.button(Rect(160, bar.y + 11, 64, 32), "Next >", ID_PB_NEXT);
            ui.button(Rect(232, bar.y + 11, 34, 32), ">|", ID_PB_LAST);
            setColor(Theme::TEXT_PRIMARY);
            Renderer::drawText(278, bar.y + 31, std::to_string(animator.getRevealed()) + " / " +
                               std::to_string(animator.getTotal()), Fonts::body());
        }
    }

    void onWidget(int id) override {
        if (handlePlaybackWidget(id, animator)) return;
        if (id >= ID_METHOD && id < ID_METHOD + static_cast<int>(methods.size())) {
            selected = static_cast<size_t>(id - ID_METHOD);
            if (!editing) recalculate();
        } else if (id == ID_CONCAVE || id == ID_DIAMOND) {
            loadPreset(id == ID_CONCAVE ? Preset::Concave : Preset::Diamond);
        } else if (id == ID_CUSTOM) {
            loadPreset(Preset::Custom);
        } else if (id == ID_UNDO) {
            undoCorner();
        } else if (id == ID_CLOSE) {
            closeCustom();
        }
    }

    void onSlider(int id, float value) override { handlePlaybackSlider(id, value, animator); }

    void onMouseDown(int button, int x, int y) override {
        if (button != GLUT_LEFT_BUTTON || (!editing && scanline()) || cellSize < 2 || !gridRect.contains(x, y)) return;
        const Cell clicked{static_cast<int>((x - gridRect.x) / cellSize),
                           static_cast<int>((y - gridRect.y) / cellSize)};
        if (editing) {
            const auto sameCell = [clicked](Cell vertex) { return vertex.x == clicked.x && vertex.y == clicked.y; };
            if (std::any_of(vertices.begin(), vertices.end(), sameCell)) {
                draftError = "Choose a cell that is not already a corner.";
                return;
            }
            vertices.push_back(clicked);
            draftError = nullptr;
            return;
        }
        const auto colour = canvas[clicked.y][clicked.x];
        if (colour == Colour::Boundary || (preset == Preset::Custom && colour == Colour::Exterior)) {
            rejectedSeed = true;
            return;
        }
        seed = clicked;
        recalculate();
    }

    int cursorFor(int x, int y) const override {
        return (editing || !scanline()) && cellSize >= 2 && gridRect.contains(x, y)
             ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_LEFT_ARROW;
    }

    bool onKey(unsigned char key) override {
        if (key >= '1' && key <= '5') {
            onWidget(ID_METHOD + key - '1');
            return true;
        }
        switch (key) {
            case 'c': case 'C': loadPreset(Preset::Concave); return true;
            case 'd': case 'D': loadPreset(Preset::Diamond); return true;
            case 'x': case 'X': loadPreset(Preset::Custom); return true;
            case 8: case 127: case 'u': case 'U': undoCorner(); return editing;
            case 13: case 10:
                if (editing) closeCustom();
                else animator.play();
                return true;
            default: return handlePlaybackKey(key, animator);
        }
    }

    bool onSpecialKey(int key) override {
        switch (key) {
            case GLUT_KEY_HOME: animator.toStart(); return true;
            case GLUT_KEY_END: animator.toEnd(); return true;
            case GLUT_KEY_PAGE_UP: animator.prev(); return true;
            case GLUT_KEY_PAGE_DOWN: animator.next(); return true;
            default: return false;
        }
    }

    bool tick(int elapsed) override { return animator.tick(elapsed, settings.stepDelayMs); }
};

#endif // GLIDE_POLYGON_EXPLORER_H
