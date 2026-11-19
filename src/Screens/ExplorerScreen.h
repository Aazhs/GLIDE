/*
 * ExplorerScreen.h
 * ----------------
 * Common base for the Line Explorer and the Circle Explorer.
 *
 * Layout:
 *   +-------------------------------------------------------------+
 *   | header                                                      |
 *   | toolbar (algorithm choice + options)                        |
 *   +------------------------------------------+------------------+
 *   |                                          |  side panel:     |
 *   |   plot (pixel grid)                      |  setup values,   |
 *   |                                          |  current step,   |
 *   |                                          |  iteration table |
 *   +------------------------------------------+------------------+
 *   | playback bar                                                |
 *   +-------------------------------------------------------------+
 *
 * Mouse in the plot:
 *   - drag a handle (endpoint / centre / radius)  -> move it, live update
 *   - click on empty space                        -> screen-specific action
 *   - drag on empty space (or right-drag)         -> pan
 *   - scroll wheel (where supported)              -> zoom at cursor
 *
 * The plot can be split into several side-by-side "sub-views" that share
 * one pan/zoom (used by the line explorer's Compare mode).
 */

#ifndef GLIDE_EXPLORER_SCREEN_H
#define GLIDE_EXPLORER_SCREEN_H

#include "Screen.h"
#include "../Algorithms/DrawingAlgorithm.h"
#include <algorithm>
#include <cstdlib>
#include <map>

/* Widget ids shared by both explorers (120 - 199, table rows 1000+) */
enum ExplorerIds {
    ID_EX_ZOOM_IN = 120,
    ID_EX_ZOOM_OUT,
    ID_EX_FIT,
    ID_EX_GRID,
    ID_EX_IDEAL,
    ID_EX_QUIZ,
    ID_EX_RACE_COST,
    ID_EX_RACE_STEPS,
    ID_QUIZ_OPT0 = 130,      // 130, 131, 132
    ID_QUIZ_NEXT = 135,
    ID_QUIZ_CLOSE,
    ID_TABLE_ROW = 1000      // 1000 + row index
};

/* "Predict the next pixel" question */
struct Quiz {
    bool active;
    bool answered;
    int correct;             // index of the correct option
    int selected;            // option picked by the user (-1 = none)
    int stepIndex;           // step whose pixel is being asked about
    std::string question;
    std::string options[3];
    std::string explanation; // shown after answering
    int score, attempts;

    Quiz() : active(false), answered(false), correct(0), selected(-1),
             stepIndex(0), score(0), attempts(0) {}
};

class ExplorerScreen : public Screen {
protected:
    enum DragMode { DRAG_NONE, DRAG_PENDING, DRAG_PAN, DRAG_HANDLE };

    CoordinateSystem view;      // pan/zoom, plot area = first sub-view
    StepAnimator anim;
    Quiz quiz;

    Rect toolbarR, plotR, panelR, navR;

    DragMode drag;
    int dragHandle;
    int pressX, pressY, lastX, lastY;

    bool mouseInPlot;
    int hoverSubView;
    float hoverWX, hoverWY;     // world coordinates under the mouse
    bool needFit;               // fit the view after the next layout pass

    /* ---------------------------------------------------------------
     * COMPARISON / RACE
     * In compare mode several algorithms run side by side. With
     * "race by cost" each one advances according to the arithmetic it
     * performs (OpCounts::cost), so cheaper algorithms visibly finish
     * first. With "equal steps" they advance one iteration per tick.
     * --------------------------------------------------------------- */
    struct Entry {
        const DrawingAlgorithm* algo;
        Color color;
    };
    bool raceByCost;
    std::vector<std::vector<float>> racePrefix;   // cumulative cost after each step
    float raceUnit;                               // cost units per animation tick

    /* Measured timings, cached until the parameters change */
    std::map<const DrawingAlgorithm*, double> benchNs;
    std::string benchDoneKey;

    /* ---------------------------------------------------------------
     * HOOKS FOR SUBCLASSES
     * --------------------------------------------------------------- */
    virtual std::string title() const = 0;
    virtual std::string subtitle() const = 0;
    virtual void drawToolbar(UI& ui) = 0;
    virtual void drawSubView(UI& ui, const CoordinateSystem& cs, int index) = 0;
    virtual void drawPanel(UI& ui) = 0;
    virtual void fitView() = 0;
    virtual std::string plotHint() const = 0;

    virtual int subViewCount() const { return 1; }
    virtual void onPlotClick(int /*cellX*/, int /*cellY*/) {}
    virtual int handleAt(const CoordinateSystem& /*cs*/, int /*mx*/, int /*my*/) const { return -1; }
    virtual void onHandleDrag(int /*handle*/, float /*wx*/, float /*wy*/) {}
    virtual void onHandleRelease(int /*handle*/) {}
    virtual void onHoverChanged() {}
    virtual bool buildQuiz() { return false; }
    virtual bool quizSupported() const { return false; }

    /* All algorithms this explorer offers (for benchmarks) */
    virtual std::vector<Entry> allAlgorithms() const = 0;
    /* Algorithms shown side by side; empty when not in compare mode */
    virtual std::vector<Entry> comparedAlgorithms() const { return {}; }
    /* Changes whenever the shape parameters change (invalidates benchmarks) */
    virtual std::string paramsKey() const = 0;

