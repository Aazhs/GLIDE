/*
 * Screen.h
 * --------
 * Base class for every page of the application (menu, graph plotter,
 * line explorer, circle explorer, polygon explorer, help).
 *
 * Application owns one object per screen and forwards GLUT events to the
 * active one. A screen asks to switch pages by calling navigate().
 *
 * This file also contains UI pieces shared by several screens:
 * the page header and the playback bar (|< < Play > >| + timeline).
 */

#ifndef GLIDE_SCREEN_H
#define GLIDE_SCREEN_H

#include "../UI/Widgets.h"
#include "../UI/StepAnimator.h"
#include <string>

enum ScreenId {
    SCREEN_MENU,
    SCREEN_GRAPH,
    SCREEN_LINES,
    SCREEN_CIRCLES,
    SCREEN_HELP,
    SCREEN_POLYGONS,
    SCREEN_EXIT,
    SCREEN_COUNT
};

/* Shared layout constants */
namespace Layout {
    const float HEADER_H  = 48.0f;
    const float TOOLBAR_H = 44.0f;
    const float NAV_H     = 54.0f;
    const float GAP       = 8.0f;
}

/* Widget ids used by the playback bar (100 - 119) */
enum PlaybackIds {
    ID_PB_FIRST = 100,
    ID_PB_PREV,
    ID_PB_PLAY,
    ID_PB_NEXT,
    ID_PB_LAST,
    ID_PB_SCRUB,
    ID_PB_SPEED
};

class Screen {
protected:
    Settings& settings;
    int navRequest;

    void navigate(ScreenId id) { navRequest = (int)id; }

public:
    explicit Screen(Settings& s) : settings(s), navRequest(-1) {}
    virtual ~Screen() {}

    /* Lifecycle */
    virtual void onEnter() {}
    virtual void render(UI& ui) = 0;

    /* Popups */
    virtual bool hasModal() const { return false; }
    virtual void closeModal() {}

    /* Input (widgets are hit-tested by Application first) */
    virtual void onWidget(int /*id*/) {}
    virtual void onSlider(int /*id*/, float /*value*/) {}
    virtual void onMouseDown(int /*button*/, int /*mx*/, int /*my*/) {}
    virtual void onMouseDrag(int /*mx*/, int /*my*/) {}
    virtual void onMouseUp(int /*button*/, int /*mx*/, int /*my*/) {}
    virtual void onMouseMove(int /*mx*/, int /*my*/) {}
    virtual void onScroll(int /*direction*/, int /*mx*/, int /*my*/) {}
    virtual bool onKey(unsigned char /*key*/) { return false; }
    virtual bool onSpecialKey(int /*key*/) { return false; }

    /* Called ~60 times per second. Return true to request a redraw. */
    virtual bool tick(int /*dtMs*/) { return false; }

    /* Mouse cursor shape for a position (GLUT_CURSOR_*) */
    virtual int cursorFor(int /*mx*/, int /*my*/) const { return GLUT_CURSOR_LEFT_ARROW; }

    /* True while a text box has keyboard focus (disables shortcuts) */
    virtual bool wantsTextInput() const { return false; }

    int takeNavRequest() { int r = navRequest; navRequest = -1; return r; }

    /* ---------------------------------------------------------------
     * SHARED UI PIECES
     * --------------------------------------------------------------- */

    /* Top bar: GLIDE logo, page title, subtitle and a back button */
    static void drawHeader(UI& ui, const std::string& title, const std::string& subtitle) {
        float w = (float)ui.width();
        setColor(Theme::HEADER);
        Renderer::drawRect(0, 0, w, Layout::HEADER_H);
        setColor(Theme::BORDER);
        Renderer::drawLine(0, Layout::HEADER_H - 0.5f, w, Layout::HEADER_H - 0.5f);

        setColor(Theme::ACCENT);
        Renderer::drawText(16, 31, "GLIDE", Fonts::title());
        float x = 16 + Renderer::textWidth("GLIDE", Fonts::title()) + 14;
        setColor(Theme::BORDER);
        Renderer::drawLine(x - 7, 14, x - 7, 34);
        setColor(Theme::TEXT);
        Renderer::drawText(x, 29, title, Fonts::body());
        x += Renderer::textWidth(title, Fonts::body()) + 12;
        setColor(Theme::TEXT_DIM);
        Renderer::drawText(x, 29, subtitle, Fonts::small());

        ui.button(Rect(w - 100, 9, 88, 30), "< Menu", BTN_BACK);
    }

    /* Dark band behind a toolbar row */
    static void drawBand(const Rect& r) {
        setColor(Theme::PANEL);
        Renderer::drawRect(r);
        setColor(Theme::BORDER);
        Renderer::drawLine(r.x, r.bottom() - 0.5f, r.right(), r.bottom() - 0.5f);
    }

    /* Small dim caption such as "SPEED" */
    static void drawCaption(float x, float y, const std::string& text) {
        setColor(Theme::TEXT_FAINT);
        Renderer::drawText(x, y, text, Fonts::small());
    }

