/*
 * LineExplorer.h
 * --------------
 * Step-by-step visualisation of the DDA and Bresenham line algorithms.
 *
 * Interaction:
 *   - Click once to place P1, move the mouse (live preview), click again
 *     to place P2. The algorithm then animates from the first step.
 *   - Drag the P1 / P2 handles to reshape the line; the result updates live.
 *   - Compare mode runs both algorithms side by side and marks pixels
 *     where they disagree.
 *
 * Visual language:
 *   cyan / orange cells  pixels already plotted (DDA / Bresenham)
 *   yellow cell          the pixel chosen in the CURRENT step
 *   white dot            the exact point on the ideal line for this step;
 *                        the short white line shows the rounding error
 */

#ifndef GLIDE_LINE_EXPLORER_H
#define GLIDE_LINE_EXPLORER_H

#include "ExplorerScreen.h"
#include "../Algorithms/DDALine.h"
#include "../Algorithms/BresenhamLine.h"

class LineExplorer : public ExplorerScreen {
private:
    enum Mode { MODE_DDA, MODE_BRESENHAM, MODE_COMPARE };
    enum Ids { ID_L_DDA = 200, ID_L_BRES, ID_L_COMPARE, ID_L_SWAP };
    enum Handles { H_START = 0, H_END = 1 };

    Mode mode;
    DDALine dda;
    BresenhamLine bres;
    Point p1, p2;

    /* Two-click placement */
    bool placing;           // P1 placed, waiting for the P2 click
    Point savedP1;          // to restore if placement is cancelled
    DDALine previewDDA;
    BresenhamLine previewBres;

    /* Compare mode: rows where DDA and Bresenham pick different pixels */
    std::vector<bool> diffRows;
    int diffCount;

    const DrawingAlgorithm& activeAlgo() const {
        return mode == MODE_BRESENHAM ? (const DrawingAlgorithm&)bres : (const DrawingAlgorithm&)dda;
    }
    Color algoColor(bool bresenham) const { return bresenham ? Theme::ACCENT2 : Theme::ACCENT; }

    int totalSteps() const {
        if (mode == MODE_COMPARE) return compareTotal();
        return activeAlgo().getStepCount();
    }

    /* Re-run both algorithms on the current endpoints */
    void compute() {
        dda.setEndpoints(p1, p2);
        dda.execute();
        bres.setEndpoints(p1, p2);
        bres.execute();

        diffRows.assign(std::max(dda.getStepCount(), bres.getStepCount()), false);
        diffCount = 0;
        for (size_t i = 0; i < diffRows.size(); i++) {
            bool differ = true;
            if (i < dda.getSteps().size() && i < bres.getSteps().size()) {
                differ = !(dda.getSteps()[i].plottedPoints[0] == bres.getSteps()[i].plottedPoints[0]);
            }
            diffRows[i] = differ;
            if (differ) diffCount++;
        }
        rebuildRace();
    }

    void runFromStart() { compute(); anim.reset(totalSteps(), true); }
    void runToEnd()     { compute(); anim.resetToEnd(totalSteps()); }

    void setMode(Mode m) {
        bool wasCompare = mode == MODE_COMPARE;
        mode = m;
        if (wasCompare != (m == MODE_COMPARE)) needFit = true;   // pane width changes
        if (m == MODE_COMPARE) {
            if (!wasCompare) startRace();   // entering compare: start the race
        } else {
            anim.setTotal(totalSteps());    // keep the step position when switching
        }
    }

    void cancelPlacing() {
        if (!placing) return;
        placing = false;
        p1 = savedP1;
        runToEnd();
    }

    /* Exact point on the ideal line that step 'i' approximates */
    Point exactPoint(const DrawingAlgorithm& algo, int i) const {
        const AlgorithmStep& s = algo.getSteps()[i];
        if (&algo == &dda) return Point(s.x, s.y);
        float dx = p2.x - p1.x, dy = p2.y - p1.y;
        if (fabsf(dx) < 1e-6f && fabsf(dy) < 1e-6f) return Point(s.x, s.y);
        if (fabsf(dy) > fabsf(dx)) return Point(p1.x + (s.y - p1.y) * dx / dy, s.y);
        return Point(s.x, p1.y + (s.x - p1.x) * dy / dx);
    }