    bool inCompare() const { return !comparedAlgorithms().empty(); }

    /* Recompute race timing; call after the algorithms were re-run */
    void rebuildRace() {
        std::vector<Entry> algos = comparedAlgorithms();
        racePrefix.clear();
        float maxCost = 0;
        int maxSteps = 0;
        for (const auto& e : algos) {
            std::vector<float> prefix;
            float c = e.algo->getSetupOps().cost();
            for (const auto& st : e.algo->getSteps()) {
                c += st.ops.cost();
                prefix.push_back(c);
            }
            maxCost = std::max(maxCost, c);
            maxSteps = std::max(maxSteps, e.algo->getStepCount());
            racePrefix.push_back(prefix);
        }
        /* The most expensive algorithm takes one tick per iteration */
        raceUnit = maxSteps > 0 ? maxCost / maxSteps : 1.0f;
    }

    /* Animation length for the current mode */
    int compareTotal() const {
        int maxSteps = 0;
        for (const auto& p : racePrefix) maxSteps = std::max(maxSteps, (int)p.size());
        return maxSteps;
    }

    /* How many steps algorithm k has completed at the current tick */
    int revealedFor(int k) const {
        if (k < 0 || k >= (int)racePrefix.size()) return anim.getRevealed();
        const auto& p = racePrefix[k];
        int t = anim.getRevealed();
        if (t >= anim.getTotal()) return (int)p.size();
        if (!raceByCost) return std::min(t, (int)p.size());
        float budget = t * raceUnit + 1e-3f;
        return (int)(std::upper_bound(p.begin(), p.end(), budget) - p.begin());
    }

    /* Tick at which algorithm k finishes */
    int finishTick(int k) const {
        if (k < 0 || k >= (int)racePrefix.size() || racePrefix[k].empty()) return 0;
        if (!raceByCost) return (int)racePrefix[k].size();
        return std::max(1, (int)ceilf(racePrefix[k].back() / raceUnit - 1e-3f));
    }

    void runBenchmarksIfNeeded() {
        std::string key = paramsKey();
        if (key == benchDoneKey || drag != DRAG_NONE) return;
        benchNs.clear();
        for (const auto& e : allAlgorithms()) benchNs[e.algo] = e.algo->benchmarkNs();
        benchDoneKey = key;
    }

    double measured(const DrawingAlgorithm* a) const {
        auto it = benchNs.find(a);
        return it == benchNs.end() ? -1.0 : it->second;
    }

    static std::string formatNs(double ns) {
        if (ns < 0) return "...";
        if (ns < 1000) return Format::num((float)ns, ns < 10 ? 1 : 0) + " ns";
        return Format::num((float)(ns / 1000.0), 2) + " us";
    }

    /* ---------------------------------------------------------------
     * LAYOUT
     * --------------------------------------------------------------- */
    void computeLayout(int w, int h) {
        using namespace Layout;
        float panelW = MathUtils::clamp(w * 0.32f, 300.0f, 460.0f);
        toolbarR = Rect(0, HEADER_H, (float)w, TOOLBAR_H);
        navR = Rect(0, h - NAV_H, (float)w, NAV_H);
        float top = HEADER_H + TOOLBAR_H + GAP;
        float plotH = std::max(120.0f, h - NAV_H - GAP - top);
        plotR = Rect(GAP, top, std::max(200.0f, w - panelW - 3 * GAP), plotH);
        panelR = Rect(plotR.right() + GAP, top, w - plotR.right() - 2 * GAP, plotH);

        CoordinateSystem first = subView(0);
        view.setPlotArea(first.plotAreaX, first.plotAreaY, first.plotAreaW, first.plotAreaH);
        view.screenWidth = w;
        view.screenHeight = h;
    }

    /* Geometry of sub-view i (shares pan/zoom with 'view') */
    CoordinateSystem subView(int i) const {
        int n = subViewCount();
        float gap = 6.0f;
        float sw = (plotR.w - gap * (n - 1)) / n;
        CoordinateSystem cs = view;
        cs.setPlotArea((int)(plotR.x + i * (sw + gap)), (int)plotR.y, (int)sw, (int)plotR.h);
        return cs;
    }

    int subViewAt(int mx, int my) const {
        for (int i = 0; i < subViewCount(); i++) {
            if (subView(i).containsScreen(mx, my)) return i;
        }
        return -1;
    }

    /* Convert a mouse position in sub-view 'i' into the matching position in sub-view 0 */
    float toFirstViewX(int i, float mx) const {
        return mx - (subView(i).plotAreaX - subView(0).plotAreaX);
    }

    /* ---------------------------------------------------------------
     * DRAWING HELPERS FOR SUBCLASSES
     * --------------------------------------------------------------- */

