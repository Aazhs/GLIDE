/*
 * GraphScreen.h
 * -------------
 * The graph plotter: type y = f(x) and GLIDE draws it by joining sample
 * points with the DDA or Bresenham line algorithm.
 *
 * "Step through" mode replays the rasterisation pixel by pixel, segment
 * by segment, with the same playback bar as the explorers.
 *
 * Mouse: drag to pan, hover to trace the curve, +/- buttons to zoom.
 */

#ifndef GLIDE_GRAPH_SCREEN_H
#define GLIDE_GRAPH_SCREEN_H

#include "Screen.h"
#include "../Graph/GraphPlotter.h"
#include "../Common/Benchmark.h"
#include <cstdlib>

class GraphScreen : public Screen {
private:
    enum Ids {
        ID_G_INPUT = 400, ID_G_PLOT, ID_G_DDA, ID_G_BRES, ID_G_STEP, ID_G_HOW,
        ID_G_HOW_CLOSE, ID_G_SAMPLES, ID_G_ZOOM_IN, ID_G_ZOOM_OUT, ID_G_RESET,
        ID_G_PRESET = 420
    };

    GraphPlotter plotter;
    std::string input;
    bool inputActive;
    std::string error;
    bool stepMode;
    bool pendingStepStart;         // step mode chosen before the graph existed
    bool showHow;
    StepAnimator anim;
    std::vector<int> segOffset;    // first flat step index of each segment

    Rect toolbarR, plotR, panelR, bottomR, inputR;

    bool dragging, dragMoved;
    int lastX, lastY, pressX, pressY;
    bool mouseInPlot;
    int mouseX, mouseY;

    /* DDA vs Bresenham on the current graph (cached until the view changes) */
    std::string benchDoneKey;
    int lastViewChangeMs;          // benchmarks wait until the view is still
    double nsDDA, nsBres;
    float costDDA, costBres;

    std::string viewKey() const {
        const CoordinateSystem& cs = plotter.getCoordSystem();
        return plotter.getFunction().getExpression() + "|" + Format::num(cs.panX, 4) + "|" +
               Format::num(cs.panY, 4) + "|" + Format::num(cs.zoom, 4) + "|" +
               std::to_string(plotter.getSampleCount()) + "|" + std::to_string(cs.plotAreaW) + "x" +
               std::to_string(cs.plotAreaH);
    }

    /*
     * Rasterise every segment of the current graph with BOTH algorithms:
     * once with operation counting (model cost) and many times with the
     * lean rasterizers (measured time).
     */
    void refreshBenchmark() {
        std::string key = viewKey();
        if (key == benchDoneKey || dragging || !plotter.graphExists()) return;
        if (glutGet(GLUT_ELAPSED_TIME) - lastViewChangeMs < 250) return;   // still zooming / panning
        benchDoneKey = key;

        const auto& segs = plotter.getSegments();
        costDDA = costBres = 0;
        size_t maxLen = 4;
        for (const auto& sg : segs) {
            DDALine a(sg.screenStart, sg.screenEnd);
            a.execute();
            costDDA += a.getTotalOps().cost();
            BresenhamLine b(sg.screenStart, sg.screenEnd);
            b.execute();
            costBres += b.getTotalOps().cost();
            maxLen = std::max(maxLen, (size_t)std::max(a.getStepCount(), b.getStepCount()) + 2);
        }
        std::vector<int> buf(2 * maxLen + 8);
        nsDDA = Bench::nsPerCall([&]() {
            int t = 0;
            for (const auto& sg : segs) {
                t += DDALine::rasterize(sg.screenStart.x, sg.screenStart.y,
                                        sg.screenEnd.x, sg.screenEnd.y, buf.data());
            }
            return t + buf[0];
        });
        nsBres = Bench::nsPerCall([&]() {
            int t = 0;
            for (const auto& sg : segs) {
                t += BresenhamLine::rasterize(sg.screenStart.pixelX(), sg.screenStart.pixelY(),
                                              sg.screenEnd.pixelX(), sg.screenEnd.pixelY(), buf.data());
            }
            return t + buf[0];
        });
    }