    /* ---------------------------------------------------------------
     * DRAWING
     * --------------------------------------------------------------- */
    void drawEndpoints(const CoordinateSystem& cs, bool showP2) const {
        float x1 = cs.worldToScreenX(p1.x), y1 = cs.worldToScreenY(p1.y);
        bool hot1 = drag == DRAG_HANDLE && dragHandle == H_START;
        Renderer::drawHandle(x1, y1, Theme::GREEN, hot1);
        Renderer::drawLabel(x1 + 12, y1 - 10, "P1 " + Format::px(p1), Theme::GREEN);
        if (showP2) {
            float x2 = cs.worldToScreenX(p2.x), y2 = cs.worldToScreenY(p2.y);
            bool hot2 = drag == DRAG_HANDLE && dragHandle == H_END;
            Renderer::drawHandle(x2, y2, Theme::RED, hot2);
            Renderer::drawLabel(x2 + 12, y2 - 10, "P2 " + Format::px(p2), Theme::RED);
        }
    }

    void drawIdealLine(const CoordinateSystem& cs, const Point& a, const Point& b) const {
        if (!settings.showIdeal) return;
        setColor(Theme::TEXT_PRIMARY.withAlpha(0.55f));
        Renderer::drawLine(cs.worldToScreenX(a.x), cs.worldToScreenY(a.y),
                           cs.worldToScreenX(b.x), cs.worldToScreenY(b.y), 1.5f);
    }

    /* Pixels of one algorithm up to the current animation step */
    void drawAlgorithm(const CoordinateSystem& cs, const DrawingAlgorithm& algo,
                       const Color& color, bool markDiffs, int revealedSteps) const {
        const auto& steps = algo.getSteps();
        int revealed = std::min(revealedSteps, (int)steps.size());
        int cur = revealed - 1;

        for (int i = 0; i < revealed; i++) {
            const Point& p = steps[i].plottedPoints[0];
            drawPixel(cs, p.x, p.y, i == cur ? Theme::YELLOW : color.withAlpha(0.6f));
        }
        if (markDiffs) {
            setColor(Theme::RED);
            for (int i = 0; i < revealed && i < (int)diffRows.size(); i++) {
                if (!diffRows[i]) continue;
                const Point& p = steps[i].plottedPoints[0];
                Renderer::drawPixelCellOutline(cs, p.x, p.y, 2.0f);
            }
        }

        /* Rounding visualisation for the current step */
        if (cur >= 0 && cur < (int)steps.size()) {
            const Point& p = steps[cur].plottedPoints[0];
            Point e = exactPoint(algo, cur);
            float px = cs.worldToScreenX(p.x), py = cs.worldToScreenY(p.y);
            float ex = cs.worldToScreenX(e.x), ey = cs.worldToScreenY(e.y);
            setColor(Theme::TEXT_PRIMARY);
            Renderer::drawLine(ex, ey, px, py, 1.5f);
            Renderer::drawCircle(ex, ey, 3.5f, true);
            setColor(Theme::PLOT_BG);
            Renderer::drawCircle(px, py, 2.0f, true);
            labelPixel(cs, p.x, p.y, Format::px(p), Theme::YELLOW);
        }
    }

    void drawPlacingPreview(const CoordinateSystem& cs, int index) const {
        bool isBres = mode == MODE_BRESENHAM || (mode == MODE_COMPARE && index == 1);
        const DrawingAlgorithm& prev = isBres ? (const DrawingAlgorithm&)previewBres
                                              : (const DrawingAlgorithm&)previewDDA;
        if (mouseInPlot) {
            for (const auto& p : prev.getAllPoints()) {
                drawPixel(cs, p.x, p.y, algoColor(isBres).withAlpha(0.35f));
            }
            Point hp((float)MathUtils::roundToInt(hoverWX), (float)MathUtils::roundToInt(hoverWY));
            float hx = cs.worldToScreenX(hp.x), hy = cs.worldToScreenY(hp.y);
            setColor(Theme::TEXT_PRIMARY.withAlpha(0.7f));
            Renderer::drawDashedLine(cs.worldToScreenX(p1.x), cs.worldToScreenY(p1.y), hx, hy, 1.5f);
            Renderer::drawHandle(hx, hy, Theme::RED.withAlpha(0.6f), false);
            Renderer::drawLabel(hx + 12, hy - 10, "P2 " + Format::px(hp) + " - click to place",
                                Theme::RED);
        }
        drawEndpoints(cs, false);
    }