    /* One raster pixel, honouring the pixel-grid setting */
    void drawPixel(const CoordinateSystem& cs, float wx, float wy, const Color& c) const {
        setColor(c);
        if (settings.showPixelGrid && cs.pixelsPerUnitX() >= 8.0f) {
            Renderer::drawPixelCell(cs, wx, wy, 1.0f);
        } else {
            /* Compact dot when the grid is off or cells are tiny */
            float size = std::max(4.0f, std::min(cs.pixelsPerUnitX() * 0.45f, 14.0f));
            float sx = cs.worldToScreenX(wx), sy = cs.worldToScreenY(wy);
            Renderer::drawRect(sx - size / 2, sy - size / 2, size, size);
        }
    }

    /* Coordinate label above a pixel, only when there is room */
    void labelPixel(const CoordinateSystem& cs, float wx, float wy, const std::string& text,
                    const Color& c) const {
        if (cs.pixelsPerUnitX() < 22.0f) return;
        float sx = cs.worldToScreenX(wx), sy = cs.worldToScreenY(wy);
        int tw = Renderer::textWidth(text, Fonts::small());
        Renderer::drawLabel(sx - tw / 2.0f, sy - cs.pixelsPerUnitY() / 2.0f - 5, text, c);
    }

    /* Toolbar toggle buttons shared by both explorers; returns x after them */
    float drawCommonToggles(UI& ui, float x, float y) {
        ui.button(Rect(x, y, 92, 28), "Pixel grid", ID_EX_GRID, settings.showPixelGrid);
        x += 96;
        ui.button(Rect(x, y, 92, 28), "Ideal shape", ID_EX_IDEAL, settings.showIdeal);
        return x + 96;
    }

    /* Section heading in the side panel */
    float drawSection(float x, float y, float w, const std::string& text,
                      const std::string& right = "") const {
        setColor(Theme::TEXT_FAINT);
        Renderer::drawText(x, y, text, Fonts::small());
        if (!right.empty()) {
            setColor(Theme::TEXT_DIM);
            Renderer::drawText(x + w - Renderer::textWidth(right, Fonts::small()), y, right,
                               Fonts::small());
        }
        setColor(Theme::BORDER);
        Renderer::drawLine(x, y + 5.5f, x + w, y + 5.5f);
        return y + 20;
    }

    /*
     * Generic side panel for a single algorithm:
     *   name + description, setup values, current step, iteration table.
     */
    void drawAlgorithmPanel(UI& ui, const DrawingAlgorithm& algo, const Color& color) {
        float x = panelR.x + 14, w = panelR.w - 28;
        float y = panelR.y + 26;
        float maxY = panelR.bottom() - 8;

        setColor(color);
        Renderer::drawText(x, y, algo.getName(), Fonts::title());
        y += 20;
        setColor(Theme::TEXT_DIM);
        y = Renderer::drawWrapped(x, y, w, algo.getDescription(), Fonts::small()) + 8;

        /* Setup values */
        y = drawSection(x, y, w, "SETUP");
        setColor(Theme::TEXT);
        for (const auto& line : algo.getSetupLines()) {
            y = Renderer::drawWrapped(x, y, w, line, Fonts::mono(), maxY);
        }
        y += 8;

        /* Cost so far vs. whole run */
        const auto& steps = algo.getSteps();
        int cur = anim.currentIndex();
        {
            OpCounts sofar = algo.getSetupOps();
            for (int i = 0; i <= cur && i < (int)steps.size(); i++) sofar += steps[i].ops;
            OpCounts total = algo.getTotalOps();
            y = drawSection(x, y, w, "COST", "measured " + formatNs(measured(&algo)) + " per run");
            setColor(Theme::TEXT);
            if (cur >= 0 && cur < (int)steps.size()) {
                y = Renderer::drawWrapped(x, y, w, "this step: " + steps[cur].ops.summary() + " = " +
                                          Format::num(steps[cur].ops.cost()) + " units", Fonts::mono(), maxY);
            }
            setColor(Theme::TEXT_DIM);
            y = Renderer::drawWrapped(x, y, w, "so far " + Format::num(sofar.cost()) + " of " +
                                      Format::num(total.cost()) + " units  (" +
                                      std::to_string(total.floatOps() + total.rounds) + " float, " +
                                      std::to_string(total.integerOps()) + " int ops)",
                                      Fonts::mono(), maxY);
            y += 8;
        }

        /* Current step */
        y = drawSection(x, y, w, "CURRENT STEP",
                        cur >= 0 ? std::to_string(cur + 1) + " of " + std::to_string(steps.size()) : "");
        if (cur >= 0 && cur < (int)steps.size()) {
            auto lines = Renderer::wrapText(steps[cur].explanation, Fonts::mono(), w - 12);
            float boxH = lines.size() * Renderer::lineHeight(Fonts::mono()) + 10.0f;
            setColor(Theme::PANEL_ALT);
            Renderer::drawRect(x - 4, y - 13, w + 8, boxH);
            setColor(Theme::YELLOW);
            Renderer::drawRect(x - 4, y - 13, 3, boxH);
            setColor(Theme::TEXT);
            for (const auto& line : lines) {
                Renderer::drawText(x + 4, y, line, Fonts::mono());
                y += Renderer::lineHeight(Fonts::mono());
            }
            y += 6;
        } else {
            setColor(Theme::TEXT_DIM);
            y = Renderer::drawWrapped(x, y, w,
                "Press Play (Space) to animate, or Next (N) to go one step at a time.",
                Fonts::body()) + 2;
        }
        y += 8;

        /* Iteration table */
        std::vector<std::vector<std::string>> rows;
        rows.reserve(steps.size());
        for (const auto& s : steps) {
            rows.push_back(s.tableCells);
            rows.back().push_back(Format::num(s.ops.cost()));
        }
        std::vector<std::string> cols = algo.getTableColumns();
        cols.push_back("cost");
        drawTable(ui, Rect(x, y, w, maxY - y), cols, rows, nullptr);
    }