    static std::string fmtTime(double ns) {
        if (ns < 1000) return Format::num((float)ns, 0) + " ns";
        if (ns < 1e6) return Format::num((float)(ns / 1000), 1) + " us";
        return Format::num((float)(ns / 1e6), 2) + " ms";
    }

    static const std::vector<std::string>& presets() {
        static const std::vector<std::string> p = {
            "x^2", "sin(x)", "cos(x)", "2*x+1", "x^3 - 3*x", "sqrt(abs(x))",
            "tan(x)", "1/x", "abs(x) - 2", "x*sin(x)"
        };
        return p;
    }

    /* Recompute the raster after the function or the view changed */
    void recompute() {
        plotter.computeGraph();
        segOffset.clear();
        int total = 0;
        for (const auto& seg : plotter.getSegments()) {
            segOffset.push_back(total);
            total += (int)seg.steps.size();
        }
        anim.setTotal(total);
    }

    /* Flat step index -> (segment, step within segment) */
    bool locate(int flat, int& seg, int& step) const {
        if (flat < 0 || segOffset.empty()) return false;
        auto it = std::upper_bound(segOffset.begin(), segOffset.end(), flat);
        seg = (int)(it - segOffset.begin()) - 1;
        step = flat - segOffset[seg];
        return seg >= 0 && seg < (int)plotter.getSegments().size() &&
               step < (int)plotter.getSegments()[seg].steps.size();
    }

    void plotInput() {
        GraphFunction test(input);
        if (!test.isValid()) {
            error = test.getError();
            return;
        }
        error.clear();
        plotter.setFunction(input);
        recompute();
        anim.reset(anim.getTotal(), stepMode);
    }

    void computeLayout(int w, int h) {
        using namespace Layout;
        float panelW = MathUtils::clamp(w * 0.28f, 280.0f, 380.0f);
        toolbarR = Rect(0, HEADER_H, (float)w, TOOLBAR_H);
        bottomR = Rect(0, h - NAV_H, (float)w, NAV_H);
        float top = HEADER_H + TOOLBAR_H + GAP;
        float ph = std::max(120.0f, h - NAV_H - GAP - top);
        plotR = Rect(GAP, top, std::max(200.0f, w - panelW - 3 * GAP), ph);
        panelR = Rect(plotR.right() + GAP, top, w - plotR.right() - 2 * GAP, ph);

        CoordinateSystem& cs = plotter.getCoordSystem();
        if (cs.plotAreaX != (int)plotR.x || cs.plotAreaY != (int)plotR.y ||
            cs.plotAreaW != (int)plotR.w || cs.plotAreaH != (int)plotR.h) {
            cs.setPlotArea((int)plotR.x, (int)plotR.y, (int)plotR.w, (int)plotR.h);
            cs.screenWidth = w;
            cs.screenHeight = h;
            if (plotter.getFunction().isValid()) recompute();
        }
    }

    void viewChanged() {
        lastViewChangeMs = glutGet(GLUT_ELAPSED_TIME);
        if (plotter.getFunction().isValid()) recompute();
    }

