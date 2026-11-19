/*
 * CircleExplorer.h
 * ----------------
 * Step-by-step visualisation of the DDA, Bresenham and Midpoint circle
 * algorithms.
 *
 * Every step computes ONE point in a single octant and mirrors it into
 * the other seven (8-way symmetry). The visualisation makes that visible:
 *   - the octant actually being computed is shaded
 *   - the computed point is drawn in the algorithm colour
 *   - its 7 mirror images are drawn in pink
 *   - the current step is yellow, with spokes from the centre to all
 *     8 points so the symmetry is obvious
 *
 * Interaction: click to move the centre, drag the centre or the rim
 * handle, use the radius slider / [ and ] keys.
 */

#ifndef GLIDE_CIRCLE_EXPLORER_H
#define GLIDE_CIRCLE_EXPLORER_H

#include "ExplorerScreen.h"
#include "../Algorithms/DDACircle.h"
#include "../Algorithms/BresenhamCircle.h"
#include "../Algorithms/MidpointCircle.h"

class CircleExplorer : public ExplorerScreen {
private:
    enum Mode { MODE_DDA, MODE_BRESENHAM, MODE_MIDPOINT, MODE_COMPARE };
    enum Ids { ID_C_DDA = 300, ID_C_BRES, ID_C_MID, ID_C_RMINUS, ID_C_RPLUS,
               ID_C_RADIUS, ID_C_OCTANTS, ID_C_COMPARE };
    enum Handles { H_CENTRE = 0, H_RADIUS = 1 };

    static const int MIN_RADIUS = 1;
    static const int MAX_RADIUS = 60;

    /* Index of the computed (x, y) point inside getSymmetricPoints() */
    static const int PRIMARY_INDEX = 3;

    Mode mode;
    DDACircle dda;
    BresenhamCircle bres;
    MidpointCircle mid;
    Point centre;
    int radius;
    bool showOctants;

    const DrawingAlgorithm& activeAlgo() const {
        if (mode == MODE_DDA) return dda;
        if (mode == MODE_BRESENHAM) return bres;
        return mid;
    }
    Color algoColor() const {
        if (mode == MODE_DDA) return Theme::ACCENT;
        if (mode == MODE_BRESENHAM) return Theme::ACCENT2;
        return Theme::PURPLE;
    }

    void compute() {
        dda.setParameters(centre, (float)radius);
        dda.execute();
        bres.setParameters(centre, (float)radius);
        bres.execute();
        mid.setParameters(centre, (float)radius);
        mid.execute();
        rebuildRace();
    }
    int totalSteps() const {
        return mode == MODE_COMPARE ? compareTotal() : activeAlgo().getStepCount();
    }
    void runFromStart() { compute(); anim.reset(totalSteps(), true); }
    void runToEnd()     { compute(); anim.resetToEnd(totalSteps()); }

    void setMode(Mode m) {
        bool wasCompare = mode == MODE_COMPARE;
        mode = m;
        if (wasCompare != (m == MODE_COMPARE)) needFit = true;   // pane width changes
        if (m == MODE_COMPARE) {
            if (!wasCompare) startRace();
        } else {
            anim.setTotal(totalSteps());
        }
    }

    const DrawingAlgorithm& algoAt(int i) const {
        if (i == 0) return dda;
        if (i == 1) return bres;
        return mid;
    }
    static Color colorAt(int i) {
        return i == 0 ? Theme::ACCENT : (i == 1 ? Theme::ACCENT2 : Theme::PURPLE);
    }

    void setRadius(int r) {
        r = std::max(MIN_RADIUS, std::min(MAX_RADIUS, r));
        if (r == radius) return;
        radius = r;
        runToEnd();
        /* Zoom out if the circle no longer fits on screen */
        float m = radius + 1.0f;
        if (!view.showsWorldRect(centre.x - m, centre.y - m, centre.x + m, centre.y + m)) fitView();
    }

    /* ---------------------------------------------------------------
     * DRAWING
     * --------------------------------------------------------------- */
    void drawOctantGuides(const CoordinateSystem& cs, bool isDDA, const Color& color) const {
        float cx = cs.worldToScreenX(centre.x), cy = cs.worldToScreenY(centre.y);
        float L = (radius + 2.0f) * cs.pixelsPerUnitX();

        /* Shade the octant that the algorithm actually computes:
         * DDA goes from 0 to 45 deg, Bresenham / Midpoint from 90 down to 45 deg. */
        float a0 = isDDA ? 0.0f : 45.0f;
        float a1 = isDDA ? 45.0f : 90.0f;
        setColor(color.withAlpha(0.08f));
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 16; i++) {
            float a = MathUtils::degToRad(a0 + (a1 - a0) * i / 16.0f);
            glVertex2f(cx + L * cosf(a), cy - L * sinf(a));   // screen y is flipped
        }
        glEnd();