    /*
     * Iteration table. Only rows that have been revealed are shown and the
     * view auto-scrolls so the current row is visible. Rows are clickable.
     * 'rowFlag' (optional) marks rows to draw in red (e.g. differing pixels).
     */
    void drawTable(UI& ui, const Rect& area, const std::vector<std::string>& cols,
                   const std::vector<std::vector<std::string>>& rows,
                   const std::vector<bool>* rowFlag, int revealedRows = -1, int currentRow = -2) {
        if (area.h < 40 || cols.empty()) return;
        float y = drawSection(area.x, area.y, area.w, "ITERATION TABLE",
                              "click a row to jump");

        /* Column widths from the longest entry (monospace font) */
        const int charW = 8;
        std::vector<float> colW(cols.size());
        for (size_t c = 0; c < cols.size(); c++) {
            size_t len = cols[c].size();
            for (const auto& r : rows) if (c < r.size()) len = std::max(len, r[c].size());
            colW[c] = (float)(len * charW + 14);
        }
        /* Spread spare width evenly so the table fills the panel */
        float sum = 0;
        for (float cw : colW) sum += cw;
        if (sum < area.w) for (auto& cw : colW) cw += (area.w - sum) / colW.size();

        const float rowH = 16.0f;
        auto drawRow = [&](float ry, const std::vector<std::string>& cells, const Color& c) {
            float cx = area.x + 4;
            setColor(c);
            for (size_t k = 0; k < cols.size() && k < cells.size(); k++) {
                Renderer::drawText(cx, ry, cells[k], Fonts::mono());
                cx += colW[k];
            }
        };

        /* Header row */
        setColor(Theme::PANEL_ALT);
        Renderer::drawRect(area.x, y - 12, area.w, rowH);
        drawRow(y, cols, Theme::TEXT_DIM);
        y += rowH + 2;

        int revealed = std::min(revealedRows >= 0 ? revealedRows : anim.getRevealed(), (int)rows.size());
        int cur = currentRow != -2 ? currentRow : anim.currentIndex();
        int visible = std::max(1, (int)((area.bottom() - y + 10) / rowH));
        int first = std::max(0, revealed - visible);

        if (revealed == 0) {
            setColor(Theme::TEXT_FAINT);
            Renderer::drawText(area.x + 4, y, "(rows appear as the algorithm runs)", Fonts::small());
            return;
        }

        for (int i = first; i < revealed && i < first + visible; i++) {
            Rect rowRect(area.x, y - 12, area.w, rowH);
            bool hover = ui.region(rowRect, ID_TABLE_ROW + i);
            bool flagged = rowFlag && i < (int)rowFlag->size() && (*rowFlag)[i];
            if (i == cur) {
                setColor(Theme::YELLOW.withAlpha(0.18f));
                Renderer::drawRect(rowRect);
                setColor(Theme::YELLOW);
                Renderer::drawRect(rowRect.x, rowRect.y, 3, rowRect.h);
            } else if (hover) {
                setColor(Theme::BTN_HOVER.withAlpha(0.6f));
                Renderer::drawRect(rowRect);
            }
            Color tc = flagged ? Theme::RED : (i == cur ? Theme::YELLOW : Theme::TEXT);
            drawRow(y, rows[i], tc);
            y += rowH;
        }
    }