    /* Horizontal bar for the scoreboard charts */
    static void drawBar(float x, float y, float w, float frac, const Color& c, const std::string& label,
                 const std::string& value, bool best) {
        const float labelW = 92, valueW = 78;
        setColor(c);
        Renderer::drawText(x, y, label, Fonts::small());
        float bx = x + labelW, bw = std::max(20.0f, w - labelW - valueW);
        setColor(Theme::PANEL_ALT);
        Renderer::drawRect(bx, y - 9, bw, 11);
        setColor(c.withAlpha(0.85f));
        Renderer::drawRect(bx, y - 9, bw * MathUtils::clamp(frac, 0.0f, 1.0f), 11);
        setColor(best ? Theme::GREEN : Theme::TEXT);
        Renderer::drawText(bx + bw + 8, y, value + (best ? "  *" : ""), Fonts::small());
    }

    /*
     * Playback bar:  [|<] [< Prev] [Play] [Next >] [>|]  step k/N  ====timeline====  speed ===
     * 'extraRight' reserves room on the right for screen-specific buttons.
     */
    void drawPlaybackBar(UI& ui, const Rect& bar, const StepAnimator& anim, float extraRight,
                         const std::string& unit = "Step") {
        setColor(Theme::PANEL);
        Renderer::drawRect(bar);
        setColor(Theme::BORDER);
        Renderer::drawLine(bar.x, bar.y + 0.5f, bar.right(), bar.y + 0.5f);

        float y = bar.y + 11, h = 32;
        float x = bar.x + 12;
        bool hasSteps = anim.getTotal() > 0;

        ui.button(Rect(x, y, 34, h), "|<", ID_PB_FIRST, false, hasSteps); x += 38;
        ui.button(Rect(x, y, 68, h), "< Prev", ID_PB_PREV, false, hasSteps); x += 72;
        ui.button(Rect(x, y, 78, h), anim.isPlaying() ? "Pause" : (anim.isComplete() ? "Replay" : "Play"),
                  ID_PB_PLAY, anim.isPlaying(), hasSteps); x += 82;
        ui.button(Rect(x, y, 68, h), "Next >", ID_PB_NEXT, false, hasSteps); x += 72;
        ui.button(Rect(x, y, 34, h), ">|", ID_PB_LAST, false, hasSteps); x += 46;

        /* Step counter */
        std::string counter = unit + " " + std::to_string(anim.getRevealed()) + " / " +
                              std::to_string(anim.getTotal());
        setColor(anim.isComplete() ? Theme::GREEN : Theme::TEXT);
        Renderer::drawText(x, y + 20, counter, Fonts::body());
        x += std::max(96, Renderer::textWidth(counter, Fonts::body()) + 14);

        /* Speed slider (fixed width, right side) */
        float speedW = 110;
        float speedX = bar.right() - extraRight - speedW - 70;
        /* Timeline fills the gap between the counter and the speed control */
        float scrubW = speedX - 70 - x;
        if (scrubW > 60 && hasSteps) {
            ui.slider(Rect(x, y + 9, scrubW, 14), (float)anim.getRevealed(), 0.0f,
                      (float)anim.getTotal(), ID_PB_SCRUB, Theme::YELLOW);
        }

        drawCaption(speedX - 46, y + 20, "SPEED");
        ui.slider(Rect(speedX, y + 9, speedW, 14), settings.speedSliderValue(), 0.0f, 1.0f,
                  ID_PB_SPEED);
        setColor(Theme::TEXT_DIM);
        Renderer::drawText(speedX + speedW + 12, y + 20,
                           std::to_string(settings.stepDelayMs) + " ms", Fonts::small());
    }

    /* Returns true if the id belonged to the playback bar */
    bool handlePlaybackWidget(int id, StepAnimator& anim) {
        switch (id) {
            case ID_PB_FIRST: anim.toStart();    return true;
            case ID_PB_PREV:  anim.prev();       return true;
            case ID_PB_PLAY:  anim.togglePlay(); return true;
            case ID_PB_NEXT:  anim.next();       return true;
            case ID_PB_LAST:  anim.toEnd();      return true;
        }
        return false;
    }

    bool handlePlaybackSlider(int id, float value, StepAnimator& anim) {
        if (id == ID_PB_SCRUB) {
            anim.jumpTo((int)(value + 0.5f) - 1);
            return true;
        }
        if (id == ID_PB_SPEED) {
            settings.setSpeedFromSlider(value);
            return true;
        }
        return false;
    }

    /* Keyboard shortcuts shared by every screen with a playback bar */
    bool handlePlaybackKey(unsigned char key, StepAnimator& anim) {
        switch (key) {
            case ' ':           anim.togglePlay(); return true;
            case 'n': case 'N':
            case '.': case '>': anim.next();       return true;
            case 'p': case 'P':
            case ',': case '<': anim.prev();       return true;
            case 's': case 'S': anim.toStart();    return true;
            case 'e': case 'E': anim.toEnd();      return true;
        }
        return false;
    }
};

#endif // GLIDE_SCREEN_H