    /*
     * Magnifier for step-through mode: graph pixels are real screen pixels,
     * far too small to follow, so show the neighbourhood of the current
     * pixel enlarged with its pixel grid.
     */
    void drawMagnifier(UI& ui, int seg, int st) {
        const auto& segs = plotter.getSegments();
        const GraphSegment& cur = segs[seg];
        const Point& c = cur.steps[st].plottedPoints[0];
        const int R = 8;               // cells on each side of the centre
        const float cell = 11.0f;
        float size = (2 * R + 1) * cell;
        Rect box(plotR.right() - size - 14, plotR.bottom() - size - 40, size, size);
        int cx = c.pixelX(), cy = c.pixelY();

        auto toBoxX = [&](float sx) { return box.x + (sx - (cx - R) + 0.0f) * cell; };
        auto toBoxY = [&](float sy) { return box.y + (sy - (cy - R) + 0.0f) * cell; };

        setColor(Theme::PLOT_BG);
        Renderer::drawRect(box);
        Renderer::beginClip(box, ui.height());

        setColor(Theme::PIXEL_GRID);
        glBegin(GL_LINES);
        for (int i = 0; i <= 2 * R + 1; i++) {
            glVertex2f(box.x + i * cell, box.y); glVertex2f(box.x + i * cell, box.bottom());
            glVertex2f(box.x, box.y + i * cell); glVertex2f(box.right(), box.y + i * cell);
        }
        glEnd();

        auto cellAt = [&](const Point& p, const Color& col) {
            int dx = p.pixelX() - cx, dy = p.pixelY() - cy;
            if (abs(dx) > R || abs(dy) > R) return;
            setColor(col);
            Renderer::drawRect(box.x + (dx + R) * cell + 1, box.y + (dy + R) * cell + 1, cell - 2, cell - 2);
        };
        Color done = plotter.isUsingDDA() ? Theme::ACCENT : Theme::ACCENT2;
        for (int s = std::max(0, seg - 4); s < seg; s++) {
            for (const auto& p : segs[s].rasterPoints) cellAt(p, done.withAlpha(0.6f));
        }
        for (int i = 0; i < st; i++) cellAt(cur.steps[i].plottedPoints[0], Theme::GREEN.withAlpha(0.8f));
        cellAt(c, Theme::YELLOW);

        /* Ideal segment (pixel centres are at +0.5 inside each cell) */
        setColor(Theme::TEXT.withAlpha(0.8f));
        Renderer::drawLine(toBoxX(cur.screenStart.x + 0.5f), toBoxY(cur.screenStart.y + 0.5f),
                           toBoxX(cur.screenEnd.x + 0.5f), toBoxY(cur.screenEnd.y + 0.5f), 1.5f);
        Renderer::endClip();

        setColor(Theme::YELLOW);
        Renderer::drawRectOutline(box, 1.5f);
        Renderer::drawLabel(box.x + 4, box.y - 6, "magnified x" + std::to_string((int)cell) +
                            "   pixel " + Format::px(c), Theme::YELLOW);
    }

    /* ---------------------------------------------------------------
     * DRAWING
     * --------------------------------------------------------------- */
    void drawToolbar(UI& ui) {
        float x = 12, y = toolbarR.y + 8;
        setColor(Theme::TEXT_DIM);
        Renderer::drawText(x, y + 19, "y =", Fonts::body());
        x += 30;

        float inputW = MathUtils::clamp(toolbarR.w * 0.25f, 180.0f, 340.0f);
        inputR = Rect(x, y, inputW, 28);
        bool hover = ui.region(inputR, ID_G_INPUT);
        setColor(inputActive ? Theme::PLOT_BG : (hover ? Theme::BTN_HOVER : Theme::BTN));
        Renderer::drawRect(inputR);
        setColor(!error.empty() ? Theme::RED : (inputActive ? Theme::ACCENT : Theme::BTN_BORDER));
        Renderer::drawRectOutline(inputR, inputActive ? 1.5f : 1.0f);
        setColor(Theme::TEXT);
        std::string shown = input;
        while (!shown.empty() && Renderer::textWidth(shown, Fonts::body()) > inputW - 16) {
            shown.erase(0, 1);   // keep the end (where the caret is) visible
        }
        Renderer::drawText(inputR.x + 7, y + 19, shown, Fonts::body());
        if (inputActive) {
            float cx = inputR.x + 8 + Renderer::textWidth(shown, Fonts::body());
            setColor(Theme::ACCENT);
            Renderer::drawLine(cx, y + 6, cx, y + 22, 1.5f);
        } else if (input.empty()) {
            setColor(Theme::TEXT_FAINT);
            Renderer::drawText(inputR.x + 7, y + 19, "click and type, e.g. x^2 - 3", Fonts::small());
        }
        x += inputW + 6;

        ui.button(Rect(x, y, 56, 28), "Plot", ID_G_PLOT, true); x += 72;

        Color orange = Theme::ACCENT2;
        ui.button(Rect(x, y, 56, 28), "DDA", ID_G_DDA, plotter.isUsingDDA()); x += 60;
        ui.button(Rect(x, y, 96, 28), "Bresenham", ID_G_BRES, !plotter.isUsingDDA(),
                  true, Fonts::body(), &orange); x += 112;
        ui.button(Rect(x, y, 110, 28), "Step through", ID_G_STEP, stepMode); x += 114;
        ui.button(Rect(x, y, 110, 28), "How it works", ID_G_HOW, showHow); x += 114;
    }