    /*
     * Side panel for compare mode: race progress, cost and timing charts,
     * and a scoreboard of accuracy metrics. Returns the y below it.
     */
    float drawComparePanel(UI& ui, const std::string& heading, const std::string& blurb) {
        std::vector<Entry> algos = comparedAlgorithms();
        int n = (int)algos.size();
        float x = panelR.x + 14, w = panelR.w - 28;
        float y = panelR.y + 26;

        setColor(Theme::TEXT);
        Renderer::drawText(x, y, heading, Fonts::title());
        y += 20;
        setColor(Theme::TEXT_DIM);
        y = Renderer::drawWrapped(x, y, w, blurb, Fonts::small()) + 6;

        /* Race mode toggle */
        ui.button(Rect(x, y, w / 2 - 3, 24), "Race by CPU cost", ID_EX_RACE_COST, raceByCost,
                  true, Fonts::small());
        ui.button(Rect(x + w / 2 + 3, y, w / 2 - 3, 24), "Equal steps", ID_EX_RACE_STEPS, !raceByCost,
                  true, Fonts::small());
        y += 42;

        /* Race progress */
        y = drawSection(x, y, w, "RACE", raceByCost ? "each tick = same CPU budget" : "one iteration per tick");
        for (int k = 0; k < n; k++) {
            int done = revealedFor(k), total = algos[k].algo->getStepCount();
            bool finished = done >= total;
            std::string v = finished ? "done @" + std::to_string(finishTick(k))
                                     : std::to_string(done) + "/" + std::to_string(total);
            drawBar(x, y, w, total ? (float)done / total : 0, algos[k].color,
                    shortName(algos[k].algo), v, false);
            y += 17;
        }
        y += 6;

        /* Model cost chart */
        float maxCost = 0, minCost = 1e30f;
        double maxNs = 0, minNs = 1e30;
        for (const auto& e : algos) {
            float c = e.algo->getTotalOps().cost();
            maxCost = std::max(maxCost, c); minCost = std::min(minCost, c);
            double t = measured(e.algo);
            if (t > 0) { maxNs = std::max(maxNs, t); minNs = std::min(minNs, t); }
        }
        y = drawSection(x, y, w, "MODEL COST", "weighted ops, lower is better");
        for (const auto& e : algos) {
            float c = e.algo->getTotalOps().cost();
            drawBar(x, y, w, maxCost > 0 ? c / maxCost : 0, e.color, shortName(e.algo),
                    Format::num(c, 0), c <= minCost + 0.5f);
            y += 17;
        }
        y += 6;
        y = drawSection(x, y, w, "MEASURED TIME", "this CPU, best of many runs");
        for (const auto& e : algos) {
            double t = measured(e.algo);
            drawBar(x, y, w, maxNs > 0 && t > 0 ? (float)(t / maxNs) : 0, e.color, shortName(e.algo),
                    formatNs(t), t > 0 && t <= minNs * 1.05);
            y += 17;
        }
        y += 6;

        /* Scoreboard */
        y = drawSection(x, y, w, "SCOREBOARD", "* = best");
        float colW = (w - 112) / std::max(n, 1);
        for (int k = 0; k < n; k++) {
            setColor(algos[k].color);
            Renderer::drawText(x + 112 + k * colW, y, shortName(algos[k].algo), Fonts::small());
        }
        y += 15;

        /* 'ranked' rows get a star for the lowest value; others are just facts */
        struct Row { std::string name; std::vector<float> v; int decimals; bool ranked; };
        std::vector<Row> rows;
        auto addRow = [&](const std::string& name, int dec, auto getter, bool ranked = true) {
            Row r{ name, {}, dec, ranked };
            for (const auto& e : algos) r.v.push_back(getter(e.algo));
            rows.push_back(r);
        };
        addRow("iterations", 0, [](const DrawingAlgorithm* a) { return (float)a->getStepCount(); }, false);
        addRow("float ops", 0, [](const DrawingAlgorithm* a) {
            OpCounts o = a->getTotalOps(); return (float)(o.floatOps() + o.rounds); });
        addRow("integer ops", 0, [](const DrawingAlgorithm* a) { return (float)a->getTotalOps().integerOps(); },
               false);
        addRow("cost / pixel", 1, [](const DrawingAlgorithm* a) {
            return a->getAllPoints().empty() ? 0.0f : a->getTotalOps().cost() / a->getAllPoints().size(); });
        addRow("max error px", 3, [](const DrawingAlgorithm* a) { return a->getQuality().maxError; });
        addRow("avg error px", 3, [](const DrawingAlgorithm* a) { return a->getQuality().avgError; });
        addRow("duplicate px", 0, [](const DrawingAlgorithm* a) { return (float)a->getQuality().duplicates; });
        bool anyDrift = false;
        for (const auto& e : algos) anyDrift |= e.algo->getQuality().hasDrift;
        if (anyDrift) {
            addRow("radius drift", 3, [](const DrawingAlgorithm* a) { return fabsf(a->getQuality().drift); });
        }

        for (size_t r = 0; r < rows.size(); r++) {
            const Row& row = rows[r];
            /* Lower is better for every ranked metric */
            float best = *std::min_element(row.v.begin(), row.v.end());
            float worst = *std::max_element(row.v.begin(), row.v.end());
            setColor(Theme::TEXT_DIM);
            Renderer::drawText(x, y, row.name, Fonts::small());
            for (int k = 0; k < n; k++) {
                bool isBest = row.ranked && worst - best > 1e-4f && row.v[k] <= best + 1e-4f;
                setColor(isBest ? Theme::GREEN : Theme::TEXT);
                Renderer::drawText(x + 112 + k * colW, y,
                                   Format::num(row.v[k], row.decimals) + (isBest ? " *" : ""), Fonts::mono());
            }
            y += 15;
        }
        y += 4;

        /* Explain when the model and the real measurement disagree */
        int modelBest = 0, measBest = 0;
        for (int k = 1; k < n; k++) {
            if (algos[k].algo->getTotalOps().cost() < algos[modelBest].algo->getTotalOps().cost()) modelBest = k;
            if (measured(algos[k].algo) > 0 && measured(algos[k].algo) < measured(algos[measBest].algo)) measBest = k;
        }
        setColor(Theme::TEXT_FAINT);
        std::string note = "Cost model: int op 1, float add 3, mul 4, div 12, round 6.";
        if (measured(algos[0].algo) > 0 && modelBest != measBest &&
            measured(algos[modelBest].algo) > measured(algos[measBest].algo) * 1.1) {
            note += " On this CPU the measured winner differs: modern processors have fast "
                    "floating-point units and branch-free loops pipeline well, while the "
                    "if (d >= 0) branch is hard to predict. Integer-only algorithms win on "
                    "hardware without a fast FPU (the reason they were invented).";
        }
        y = Renderer::drawWrapped(x, y, w, note, Fonts::small(), panelR.bottom() - 8) + 6;
        return y;
    }