    /* ---------------------------------------------------------------
     * EXPLORER HOOKS
     * --------------------------------------------------------------- */
    std::string title() const override { return "Line Explorer"; }
    std::string subtitle() const override { return "DDA and Bresenham, one pixel at a time"; }
    int subViewCount() const override { return mode == MODE_COMPARE ? 2 : 1; }
    bool quizSupported() const override { return mode != MODE_COMPARE && !placing; }

    std::string plotHint() const override {
        if (placing) return "Click to place P2   (Esc cancels)";
        return "Click: start a new line   Drag P1/P2: move   Drag empty space: pan";
    }

    void drawToolbar(UI& ui) override {
        float x = 12, y = toolbarR.y + 8;
        ui.button(Rect(x, y, 64, 28), "DDA", ID_L_DDA, mode == MODE_DDA); x += 68;
        Color orange = Theme::ACCENT2;
        ui.button(Rect(x, y, 96, 28), "Bresenham", ID_L_BRES, mode == MODE_BRESENHAM,
                  true, Fonts::body(), &orange); x += 100;
        ui.button(Rect(x, y, 112, 28), "Compare both", ID_L_COMPARE, mode == MODE_COMPARE); x += 128;
        x = drawCommonToggles(ui, x, y) + 12;
        ui.button(Rect(x, y, 106, 28), "Swap P1 / P2", ID_L_SWAP, false, !placing); x += 118;

        std::string info = "P1 " + Format::px(p1) + "  ->  P2 " + Format::px(p2);
        int tw = Renderer::textWidth(info, Fonts::body());
        if (x + tw < toolbarR.right() - 12) {
            setColor(Theme::TEXT_DIM);
            Renderer::drawText(toolbarR.right() - 12 - tw, y + 19, info, Fonts::body());
        }
    }

    void drawSubView(UI& /*ui*/, const CoordinateSystem& cs, int index) override {
        if (placing) {
            drawPlacingPreview(cs, index);
            return;
        }
        drawIdealLine(cs, p1, p2);
        if (mode == MODE_COMPARE) {
            bool isBres = index == 1;
            const DrawingAlgorithm& algo = isBres ? (const DrawingAlgorithm&)bres : (const DrawingAlgorithm&)dda;
            int revealed = revealedFor(index);
            drawAlgorithm(cs, algo, algoColor(isBres), true, revealed);
            drawEndpoints(cs, true);
            Renderer::drawLabel((float)cs.plotAreaX + 10, (float)cs.plotAreaY + 20,
                                isBres ? "BRESENHAM  (integer decision e)" : "DDA  (float increments + round)",
                                algoColor(isBres), Fonts::body());
            std::string status = revealed >= algo.getStepCount()
                ? "DONE at tick " + std::to_string(finishTick(index))
                : std::to_string(revealed) + " / " + std::to_string(algo.getStepCount()) + " pixels";
            Renderer::drawLabel((float)cs.plotAreaX + 10, (float)cs.plotAreaY + 40, status,
                                revealed >= algo.getStepCount() ? Theme::GREEN : Theme::TEXT_DIM, Fonts::body());
        } else {
            drawAlgorithm(cs, activeAlgo(), algoColor(mode == MODE_BRESENHAM), false, anim.getRevealed());
            drawEndpoints(cs, true);
        }
    }

    void drawPanel(UI& ui) override {
        float x = panelR.x + 14, w = panelR.w - 28;
        if (placing) {
            float y = panelR.y + 26;
            setColor(Theme::TEXT_PRIMARY);
            Renderer::drawText(x, y, "Placing a new line", Fonts::title());
            y += 26;
            setColor(Theme::TEXT_DIM);
            y = Renderer::drawWrapped(x, y, w,
                "P1 is at " + Format::px(p1) + ". Move the mouse to preview the line, "
                "then click to place P2. Press Esc to cancel.", Fonts::body()) + 10;
            const DrawingAlgorithm& prev = mode == MODE_BRESENHAM
                ? (const DrawingAlgorithm&)previewBres : (const DrawingAlgorithm&)previewDDA;
            if (mouseInPlot) {
                y = drawSection(x, y, w, "PREVIEW");
                setColor(Theme::TEXT_PRIMARY);
                for (const auto& line : prev.getSetupLines()) {
                    y = Renderer::drawWrapped(x, y, w, line, Fonts::mono());
                }
            }
            return;
        }
        if (mode != MODE_COMPARE) {
            drawAlgorithmPanel(ui, activeAlgo(), algoColor(mode == MODE_BRESENHAM));
            return;
        }
        drawLineComparePanel(ui);
    }