        /* Axes and diagonals through the centre split the circle into 8 octants */
        setColor(Theme::TEXT_DIM.withAlpha(0.5f));
        float d = L * 0.7071f;
        Renderer::drawDashedLine(cx - L, cy, cx + L, cy);
        Renderer::drawDashedLine(cx, cy - L, cx, cy + L);
        Renderer::drawDashedLine(cx - d, cy + d, cx + d, cy - d);
        Renderer::drawDashedLine(cx - d, cy - d, cx + d, cy + d);
        Renderer::drawLabel(cx + d + 4, cy - d - 2, "x = y", Theme::TEXT_DIM);
    }

    void drawSubView(UI& /*ui*/, const CoordinateSystem& cs, int index) override {
        if (mode != MODE_COMPARE) {
            drawCircleAlgorithm(cs, activeAlgo(), algoColor(), mode == MODE_DDA, anim.getRevealed());
            return;
        }
        const DrawingAlgorithm& algo = algoAt(index);
        int revealed = revealedFor(index);
        drawCircleAlgorithm(cs, algo, colorAt(index), index == 0, revealed);
        Renderer::drawLabel((float)cs.plotAreaX + 10, (float)cs.plotAreaY + 20,
                            algo.getName(), colorAt(index), Fonts::body());
        bool done = revealed >= algo.getStepCount();
        Renderer::drawLabel((float)cs.plotAreaX + 10, (float)cs.plotAreaY + 40,
                            done ? "DONE at tick " + std::to_string(finishTick(index))
                                 : std::to_string(revealed) + " / " + std::to_string(algo.getStepCount()) + " steps",
                            done ? Theme::GREEN : Theme::TEXT_DIM, Fonts::body());
    }

    void drawCircleAlgorithm(const CoordinateSystem& cs, const DrawingAlgorithm& algo,
                             const Color& base, bool isDDA, int revealedSteps) const {
        float cx = cs.worldToScreenX(centre.x), cy = cs.worldToScreenY(centre.y);

        if (showOctants) drawOctantGuides(cs, isDDA, base);

        if (settings.showIdeal) {
            setColor(Theme::TEXT_PRIMARY.withAlpha(0.45f));
            glLineWidth(1.5f);
            Renderer::drawCircle(cx, cy, radius * cs.pixelsPerUnitX(), false, 180);
            glLineWidth(1.0f);
        }

        const auto& steps = algo.getSteps();
        int revealed = std::min(revealedSteps, (int)steps.size());
        int cur = revealed - 1;

        for (int i = 0; i < revealed; i++) {
            if (i == cur) continue;   // drawn last, on top
            const auto& pts = steps[i].symmetricPoints;
            for (size_t j = 0; j < pts.size(); j++) {
                bool primary = (int)j == PRIMARY_INDEX;
                drawPixel(cs, pts[j].x, pts[j].y,
                          primary ? base.withAlpha(0.8f) : Theme::MIRROR.withAlpha(0.45f));
            }
        }

        if (cur >= 0 && cur < (int)steps.size()) {
            const auto& pts = steps[cur].symmetricPoints;
            setColor(Theme::YELLOW.withAlpha(0.35f));
            for (const auto& p : pts) {
                Renderer::drawLine(cx, cy, cs.worldToScreenX(p.x), cs.worldToScreenY(p.y));
            }
            for (size_t j = 0; j < pts.size(); j++) {
                bool primary = (int)j == PRIMARY_INDEX;
                drawPixel(cs, pts[j].x, pts[j].y,
                          primary ? Theme::YELLOW : Theme::YELLOW.withAlpha(0.5f));
            }
            if (pts.size() > PRIMARY_INDEX) {
                const Point& p = pts[PRIMARY_INDEX];
                labelPixel(cs, p.x, p.y, Format::px(p), Theme::YELLOW);
            }
        }

        /* Handles */
        Renderer::drawHandle(cx, cy, Theme::ACCENT2, drag == DRAG_HANDLE && dragHandle == H_CENTRE);
        Renderer::drawLabel(cx + 12, cy + 18, "C " + Format::px(centre), Theme::ACCENT2);
        float rx = cs.worldToScreenX(centre.x + radius);
        Renderer::drawHandle(rx, cy, Theme::ACCENT, drag == DRAG_HANDLE && dragHandle == H_RADIUS);
        Renderer::drawLabel(rx + 12, cy + 18, "r = " + std::to_string(radius), Theme::ACCENT);
    }

    /* ---------------------------------------------------------------
     * EXPLORER HOOKS
     * --------------------------------------------------------------- */
    std::string title() const override { return "Circle Explorer"; }
    std::string subtitle() const override { return "8-way symmetry: compute one octant, mirror the rest"; }
    bool quizSupported() const override { return mode != MODE_COMPARE; }
    int subViewCount() const override { return mode == MODE_COMPARE ? 3 : 1; }

    std::vector<Entry> allAlgorithms() const override {
        return { { &dda, Theme::ACCENT }, { &bres, Theme::ACCENT2 }, { &mid, Theme::PURPLE } };
    }
    std::vector<Entry> comparedAlgorithms() const override {
        if (mode != MODE_COMPARE) return {};
        return allAlgorithms();
    }
    std::string paramsKey() const override { return Format::px(centre) + std::to_string(radius); }

    std::string plotHint() const override {
        return "Click: move centre   Drag C / r handles   Drag empty space: pan   [ ]: radius";
    }

    void drawToolbar(UI& ui) override {
        float x = 12, y = toolbarR.y + 8;
        Color orange = Theme::ACCENT2, purple = Theme::PURPLE;
        ui.button(Rect(x, y, 70, 28), "DDA", ID_C_DDA, mode == MODE_DDA); x += 74;
        ui.button(Rect(x, y, 96, 28), "Bresenham", ID_C_BRES, mode == MODE_BRESENHAM,
                  true, Fonts::body(), &orange); x += 100;
        ui.button(Rect(x, y, 86, 28), "Midpoint", ID_C_MID, mode == MODE_MIDPOINT,
                  true, Fonts::body(), &purple); x += 90;
        ui.button(Rect(x, y, 100, 28), "Compare all", ID_C_COMPARE, mode == MODE_COMPARE); x += 116;

        drawCaption(x, y + 18, "RADIUS"); x += 48;
        ui.button(Rect(x, y, 28, 28), "-", ID_C_RMINUS, false, radius > MIN_RADIUS, Fonts::title()); x += 36;
        ui.slider(Rect(x, y + 7, 110, 14), (float)radius, (float)MIN_RADIUS, (float)MAX_RADIUS, ID_C_RADIUS);
        x += 118;
        ui.button(Rect(x, y, 28, 28), "+", ID_C_RPLUS, false, radius < MAX_RADIUS, Fonts::title()); x += 34;
        setColor(Theme::TEXT_PRIMARY);
        Renderer::drawText(x, y + 19, std::to_string(radius), Fonts::body()); x += 34;

        x = drawCommonToggles(ui, x, y);
        ui.button(Rect(x, y, 78, 28), "Octants", ID_C_OCTANTS, showOctants);
    }

    void drawPanel(UI& ui) override {
        if (mode == MODE_COMPARE) {
            drawComparePanel(ui, "Three circle algorithms",
                "All three trace one octant and mirror it 8 ways. DDA steps an angle with "
                "float multiplies and rounding; Bresenham uses an integer decision d; "
                "Midpoint uses d = 1.25 - r (a float, as in the notes).");
            return;
        }
        drawAlgorithmPanel(ui, activeAlgo(), algoColor());
    }

    void fitView() override {
        float m = radius + 1.5f;
        view.fitTo(centre.x - m, centre.y - m, centre.x + m, centre.y + m, 24.0f);
    }

    int handleAt(const CoordinateSystem& cs, int mx, int my) const override {
        float cx = cs.worldToScreenX(centre.x), cy = cs.worldToScreenY(centre.y);
        float rx = cs.worldToScreenX(centre.x + radius);
        auto withinHandleRadius = [&](float hx, float hy) {
            return (hx - mx) * (hx - mx) + (hy - my) * (hy - my) <= 12.0f * 12.0f;
        };
        if (withinHandleRadius(rx, cy)) return H_RADIUS;
        if (withinHandleRadius(cx, cy)) return H_CENTRE;
        return -1;
    }

    void onHandleDrag(int handle, float wx, float wy) override {
        if (handle == H_CENTRE) {
            Point c((float)MathUtils::roundToInt(wx), (float)MathUtils::roundToInt(wy));
            if (c == centre) return;
            centre = c;
            runToEnd();
        } else {
            float dx = wx - centre.x, dy = wy - centre.y;
            int r = MathUtils::roundToInt(sqrtf(dx * dx + dy * dy));
            r = std::max(MIN_RADIUS, std::min(MAX_RADIUS, r));
            if (r == radius) return;
            radius = r;
            runToEnd();
        }
    }

    void onPlotClick(int cx, int cy) override {
        centre = Point((float)cx, (float)cy);
        runFromStart();
    }

    /* Quiz: which octant point does the algorithm choose next? */
    bool buildQuiz() override {
        const auto& steps = activeAlgo().getSteps();
        int k = anim.currentIndex();
        if (k < 0) { anim.jumpTo(0); k = 0; }
        if (k + 1 >= (int)steps.size()) return false;

        int x = (int)steps[k].x, y = (int)steps[k].y;
        int nx = (int)steps[k + 1].x, ny = (int)steps[k + 1].y;
        std::vector<std::string> cands;
        std::ostringstream qs;

        if (mode == MODE_DDA) {
            /* DDA walks counter-clockwise from (r, 0): x shrinks, y grows */
            cands = { Format::pt(x, y + 1), Format::pt(x - 1, y + 1), Format::pt(x - 1, y) };
            qs << "DDA circle, r = " << radius << ", dtheta = " << Format::num(1.0f / radius, 4) << "\n";
            qs << "x = " << steps[k].tableCells[2] << ", y = " << steps[k].tableCells[3] << "\n";
            qs << "Current octant point " << Format::pt(x, y) << "\n";
            qs << "Next rounded octant point (relative to C)?";
        } else {
            cands = { Format::pt(x + 1, y), Format::pt(x + 1, y - 1), Format::pt(x, y - 1) };
            qs << (mode == MODE_BRESENHAM ? "Bresenham" : "Midpoint") << " circle, r = " << radius << "\n";
            qs << "Current octant point " << Format::pt(x, y) << "\n";
            qs << "d = " << steps[k].tableCells[3] << "\n";
            qs << "Next octant point (relative to C)?";
        }
        quiz.question = qs.str();
        quiz.explanation = steps[k].explanation;
        quiz.stepIndex = k + 1;
        setQuizOptions(cands, Format::pt(nx, ny));
        return true;
    }