    static std::string shortName(const DrawingAlgorithm* a) {
        std::string n = a->getName();
        size_t sp = n.find(' ');
        return sp == std::string::npos ? n : n.substr(0, sp);
    }

    /* Enter compare mode: rebuild timing and start the race */
    void startRace() {
        rebuildRace();
        anim.reset(compareTotal(), true);
    }

    /* ---------------------------------------------------------------
     * QUIZ
     * --------------------------------------------------------------- */

    /* Fill quiz options: shuffle 'candidates' (which must contain 'answer') */
    void setQuizOptions(const std::vector<std::string>& candidates, const std::string& answer) {
        std::vector<std::string> opts;
        for (const auto& c : candidates) {
            if (std::find(opts.begin(), opts.end(), c) == opts.end()) opts.push_back(c);
        }
        if (std::find(opts.begin(), opts.end(), answer) == opts.end()) opts.insert(opts.begin(), answer);
        while (opts.size() > 3) {
            /* Drop a wrong option, never the answer */
            for (size_t i = opts.size(); i-- > 0;) {
                if (opts[i] != answer) { opts.erase(opts.begin() + i); break; }
            }
        }
        while (opts.size() < 3) opts.push_back("(none of these)");
        for (int i = 2; i > 0; i--) std::swap(opts[i], opts[rand() % (i + 1)]);
        for (int i = 0; i < 3; i++) {
            quiz.options[i] = opts[i];
            if (opts[i] == answer) quiz.correct = i;
        }
        quiz.answered = false;
        quiz.selected = -1;
        quiz.active = true;
    }

    void openQuiz() {
        if (!quizSupported()) return;
        anim.pause();
        if (!buildQuiz()) {
            /* At the end: restart from the first step and try again */
            anim.jumpTo(0);
            buildQuiz();
        }
    }

    void answerQuiz(int option) {
        if (!quiz.active || quiz.answered) return;
        quiz.selected = option;
        quiz.answered = true;
        quiz.attempts++;
        if (option == quiz.correct) quiz.score++;
        anim.jumpTo(quiz.stepIndex);   // reveal the pixel that was asked about
    }

    void drawQuiz(UI& ui) {
        ui.beginModal();
        float pw = 520, ph = 470;
        Rect box((ui.width() - pw) / 2, (ui.height() - ph) / 2, pw, ph);
        setColor(Theme::PANEL);
        Renderer::drawRect(box);
        setColor(Theme::ACCENT);
        Renderer::drawRectOutline(box, 1.5f);

        float x = box.x + 22, y = box.y + 34, w = pw - 44;
        setColor(Theme::ACCENT);
        Renderer::drawText(x, y, "Predict the next pixel", Fonts::title());
        std::string score = "Score " + std::to_string(quiz.score) + " / " + std::to_string(quiz.attempts);
        setColor(Theme::TEXT_DIM);
        Renderer::drawText(box.right() - 22 - Renderer::textWidth(score, Fonts::small()), y, score,
                           Fonts::small());
        y += 26;
        setColor(Theme::TEXT);
        y = Renderer::drawWrapped(x, y, w, quiz.question, Fonts::mono()) + 8;

        for (int i = 0; i < 3; i++) {
            Rect r(x, y, w, 34);
            std::string label = std::string(1, (char)('A' + i)) + ")   " + quiz.options[i];
            bool isAnswer = quiz.answered && i == quiz.correct;
            ui.button(r, label, ID_QUIZ_OPT0 + i, isAnswer, !quiz.answered || isAnswer ||
                      i == quiz.selected, Fonts::body(), isAnswer ? &Theme::GREEN : nullptr);
            if (quiz.answered && i == quiz.selected && i != quiz.correct) {
                setColor(Theme::RED);
                Renderer::drawRectOutline(r, 2.0f);
            }
            y += 40;
        }

        if (quiz.answered) {
            bool right = quiz.selected == quiz.correct;
            setColor(right ? Theme::GREEN : Theme::RED);
            Renderer::drawText(x, y + 12, right ? "Correct!" : "Not quite - the green answer is right.",
                               Fonts::body());
            setColor(Theme::TEXT_DIM);
            Renderer::drawWrapped(x, y + 30, w, quiz.explanation, Fonts::small(), box.bottom() - 50);
        } else {
            setColor(Theme::TEXT_FAINT);
            Renderer::drawText(x, y + 12, "Click an answer or press A / B / C.", Fonts::small());
        }

        float by = box.bottom() - 44;
        if (quiz.answered) {
            ui.button(Rect(box.right() - 22 - 84 - 8 - 130, by, 130, 30), "Next question",
                      ID_QUIZ_NEXT, true);
        }
        ui.button(Rect(box.right() - 22 - 84, by, 84, 30), "Close", ID_QUIZ_CLOSE);
    }

public:
    explicit ExplorerScreen(Settings& s)
        : Screen(s), drag(DRAG_NONE), dragHandle(-1), pressX(0), pressY(0),
          lastX(0), lastY(0), mouseInPlot(false), hoverSubView(-1),
          hoverWX(0), hoverWY(0), needFit(true), raceByCost(true), raceUnit(1.0f) {
        view.scaleX = 30.0f;
        view.scaleY = 30.0f;
        view.zoom = 1.0f;
    }