    void drawGraph(UI& ui) {
        CoordinateSystem& cs = plotter.getCoordSystem();
        const auto& segs = plotter.getSegments();

        /* Raster pixels land on integer screen coordinates; +0.5 hits the pixel centre */
        auto vertex = [](const Point& p) { glVertex2f(p.x + 0.5f, p.y + 0.5f); };

        if (!stepMode) {
            setColor(plotter.isUsingDDA() ? Theme::ACCENT : Theme::ACCENT2);
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            for (const auto& pt : plotter.getAllRasterPoints()) vertex(pt);
            glEnd();
        } else {
            int seg = -1, st = -1;
            bool has = locate(anim.currentIndex(), seg, st);
            Color done = plotter.isUsingDDA() ? Theme::ACCENT : Theme::ACCENT2;

            glPointSize(2.0f);
            setColor(done);
            glBegin(GL_POINTS);
            for (int s = 0; has && s < seg; s++) {
                for (const auto& pt : segs[s].rasterPoints) vertex(pt);
            }
            glEnd();

            if (has) {
                const GraphSegment& cur = segs[seg];
                /* The segment being drawn: ideal line + revealed pixels */
                setColor(Theme::TEXT.withAlpha(0.35f));
                Renderer::drawLine(cur.screenStart.x, cur.screenStart.y,
                                   cur.screenEnd.x, cur.screenEnd.y);
                setColor(Theme::GREEN);
                glPointSize(3.0f);
                glBegin(GL_POINTS);
                for (int i = 0; i <= st; i++) {
                    for (const auto& pt : cur.steps[i].plottedPoints) vertex(pt);
                }
                glEnd();

                const Point& px = cur.steps[st].plottedPoints[0];
                setColor(Theme::YELLOW);
                Renderer::drawRectOutline(px.x - 5, px.y - 5, 11, 11, 1.5f);
                setColor(Theme::ACCENT2);
                Renderer::drawCircle(cur.screenStart.x, cur.screenStart.y, 3.5f, true);
                Renderer::drawCircle(cur.screenEnd.x, cur.screenEnd.y, 3.5f, true);
                glPointSize(1.0f);
                drawMagnifier(ui, seg, st);
            }
            glPointSize(1.0f);
        }

        /* Trace: dot on the curve under the mouse x */
        if (mouseInPlot && !dragging && !ui.isModalActive() && plotter.getFunction().isValid()) {
            float wx = cs.screenToWorldX((float)mouseX);
            float wy = plotter.evaluateAt(wx);
            if (MathUtils::isValidFloat(wy)) {
                float sx = (float)mouseX, sy = cs.worldToScreenY(wy);
                if (sy >= plotR.y && sy <= plotR.bottom()) {
                    setColor(Theme::TEXT.withAlpha(0.25f));
                    Renderer::drawDashedLine(sx, plotR.y, sx, plotR.bottom());
                    setColor(Theme::YELLOW);
                    Renderer::drawCircle(sx, sy, 4.5f, true);
                    std::string t = "x = " + Format::num(wx, 2) + ",  y = " + Format::num(wy, 3);
                    float lx = sx + 10;
                    if (lx + Renderer::textWidth(t, Fonts::small()) > plotR.right() - 10) {
                        lx = sx - 14 - Renderer::textWidth(t, Fonts::small());
                    }
                    Renderer::drawLabel(lx, sy - 8, t, Theme::YELLOW);
                }
            }
        }
    }