    void drawLineComparePanel(UI& ui) {
        float x = panelR.x + 14, w = panelR.w - 28;
        float y = drawComparePanel(ui, "DDA vs Bresenham",
            "Same line, two algorithms. DDA adds float increments and rounds each "
            "pixel; Bresenham decides with integer additions only. " +
            std::string(diffCount ? std::to_string(diffCount) + " pixel(s) differ (red)."
                                  : "Here they choose identical pixels - the difference is cost."));

        /* Side-by-side table with whatever room is left */
        int rd = revealedFor(0), rb = revealedFor(1);
        std::vector<std::vector<std::string>> rows;
        std::vector<bool> flags;
        size_t n = std::max(dda.getSteps().size(), bres.getSteps().size());
        for (size_t i = 0; i < n; i++) {
            std::vector<std::string> r = { std::to_string(i + 1) };
            if ((int)i < rd && i < dda.getSteps().size()) {
                const auto& s = dda.getSteps()[i];
                r.push_back(Format::num(s.y, 2));
                r.push_back(Format::px(s.plottedPoints[0]));
            } else { r.push_back(".."); r.push_back(".."); }
            if ((int)i < rb && i < bres.getSteps().size()) {
                const auto& s = bres.getSteps()[i];
                r.push_back(Format::px(s.plottedPoints[0]));
                r.push_back(std::to_string((int)s.decisionVariable));
            } else { r.push_back(".."); r.push_back(".."); }
            rows.push_back(r);
            flags.push_back((int)i < std::min(rd, rb) && i < diffRows.size() && diffRows[i]);
        }
        int shown = std::max(rd, rb);
        drawTable(ui, Rect(x, y, w, panelR.bottom() - 8 - y), { "#", "DDA y", "DDA", "Bres", "e" },
                  rows, &flags, shown, -1);
    }

    std::vector<Entry> allAlgorithms() const override {
        return { { &dda, Theme::ACCENT }, { &bres, Theme::ACCENT2 } };
    }
    std::vector<Entry> comparedAlgorithms() const override {
        if (mode != MODE_COMPARE || placing) return {};
        return allAlgorithms();
    }
    std::string paramsKey() const override { return Format::px(p1) + Format::px(p2); }

    void fitView() override {
        float minX = std::min(p1.x, p2.x), maxX = std::max(p1.x, p2.x);
        float minY = std::min(p1.y, p2.y), maxY = std::max(p1.y, p2.y);
        /* Keep at least a 12 x 8 window so short lines are not blown up */
        float cx = (minX + maxX) / 2, cy = (minY + maxY) / 2;
        float hw = std::max((maxX - minX) / 2 + 1.5f, 6.0f);
        float hh = std::max((maxY - minY) / 2 + 1.5f, 4.0f);
        view.fitTo(cx - hw, cy - hh, cx + hw, cy + hh, 30.0f);
    }

    int handleAt(const CoordinateSystem& cs, int mx, int my) const override {
        auto withinHandleRadius = [&](const Point& p) {
            float dx = cs.worldToScreenX(p.x) - mx, dy = cs.worldToScreenY(p.y) - my;
            return dx * dx + dy * dy <= 12.0f * 12.0f;
        };
        if (placing) return -1;
        if (withinHandleRadius(p2)) return H_END;
        if (withinHandleRadius(p1)) return H_START;
        return -1;
    }

    void onHandleDrag(int handle, float wx, float wy) override {
        Point p((float)MathUtils::roundToInt(wx), (float)MathUtils::roundToInt(wy));
        Point& target = (handle == H_START) ? p1 : p2;
        if (target == p) return;
        target = p;
        runToEnd();   // live update while dragging: show the complete result
    }

    void onPlotClick(int cx, int cy) override {
        Point p((float)cx, (float)cy);
        if (!placing) {
            savedP1 = p1;
            p1 = p;
            placing = true;
            anim.reset(0, false);
            onHoverChanged();
        } else {
            p2 = p;
            placing = false;
            runFromStart();
        }
    }