    bool hasModal() const override { return quiz.active; }
    void closeModal() override { quiz.active = false; }

    /* ---------------------------------------------------------------
     * RENDER
     * --------------------------------------------------------------- */
    void render(UI& ui) override {
        computeLayout(ui.width(), ui.height());
        if (needFit) {
            needFit = false;
            fitView();
        }

        runBenchmarksIfNeeded();
        drawHeader(ui, title(), subtitle());

        drawBand(toolbarR);
        drawToolbar(ui);

        /* Plot sub-views */
        for (int i = 0; i < subViewCount(); i++) {
            CoordinateSystem cs = subView(i);
            Rect r((float)cs.plotAreaX, (float)cs.plotAreaY, (float)cs.plotAreaW, (float)cs.plotAreaH);
            setColor(Theme::PLOT_BG);
            Renderer::drawRect(r);
            Renderer::beginClip(r, ui.height());
            Renderer::drawGrid(cs, settings.showPixelGrid);
            drawSubView(ui, cs, i);
            Renderer::endClip();
            setColor(Theme::BORDER);
            Renderer::drawRectOutline(r);
        }

        /* Plot overlays: hint, cursor coordinates, zoom buttons */
        setColor(Theme::TEXT_DIM);
        Renderer::drawLabel(plotR.x + 10, plotR.bottom() - 24, plotHint(), Theme::TEXT_DIM);
        if (mouseInPlot && !ui.isModalActive()) {
            std::string c = "cursor " + Format::pt(MathUtils::roundToInt(hoverWX),
                                                   MathUtils::roundToInt(hoverWY));
            Renderer::drawLabel(plotR.right() - 12 - Renderer::textWidth(c, Fonts::small()),
                                plotR.bottom() - 24, c, Theme::TEXT_DIM);
        }
        float zx = plotR.right() - 10 - 30;
        ui.button(Rect(zx, plotR.y + 10, 30, 28), "+", ID_EX_ZOOM_IN, false, true, Fonts::title());
        ui.button(Rect(zx, plotR.y + 42, 30, 28), "-", ID_EX_ZOOM_OUT, false, true, Fonts::title());
        ui.button(Rect(zx - 4, plotR.y + 74, 38, 24), "Fit", ID_EX_FIT, false, true, Fonts::small());

        /* Side panel */
        setColor(Theme::PANEL);
        Renderer::drawRect(panelR);
        Renderer::beginClip(panelR, ui.height());
        drawPanel(ui);
        Renderer::endClip();

        /* Playback bar */
        drawPlaybackBar(ui, navR, anim, quizSupported() ? 110.0f : 0.0f,
                        inCompare() && raceByCost ? "Tick" : "Step");
        if (quizSupported()) {
            ui.button(Rect(navR.right() - 12 - 98, navR.y + 11, 98, 32), "Quiz (Q)", ID_EX_QUIZ);
        }

        if (quiz.active) drawQuiz(ui);
    }

    /* ---------------------------------------------------------------
     * INPUT
     * --------------------------------------------------------------- */
    void onWidget(int id) override {
        if (handlePlaybackWidget(id, anim)) return;
        if (id >= ID_TABLE_ROW) { anim.jumpTo(id - ID_TABLE_ROW); return; }
        switch (id) {
            case ID_EX_ZOOM_IN:  view.zoomIn();  break;
            case ID_EX_ZOOM_OUT: view.zoomOut(); break;
            case ID_EX_FIT:      fitView();      break;
            case ID_EX_GRID:     settings.showPixelGrid = !settings.showPixelGrid; break;
            case ID_EX_IDEAL:    settings.showIdeal = !settings.showIdeal; break;
            case ID_EX_QUIZ:     openQuiz(); break;
            case ID_QUIZ_OPT0: case ID_QUIZ_OPT0 + 1: case ID_QUIZ_OPT0 + 2:
                answerQuiz(id - ID_QUIZ_OPT0);
                break;
            case ID_QUIZ_NEXT:
                if (!buildQuiz()) { anim.jumpTo(0); buildQuiz(); }
                break;
            case ID_QUIZ_CLOSE:  quiz.active = false; break;
            case ID_EX_RACE_COST:  raceByCost = true;  startRace(); break;
            case ID_EX_RACE_STEPS: raceByCost = false; startRace(); break;
        }
    }

    void onSlider(int id, float value) override {
        handlePlaybackSlider(id, value, anim);
    }

    void onMouseDown(int button, int mx, int my) override {
        int sv = subViewAt(mx, my);
        if (sv < 0) return;
        pressX = lastX = mx;
        pressY = lastY = my;

        if (button != GLUT_LEFT_BUTTON) {      // right / middle drag always pans
            drag = DRAG_PAN;
            return;
        }
        int h = handleAt(subView(sv), mx, my);
        if (h >= 0) {
            drag = DRAG_HANDLE;
            dragHandle = h;
            anim.pause();
        } else {
            drag = DRAG_PENDING;
        }
    }