    void drawPanel(UI& ui) {
        float x = panelR.x + 14, w = panelR.w - 28;
        float y = panelR.y + 26, maxY = panelR.bottom() - 8;

        auto section = [&](const std::string& t, const std::string& right = "") {
            setColor(Theme::TEXT_FAINT);
            Renderer::drawText(x, y, t, Fonts::small());
            if (!right.empty()) {
                setColor(Theme::TEXT_DIM);
                Renderer::drawText(x + w - Renderer::textWidth(right, Fonts::small()), y, right,
                                   Fonts::small());
            }
            setColor(Theme::BORDER);
            Renderer::drawLine(x, y + 5.5f, x + w, y + 5.5f);
            y += 20;
        };

        setColor(Theme::TEXT);
        Renderer::drawText(x, y, "y = " + plotter.getFunction().getExpression(), Fonts::title());
        y += 22;
        if (!error.empty()) {
            setColor(Theme::RED);
            y = Renderer::drawWrapped(x, y, w, "Error: " + error, Fonts::body()) + 4;
        }
        setColor(Theme::TEXT_DIM);
        std::ostringstream info;
        info << (plotter.isUsingDDA() ? "DDA" : "Bresenham") << " | "
             << plotter.getSegments().size() << " segments | "
             << plotter.getAllRasterPoints().size() << " pixels";
        Renderer::drawText(x, y, info.str(), Fonts::small());
        y += 22;

        if (plotter.graphExists() && !benchDoneKey.empty()) {
            section("DDA VS BRESENHAM ON THIS GRAPH");
            float mc = std::max(costDDA, costBres);
            drawBar(x, y, w, mc > 0 ? costDDA / mc : 0, Theme::ACCENT, "DDA cost",
                    Format::num(costDDA, 0), costDDA < costBres);
            y += 17;
            drawBar(x, y, w, mc > 0 ? costBres / mc : 0, Theme::ACCENT2, "Bres cost",
                    Format::num(costBres, 0), costBres < costDDA);
            y += 20;
            double mt = std::max(nsDDA, nsBres);
            drawBar(x, y, w, mt > 0 ? (float)(nsDDA / mt) : 0, Theme::ACCENT, "DDA time",
                    fmtTime(nsDDA), nsDDA < nsBres);
            y += 17;
            drawBar(x, y, w, mt > 0 ? (float)(nsBres / mt) : 0, Theme::ACCENT2, "Bres time",
                    fmtTime(nsBres), nsBres < nsDDA);
            y += 16;
            setColor(Theme::TEXT_FAINT);
            y = Renderer::drawWrapped(x, y, w, "Cost = weighted operation count (model). Time = "
                                      "measured on this CPU for the whole curve.", Fonts::small()) + 10;
        }

        section("SAMPLES", std::to_string(plotter.getSampleCount()) + " points");
        ui.slider(Rect(x + 6, y - 10, w - 12, 14), (float)plotter.getSampleCount(), 10.0f, 600.0f,
                  ID_G_SAMPLES);
        y += 10;
        setColor(Theme::TEXT_FAINT);
        y = Renderer::drawWrapped(x, y, w,
            "Fewer samples = longer straight segments. Try 10 to see the line "
            "algorithm joining the dots.", Fonts::small()) + 8;

        section("EXAMPLES");
        const auto& ps = presets();
        float bw = (w - 6) / 2;
        for (size_t i = 0; i < ps.size(); i++) {
            float bx = x + (i % 2) * (bw + 6);
            ui.button(Rect(bx, y - 12, bw, 24), ps[i], ID_G_PRESET + (int)i,
                      plotter.getFunction().getExpression() == ps[i], true, Fonts::small());
            if (i % 2 == 1) y += 28;
        }
        y += 14;

        if (!stepMode) {
            section("HOW IS THIS DRAWN?");
            setColor(Theme::TEXT_DIM);
            Renderer::drawWrapped(x, y, w,
                "f(x) is sampled at evenly spaced x values. Each pair of neighbouring "
                "samples is joined by the selected line algorithm, and only those "
                "algorithm pixels are drawn. Press 'Step through' to watch it.",
                Fonts::small(), maxY);
            return;
        }

        int seg = -1, st = -1;
        if (!locate(anim.currentIndex(), seg, st)) {
            section("STEP THROUGH");
            setColor(Theme::TEXT_DIM);
            Renderer::drawWrapped(x, y, w, "Press Play (Space) or Next (N) to rasterise the "
                                  "graph one pixel at a time.", Fonts::body(), maxY);
            return;
        }
        const GraphSegment& cs = plotter.getSegments()[seg];
        section("SEGMENT " + std::to_string(seg + 1) + " OF " + std::to_string(plotter.getSegments().size()),
                "pixel " + std::to_string(st + 1) + " / " + std::to_string(cs.steps.size()));
        setColor(Theme::TEXT);
        Renderer::drawText(x, y, "world  " + Format::ptf(cs.worldStart.x, cs.worldStart.y) + " -> " +
                           Format::ptf(cs.worldEnd.x, cs.worldEnd.y), Fonts::mono());
        y += 15;
        Renderer::drawText(x, y, "screen " + Format::ptf(cs.screenStart.x, cs.screenStart.y, 1) + " -> " +
                           Format::ptf(cs.screenEnd.x, cs.screenEnd.y, 1), Fonts::mono());
        y += 22;
        setColor(Theme::TEXT);
        Renderer::drawWrapped(x, y, w, cs.steps[st].explanation, Fonts::mono(), maxY);
    }