    void onHoverChanged() override {
        if (!placing || !mouseInPlot) return;
        Point hp((float)MathUtils::roundToInt(hoverWX), (float)MathUtils::roundToInt(hoverWY));
        previewDDA.setEndpoints(p1, hp);
        previewDDA.execute();
        previewBres.setEndpoints(p1, hp);
        previewBres.execute();
    }

    /* Quiz: given the current pixel, which pixel comes next? */
    bool buildQuiz() override {
        const DrawingAlgorithm& algo = activeAlgo();
        const auto& steps = algo.getSteps();
        int k = anim.currentIndex();
        if (k < 0) { anim.jumpTo(0); k = 0; }
        if (k + 1 >= (int)steps.size()) return false;

        Point p = steps[k].plottedPoints[0];
        Point q = steps[k + 1].plottedPoints[0];
        float dx = p2.x - p1.x, dy = p2.y - p1.y;
        int sx = dx >= 0 ? 1 : -1, sy = dy >= 0 ? 1 : -1;
        bool steep = fabsf(dy) > fabsf(dx);
        Point major = steep ? Point(0, (float)sy) : Point((float)sx, 0);
        Point minor = steep ? Point((float)sx, 0) : Point(0, (float)sy);

        std::vector<std::string> cands = {
            Format::px(Point(p.x + major.x, p.y + major.y)),
            Format::px(Point(p.x + major.x + minor.x, p.y + major.y + minor.y)),
            Format::px(Point(p.x + minor.x, p.y + minor.y))
        };

        std::ostringstream qs;
        if (mode == MODE_DDA) {
            const auto& s = steps[k];
            qs << "DDA line " << Format::px(p1) << " -> " << Format::px(p2) << "\n";
            qs << "Current: x = " << Format::num(s.x) << ", y = " << Format::num(s.y) << "\n";
            qs << "Plotted pixel " << Format::px(p) << "\n";
            qs << "x_inc = " << Format::num(dx / (steps.size() - 1)) << ", y_inc = "
               << Format::num(dy / (steps.size() - 1)) << "\n";
            quiz.explanation = steps[k + 1].explanation;
        } else {
            qs << "Bresenham line " << Format::px(p1) << " -> " << Format::px(p2) << "\n";
            qs << "Current pixel " << Format::px(p) << ",  e = "
               << (int)steps[k].decisionVariable << "\n";
            int adx = abs((int)dx), ady = abs((int)dy);
            int maj = steep ? ady : adx, min = steep ? adx : ady;
            qs << "2*minor = " << 2 * min << ",  2*major = " << 2 * maj << "\n";
            quiz.explanation = steps[k].explanation;
        }
        qs << "Which pixel is plotted next?";
        quiz.question = qs.str();
        quiz.stepIndex = k + 1;
        setQuizOptions(cands, Format::px(q));
        return true;
    }

public:
    explicit LineExplorer(Settings& s)
        : ExplorerScreen(s), mode(MODE_DDA), p1(2, 2), p2(12, 8),
          placing(false), diffCount(0) {
        runToEnd();
    }

    void onWidget(int id) override {
        switch (id) {
            case ID_L_DDA:     setMode(MODE_DDA); return;
            case ID_L_BRES:    setMode(MODE_BRESENHAM); return;
            case ID_L_COMPARE: setMode(MODE_COMPARE); return;
            case ID_L_SWAP:    std::swap(p1, p2); runFromStart(); return;
        }
        ExplorerScreen::onWidget(id);
    }

    bool onKey(unsigned char key) override {
        if (!quiz.active) {
            switch (key) {
                case 27:  if (placing) { cancelPlacing(); return true; } return false;
                case '1': setMode(MODE_DDA); return true;
                case '2': setMode(MODE_BRESENHAM); return true;
                case '3': setMode(MODE_COMPARE); return true;
                case 'x': case 'X': onWidget(ID_L_SWAP); return true;
            }
        }
        return ExplorerScreen::onKey(key);
    }

    void onMouseDown(int button, int mx, int my) override {
        /* Right-click while placing cancels instead of panning */
        if (placing && button != GLUT_LEFT_BUTTON) { cancelPlacing(); return; }
        ExplorerScreen::onMouseDown(button, mx, my);
    }
};

#endif // GLIDE_LINE_EXPLORER_H