    void onMouseDrag(int mx, int my) override {
        updateHover(mx, my);
        if (drag == DRAG_PENDING && (abs(mx - pressX) > 4 || abs(my - pressY) > 4)) {
            drag = DRAG_PAN;
        }
        if (drag == DRAG_PAN) {
            view.panPixels((float)(mx - lastX), (float)(my - lastY));
        } else if (drag == DRAG_HANDLE) {
            int sv = std::max(0, subViewAt(mx, my));
            CoordinateSystem cs = subView(sv);
            onHandleDrag(dragHandle, cs.screenToWorldX((float)mx), cs.screenToWorldY((float)my));
        }
        lastX = mx;
        lastY = my;
    }

    void onMouseUp(int /*button*/, int mx, int my) override {
        if (drag == DRAG_PENDING) {
            int sv = subViewAt(mx, my);
            if (sv >= 0) {
                CoordinateSystem cs = subView(sv);
                onPlotClick(MathUtils::roundToInt(cs.screenToWorldX((float)mx)),
                            MathUtils::roundToInt(cs.screenToWorldY((float)my)));
            }
        } else if (drag == DRAG_HANDLE) {
            onHandleRelease(dragHandle);
        }
        drag = DRAG_NONE;
        dragHandle = -1;
    }

    void onMouseMove(int mx, int my) override { updateHover(mx, my); }

    void updateHover(int mx, int my) {
        int sv = subViewAt(mx, my);
        bool inPlot = sv >= 0;
        float wx = hoverWX, wy = hoverWY;
        if (inPlot) {
            CoordinateSystem cs = subView(sv);
            wx = cs.screenToWorldX((float)mx);
            wy = cs.screenToWorldY((float)my);
        }
        bool cellChanged = MathUtils::roundToInt(wx) != MathUtils::roundToInt(hoverWX) ||
                           MathUtils::roundToInt(wy) != MathUtils::roundToInt(hoverWY);
        mouseInPlot = inPlot;
        hoverSubView = sv;
        hoverWX = wx;
        hoverWY = wy;
        if (cellChanged) onHoverChanged();
    }

    void onScroll(int direction, int mx, int my) override {
        int sv = subViewAt(mx, my);
        if (sv < 0) return;
        view.zoomAt(direction > 0 ? 1.15f : 1.0f / 1.15f, toFirstViewX(sv, (float)mx), (float)my);
    }

    bool onKey(unsigned char key) override {
        if (quiz.active) {
            if (key >= 'a' && key <= 'c') { answerQuiz(key - 'a'); return true; }
            if (key >= 'A' && key <= 'C') { answerQuiz(key - 'A'); return true; }
            if (key == 13 || key == 10 || key == ' ') {
                if (quiz.answered) onWidget(ID_QUIZ_NEXT);
                return true;
            }
            return false;
        }
        if (handlePlaybackKey(key, anim)) return true;
        switch (key) {
            case '+': case '=': view.zoomIn();  return true;
            case '-': case '_': view.zoomOut(); return true;
            case 'r': case 'R': case 'f': case 'F': fitView(); return true;
            case 'g': case 'G': settings.showPixelGrid = !settings.showPixelGrid; return true;
            case 'i': case 'I': settings.showIdeal = !settings.showIdeal; return true;
            case 'q': case 'Q': openQuiz(); return true;
            case 13: case 10: anim.play(); return true;
        }
        return false;
    }

    bool onSpecialKey(int key) override {
        const float step = 60.0f;
        switch (key) {
            /* Arrow keys move the camera: Right shows more of the right side */
            case GLUT_KEY_LEFT:  view.panPixels(step, 0);  return true;
            case GLUT_KEY_RIGHT: view.panPixels(-step, 0); return true;
            case GLUT_KEY_UP:    view.panPixels(0, step);  return true;
            case GLUT_KEY_DOWN:  view.panPixels(0, -step); return true;
            case GLUT_KEY_HOME:  anim.toStart(); return true;
            case GLUT_KEY_END:   anim.toEnd();   return true;
            case GLUT_KEY_PAGE_UP:   anim.prev(); return true;
            case GLUT_KEY_PAGE_DOWN: anim.next(); return true;
        }
        return false;
    }

    bool tick(int dtMs) override {
        return anim.tick(dtMs, settings.stepDelayMs);
    }

    int cursorFor(int mx, int my) const override {
        if (quiz.active) return GLUT_CURSOR_LEFT_ARROW;
        if (drag == DRAG_PAN) return GLUT_CURSOR_CYCLE;
        if (drag == DRAG_HANDLE) return GLUT_CURSOR_INFO;
        int sv = subViewAt(mx, my);
        if (sv < 0) return GLUT_CURSOR_LEFT_ARROW;
        if (handleAt(subView(sv), mx, my) >= 0) return GLUT_CURSOR_INFO;
        return GLUT_CURSOR_CROSSHAIR;
    }
};

#endif // GLIDE_EXPLORER_SCREEN_H