    void drawHowItWorks(UI& ui) {
        ui.beginModal();
        float pw = 620, ph = 470;
        Rect box((ui.width() - pw) / 2, (ui.height() - ph) / 2, pw, ph);
        setColor(Theme::PANEL);
        Renderer::drawRect(box);
        setColor(Theme::ACCENT);
        Renderer::drawRectOutline(box, 1.5f);

        float x = box.x + 26, y = box.y + 38;
        Renderer::drawText(x, y, "How is this graph drawn?", Fonts::title());
        y += 32;
        const char* lines[] = {
            "1.  You enter a function, e.g. y = sin(x).",
            "2.  GLIDE evaluates it at many evenly spaced x values,",
            "    producing sample points (x, y).",
            "3.  Each sample is converted to screen pixels using the",
            "    current pan and zoom (note: screen y points DOWN).",
            "4.  Neighbouring samples are joined by the chosen line",
            "    algorithm - DDA or Bresenham - after clipping the",
            "    segment to the plot (Liang-Barsky).",
            "5.  Every pixel the algorithm outputs is drawn with",
            "    GL_POINTS. OpenGL's own line drawing is never used.",
            "6.  Jumps taller than the whole plot (e.g. tan(x) at an",
            "    asymptote) are treated as breaks, not joined.",
            "",
            "So the curve you see is built entirely from the classic",
            "rasterisation algorithms you can explore step by step."
        };
        setColor(Theme::TEXT);
        for (const char* l : lines) {
            Renderer::drawText(x, y, l, Fonts::body());
            y += 20;
        }
        ui.button(Rect(box.right() - 106, box.bottom() - 46, 84, 30), "Close", ID_G_HOW_CLOSE, true);
    }

public:
    explicit GraphScreen(Settings& s)
        : Screen(s), input("sin(x)"), inputActive(false), stepMode(false), pendingStepStart(false), showHow(false),
          dragging(false), dragMoved(false), lastX(0), lastY(0), pressX(0), pressY(0),
          mouseInPlot(false), mouseX(0), mouseY(0),
          lastViewChangeMs(-100000), nsDDA(0), nsBres(0), costDDA(0), costBres(0) {
        plotter.getCoordSystem().scaleX = 40.0f;
        plotter.getCoordSystem().scaleY = 40.0f;
        plotter.setFunction(input);
    }

