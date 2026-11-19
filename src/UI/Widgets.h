/*
 * Widgets.h
 * ---------
 * Buttons and sliders.
 *
 * HOW THE UI WORKS
 * Every frame each screen re-draws its widgets through the UI object.
 * Drawing a widget also REGISTERS its rectangle and id. When the mouse is
 * clicked, Application asks UI::hitTest() which registered widget is
 * under the cursor and forwards that id to the screen.
 *
 * Hover highlighting is decided while drawing, from the stored mouse
 * position, so it is always in sync with what is on screen.
 *
 * MODALS (popups): after beginModal() is called only widgets drawn from
 * that point on can be hovered or clicked - the rest of the screen is
 * blocked behind the dark overlay.
 */

#ifndef GLIDE_WIDGETS_H
#define GLIDE_WIDGETS_H

#include "Renderer.h"
#include <string>
#include <vector>

/* Reserved widget id shared by every screen */
const int BTN_BACK = 1;

class UI {
public:
    enum WidgetKind { KIND_BUTTON, KIND_SLIDER };

    struct Widget {
        Rect rect;
        int id;
        WidgetKind kind;
        bool enabled;
        bool modal;
        float minVal, maxVal;   // sliders only
    };

private:
    std::vector<Widget> widgets;
    int mouseX, mouseY;
    bool modalPending;      // a modal will be drawn this frame
    bool inModal;           // currently drawing the modal
    int activeSlider;       // slider being dragged (-1 = none)
    int windowW, windowH;

    bool canHover(const Rect& r) const {
        if (modalPending && !inModal) return false;
        if (activeSlider >= 0) return false;
        return r.contains((float)mouseX, (float)mouseY);
    }

public:
    UI() : mouseX(-1000), mouseY(-1000), modalPending(false), inModal(false),
           activeSlider(-1), windowW(1200), windowH(800) {}

    /* Called at the start of every frame */
    void beginFrame(int w, int h, bool modalWillOpen) {
        widgets.clear();
        windowW = w;
        windowH = h;
        modalPending = modalWillOpen;
        inModal = false;
    }

    int width() const  { return windowW; }
    int height() const { return windowH; }

    void setMouse(int x, int y) { mouseX = x; mouseY = y; }
    int getMouseX() const { return mouseX; }
    int getMouseY() const { return mouseY; }

    /* Dim the whole window and start drawing a popup */
    void beginModal() {
        setColor(Color(0, 0, 0, 0.62f));
        Renderer::drawRect(0, 0, (float)windowW, (float)windowH);
        inModal = true;
    }

    bool isModalActive() const { return modalPending; }

    /* ---------------------------------------------------------------
     * BUTTON
     * --------------------------------------------------------------- */
    void button(const Rect& r, const std::string& label, int id,
                bool active = false, bool enabled = true,
                void* font = Fonts::body(), const Color* activeColor = nullptr) {
        widgets.push_back({ r, id, KIND_BUTTON, enabled, inModal, 0, 0 });

        bool hover = enabled && canHover(r);
        Color fill = Theme::BTN;
        Color border = Theme::BTN_BORDER;
        Color text = enabled ? Theme::TEXT : Theme::TEXT_FAINT;

        if (active) {
            Color ac = activeColor ? *activeColor : Theme::ACCENT;
            fill = Theme::BTN.mix(ac, hover ? 0.45f : 0.35f);
            border = ac;
        } else if (hover) {
            fill = Theme::BTN_HOVER;
            border = Theme::TEXT_DIM;
        }

        setColor(fill);
        Renderer::drawRect(r);
        setColor(border);
        Renderer::drawRectOutline(r);
        setColor(text);
        Renderer::drawTextInBox(r, label, font);
    }

    /* Invisible clickable area (e.g. a table row). Returns true if hovered. */
    bool region(const Rect& r, int id) {
        widgets.push_back({ r, id, KIND_BUTTON, true, inModal, 0, 0 });
        return canHover(r);
    }

    /* ---------------------------------------------------------------
     * SLIDER  (value in [minVal, maxVal])
     * --------------------------------------------------------------- */
    void slider(const Rect& track, float value, float minVal, float maxVal, int id,
                const Color& fillColor = Theme::ACCENT) {
        widgets.push_back({ track, id, KIND_SLIDER, true, inModal, minVal, maxVal });

        float range = (maxVal - minVal);
        float t = range != 0.0f ? (value - minVal) / range : 0.0f;
        t = MathUtils::clamp(t, 0.0f, 1.0f);

        bool hover = canHover(Rect(track.x - 6, track.y - 8, track.w + 12, track.h + 16)) ||
                     activeSlider == id;

        float barH = 6.0f;
        float barY = track.cy() - barH / 2.0f;
        setColor(Theme::BTN);
        Renderer::drawRect(track.x, barY, track.w, barH);
        setColor(fillColor.withAlpha(hover ? 0.9f : 0.7f));
        Renderer::drawRect(track.x, barY, track.w * t, barH);

        float kx = track.x + track.w * t;
        setColor(hover ? Theme::TEXT : fillColor.mix(Theme::TEXT, 0.4f));
        Renderer::drawCircle(kx, track.cy(), hover ? 8.0f : 7.0f, true);
        setColor(Theme::PLOT_BG);
        Renderer::drawCircle(kx, track.cy(), 3.0f, true);
    }

    /* ---------------------------------------------------------------
     * HIT TESTING (used by the mouse handlers)
     * --------------------------------------------------------------- */

    /* Returns the widget under (mx, my), topmost first, or nullptr */
    const Widget* hitTest(int mx, int my) const {
        for (int i = (int)widgets.size() - 1; i >= 0; i--) {
            const Widget& w = widgets[i];
            if (modalPending && !w.modal) continue;
            if (!w.enabled) continue;
            Rect r = w.rect;
            if (w.kind == KIND_SLIDER) r = Rect(r.x - 8, r.y - 8, r.w + 16, r.h + 16);
            if (r.contains((float)mx, (float)my)) return &w;
        }
        return nullptr;
    }

    /* True if the point is over any clickable widget */
    bool overWidget(int mx, int my) const { return hitTest(mx, my) != nullptr; }

    /* Slider value under mouse x for a registered slider id */
    bool sliderValueAt(int id, int mx, float& outValue) const {
        for (const auto& w : widgets) {
            if (w.id == id && w.kind == KIND_SLIDER) {
                float t = MathUtils::clamp((mx - w.rect.x) / std::max(w.rect.w, 1.0f), 0.0f, 1.0f);
                outValue = w.minVal + t * (w.maxVal - w.minVal);
                return true;
            }
        }
        return false;
    }

    void setActiveSlider(int id) { activeSlider = id; }
    int getActiveSlider() const { return activeSlider; }
};

#endif // GLIDE_WIDGETS_H