public:
    explicit CircleExplorer(Settings& s)
        : ExplorerScreen(s), mode(MODE_BRESENHAM), centre(0, 0), radius(8), showOctants(true) {
        view.scaleX = 25.0f;
        view.scaleY = 25.0f;
        runToEnd();
    }

    void onWidget(int id) override {
        switch (id) {
            case ID_C_DDA:     setMode(MODE_DDA); return;
            case ID_C_BRES:    setMode(MODE_BRESENHAM); return;
            case ID_C_MID:     setMode(MODE_MIDPOINT); return;
            case ID_C_COMPARE: setMode(MODE_COMPARE); return;
            case ID_C_RMINUS:  setRadius(radius - 1); return;
            case ID_C_RPLUS:   setRadius(radius + 1); return;
            case ID_C_OCTANTS: showOctants = !showOctants; return;
        }
        ExplorerScreen::onWidget(id);
    }

    void onSlider(int id, float value) override {
        if (id == ID_C_RADIUS) { setRadius(MathUtils::roundToInt(value)); return; }
        ExplorerScreen::onSlider(id, value);
    }

    bool onKey(unsigned char key) override {
        if (!quiz.active) {
            switch (key) {
                case '1': setMode(MODE_DDA); return true;
                case '2': setMode(MODE_BRESENHAM); return true;
                case '3': setMode(MODE_MIDPOINT); return true;
                case '4': setMode(MODE_COMPARE); return true;
                case '[': setRadius(radius - 1); return true;
                case ']': setRadius(radius + 1); return true;
                case 'o': case 'O': showOctants = !showOctants; return true;
            }
        }
        return ExplorerScreen::onKey(key);
    }
};

#endif // GLIDE_CIRCLE_EXPLORER_H