    bool hasModal() const override { return showHow; }
    void closeModal() override { showHow = false; }
    bool wantsTextInput() const override { return inputActive; }

    void render(UI& ui) override {
        computeLayout(ui.width(), ui.height());
        if (!plotter.graphExists() && plotter.getFunction().isValid()) recompute();
        if (pendingStepStart && anim.getTotal() > 0) {
            pendingStepStart = false;
            anim.reset(anim.getTotal(), true);
        }
        refreshBenchmark();

        drawHeader(ui, "Graph Plotter", "every pixel comes from DDA or Bresenham");
        drawBand(toolbarR);
        drawToolbar(ui);

        CoordinateSystem& cs = plotter.getCoordSystem();
        setColor(Theme::PLOT_BG);
        Renderer::drawRect(plotR);
        Renderer::beginClip(plotR, ui.height());
        Renderer::drawGrid(cs, false);
        drawGraph(ui);
        Renderer::endClip();
        setColor(Theme::BORDER);
        Renderer::drawRectOutline(plotR);

        float zx = plotR.right() - 40;
        ui.button(Rect(zx, plotR.y + 10, 30, 28), "+", ID_G_ZOOM_IN, false, true, Fonts::title());
        ui.button(Rect(zx, plotR.y + 42, 30, 28), "-", ID_G_ZOOM_OUT, false, true, Fonts::title());
        ui.button(Rect(zx - 10, plotR.y + 74, 50, 24), "Reset", ID_G_RESET, false, true, Fonts::small());
        Renderer::drawLabel(plotR.x + 10, plotR.bottom() - 24,
                            "Drag: pan   Hover: trace   +/-: zoom   R: reset view", Theme::TEXT_DIM);

        setColor(Theme::PANEL);
        Renderer::drawRect(panelR);
        Renderer::beginClip(panelR, ui.height());
        drawPanel(ui);
        Renderer::endClip();

        if (stepMode) {
            drawPlaybackBar(ui, bottomR, anim, 0);
        } else {
            drawBand(bottomR);
            setColor(Theme::TEXT_DIM);
            Renderer::drawText(14, bottomR.y + 32,
                "Tip: switch DDA / Bresenham and use 'Step through' to watch the "
                "algorithm draw the curve. Lower the samples to see the straight segments.",
                Fonts::small());
        }

        if (showHow) drawHowItWorks(ui);
    }

    void onWidget(int id) override {
        if (stepMode && handlePlaybackWidget(id, anim)) return;
        if (id >= ID_G_PRESET && id < ID_G_PRESET + (int)presets().size()) {
            input = presets()[id - ID_G_PRESET];
            inputActive = false;
            plotInput();
            return;
        }
        if (id != ID_G_INPUT) inputActive = false;
        switch (id) {
            case ID_G_INPUT: inputActive = true; break;
            case ID_G_PLOT:  plotInput(); break;
            case ID_G_DDA:   plotter.setAlgorithm(true);  recompute(); break;
            case ID_G_BRES:  plotter.setAlgorithm(false); recompute(); break;
            case ID_G_STEP:
                stepMode = !stepMode;
                anim.reset(anim.getTotal(), stepMode);
                pendingStepStart = stepMode && anim.getTotal() == 0;
                break;
            case ID_G_HOW:       showHow = !showHow; break;
            case ID_G_HOW_CLOSE: showHow = false; break;
            case ID_G_ZOOM_IN:   plotter.getCoordSystem().zoomIn();  viewChanged(); break;
            case ID_G_ZOOM_OUT:  plotter.getCoordSystem().zoomOut(); viewChanged(); break;
            case ID_G_RESET:     plotter.getCoordSystem().reset();   viewChanged(); break;
        }
    }

    void onSlider(int id, float value) override {
        if (id == ID_G_SAMPLES) {
            int n = MathUtils::roundToInt(value);
            if (n != plotter.getSampleCount()) {
                plotter.setSampleCount(n);
                recompute();
            }
            return;
        }
        handlePlaybackSlider(id, value, anim);
    }

    void onMouseDown(int /*button*/, int mx, int my) override {
        inputActive = false;
        if (!plotR.contains((float)mx, (float)my)) return;
        dragging = true;
        dragMoved = false;
        lastX = pressX = mx;
        lastY = pressY = my;
    }

    void onMouseDrag(int mx, int my) override {
        mouseX = mx; mouseY = my;
        if (!dragging) return;
        if (abs(mx - pressX) > 3 || abs(my - pressY) > 3) dragMoved = true;
        plotter.getCoordSystem().panPixels((float)(mx - lastX), (float)(my - lastY));
        lastX = mx;
        lastY = my;
        viewChanged();
    }

    void onMouseUp(int /*button*/, int /*mx*/, int /*my*/) override { dragging = false; }

    void onMouseMove(int mx, int my) override {
        mouseX = mx;
        mouseY = my;
        mouseInPlot = plotR.contains((float)mx, (float)my);
    }

    void onScroll(int direction, int mx, int my) override {
        if (!plotR.contains((float)mx, (float)my)) return;
        plotter.getCoordSystem().zoomAt(direction > 0 ? 1.15f : 1.0f / 1.15f, (float)mx, (float)my);
        viewChanged();
    }

    bool onKey(unsigned char key) override {
        if (inputActive) {
            if (key == 13 || key == 10) { plotInput(); inputActive = false; }
            else if (key == 27) inputActive = false;
            else if (key == 8 || key == 127) { if (!input.empty()) input.pop_back(); }
            else if (key == 21) input.clear();                       // Ctrl+U clears
            else if (key >= 32 && key < 127 && input.size() < 80) input += (char)key;
            return true;
        }
        if (stepMode && handlePlaybackKey(key, anim)) return true;
        CoordinateSystem& cs = plotter.getCoordSystem();
        switch (key) {
            case '+': case '=': cs.zoomIn();  viewChanged(); return true;
            case '-': case '_': cs.zoomOut(); viewChanged(); return true;
            case 'r': case 'R': cs.reset();   viewChanged(); return true;
            case 13: case 10: case '/': inputActive = true; return true;
            case '[': plotter.setSampleCount(plotter.getSampleCount() - 20); recompute(); return true;
            case ']': plotter.setSampleCount(plotter.getSampleCount() + 20); recompute(); return true;
            case 't': case 'T': onWidget(ID_G_STEP); return true;
            case 'd': case 'D': onWidget(ID_G_DDA); return true;
            case 'b': case 'B': onWidget(ID_G_BRES); return true;
        }
        return false;
    }

    bool onSpecialKey(int key) override {
        if (inputActive) return true;
        CoordinateSystem& cs = plotter.getCoordSystem();
        const float step = 60.0f;
        switch (key) {
            case GLUT_KEY_LEFT:  cs.panPixels(step, 0);  break;
            case GLUT_KEY_RIGHT: cs.panPixels(-step, 0); break;
            case GLUT_KEY_UP:    cs.panPixels(0, step);  break;
            case GLUT_KEY_DOWN:  cs.panPixels(0, -step); break;
            default: return false;
        }
        viewChanged();
        return true;
    }

    bool tick(int dtMs) override {
        bool changed = stepMode && anim.tick(dtMs, settings.stepDelayMs);
        bool benchDue = plotter.graphExists() && !dragging && viewKey() != benchDoneKey &&
                        glutGet(GLUT_ELAPSED_TIME) - lastViewChangeMs >= 250;
        return changed || benchDue;
    }

    int cursorFor(int mx, int my) const override {
        if (showHow) return GLUT_CURSOR_LEFT_ARROW;
        if (dragging && dragMoved) return GLUT_CURSOR_CYCLE;
        if (inputR.contains((float)mx, (float)my)) return GLUT_CURSOR_TEXT;
        if (plotR.contains((float)mx, (float)my)) return GLUT_CURSOR_CROSSHAIR;
        return GLUT_CURSOR_LEFT_ARROW;
    }
};

#endif // GLIDE_GRAPH_SCREEN_H
