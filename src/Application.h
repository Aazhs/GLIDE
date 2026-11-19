/*
 * Application.h
 * -------------
 * Main application class for GLIDE.
 *
 * The application owns one object per screen and routes GLUT events to
 * the active one:
 *
 *   SCREEN_MENU     - title page
 *   SCREEN_GRAPH    - graph plotter
 *   SCREEN_LINES    - DDA / Bresenham line explorer
 *   SCREEN_CIRCLES  - DDA / Bresenham / Midpoint circle explorer
 *   SCREEN_POLYGONS - boundary, flood and scanline fill explorer
 *   SCREEN_HELP     - controls reference
 *
 * EVENT FLOW (mouse button pressed):
 *   1. Ask the UI which widget (button / slider) is under the cursor.
 *      A widget click goes to Screen::onWidget(id).
 *   2. If a popup is open, everything else is ignored.
 *   3. Otherwise the click goes to Screen::onMouseDown() (plot interaction).
 *
 * TIMING: a 16 ms GLUT timer calls Screen::tick() with the real elapsed
 * time, so animations play at the selected speed regardless of frame rate.
 */

#ifndef GLIDE_APPLICATION_H
#define GLIDE_APPLICATION_H

#include "Screens/MenuScreen.h"
#include "Screens/GraphScreen.h"
#include "Screens/LineExplorer.h"
#include "Screens/CircleExplorer.h"
#include "Screens/PolygonExplorer.h"
#include "Screens/HelpScreen.h"

#include <cstdlib>

class Application {
private:
    int windowWidth;
    int windowHeight;

    Settings settings;
    UI ui;

    MenuScreen menuScreen;
    GraphScreen graphScreen;
    LineExplorer lineScreen;
    CircleExplorer circleScreen;
    PolygonExplorer polygonScreen;
    HelpScreen helpScreen;

    Screen* screens[SCREEN_COUNT];
    ScreenId current;

    int lastTickMs;
    int currentCursor;
    bool mouseDown;

    Screen& screen() { return *screens[current]; }

    void switchTo(int id) {
        if (id < 0) return;
        if (id == SCREEN_EXIT) exit(0);
        current = (ScreenId)id;
        ui.setActiveSlider(-1);
        mouseDown = false;
        screen().onEnter();
    }

    /* After every event: follow navigation requests and redraw */
    void afterEvent() {
        switchTo(screen().takeNavRequest());
        glutPostRedisplay();
    }

    void updateCursor(int mx, int my) {
        int c = ui.overWidget(mx, my) && ui.getActiveSlider() < 0 ? GLUT_CURSOR_INFO
                                                                  : screen().cursorFor(mx, my);
        if (c != currentCursor) {
            glutSetCursor(c);
            currentCursor = c;
        }
    }

public:
    Application() :
        windowWidth(1280), windowHeight(820),
        menuScreen(settings), graphScreen(settings), lineScreen(settings),
        circleScreen(settings), polygonScreen(settings), helpScreen(settings),
        current(SCREEN_MENU), lastTickMs(0), currentCursor(-1), mouseDown(false)
    {
        screens[SCREEN_MENU] = &menuScreen;
        screens[SCREEN_GRAPH] = &graphScreen;
        screens[SCREEN_LINES] = &lineScreen;
        screens[SCREEN_CIRCLES] = &circleScreen;
        screens[SCREEN_POLYGONS] = &polygonScreen;
        screens[SCREEN_HELP] = &helpScreen;
        screens[SCREEN_EXIT] = &menuScreen;   // never displayed
    }

    /* ============================================================
     * INITIALIZATION
     * ============================================================ */
    void init(int argc, char** argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
        glutInitWindowSize(windowWidth, windowHeight);
        glutInitWindowPosition(80, 40);
        glutCreateWindow("GLIDE - Graphics Learning & Interactive Demonstration Environment");

        glClearColor(Theme::BG.r, Theme::BG.g, Theme::BG.b, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        setupProjection();
        lastTickMs = glutGet(GLUT_ELAPSED_TIME);
    }

    void setupProjection() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        /* Screen coordinates: (0,0) is the top-left corner */
        gluOrtho2D(0, windowWidth, windowHeight, 0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void reshape(int w, int h) {
        windowWidth = std::max(w, 1);
        windowHeight = std::max(h, 1);
        glViewport(0, 0, windowWidth, windowHeight);
        setupProjection();
    }

    /* ============================================================
     * RENDER
     * ============================================================ */
    void display() {
        glClear(GL_COLOR_BUFFER_BIT);
        ui.beginFrame(windowWidth, windowHeight, screen().hasModal());
        screen().render(ui);
        glutSwapBuffers();
    }

    /* ============================================================
     * KEYBOARD
     * ============================================================ */
    void keyboard(unsigned char key, int mx, int my) {
        (void)mx; (void)my;
        if (screen().wantsTextInput()) {
            screen().onKey(key);
        } else if (key == 27 && screen().hasModal()) {
            screen().closeModal();
        } else if (!screen().onKey(key)) {
            if (key == 27) switchTo(SCREEN_MENU);
            else if (key == 'h' || key == 'H' || key == '?') switchTo(SCREEN_HELP);
        }
        afterEvent();
    }

    void specialKey(int key, int /*mx*/, int /*my*/) {
        if (!screen().hasModal()) screen().onSpecialKey(key);
        afterEvent();
    }

    /* ============================================================
     * MOUSE
     * ============================================================ */
    void mouseClick(int button, int state, int mx, int my) {
        ui.setMouse(mx, my);

        /* Scroll wheel (freeglut reports it as buttons 3 / 4) */
        if (button == 3 || button == 4) {
            if (state == GLUT_DOWN && !screen().hasModal()) screen().onScroll(button == 3 ? 1 : -1, mx, my);
            afterEvent();
            return;
        }

        if (state == GLUT_DOWN) {
            mouseDown = true;
            const UI::Widget* w = (button == GLUT_LEFT_BUTTON) ? ui.hitTest(mx, my) : nullptr;
            if (w) {
                int id = w->id;
                if (w->kind == UI::KIND_SLIDER) {
                    float v;
                    ui.setActiveSlider(id);
                    if (ui.sliderValueAt(id, mx, v)) screen().onSlider(id, v);
                } else if (id == BTN_BACK) {
                    switchTo(SCREEN_MENU);
                } else {
                    screen().onWidget(id);
                }
            } else if (!screen().hasModal()) {
                screen().onMouseDown(button, mx, my);
            }
        } else {
            mouseDown = false;
            if (ui.getActiveSlider() >= 0) {
                ui.setActiveSlider(-1);
            } else {
                screen().onMouseUp(button, mx, my);
            }
        }
        updateCursor(mx, my);
        afterEvent();
    }

    /* Mouse moved with a button held */
    void mouseDrag(int mx, int my) {
        ui.setMouse(mx, my);
        int slider = ui.getActiveSlider();
        if (slider >= 0) {
            float v;
            if (ui.sliderValueAt(slider, mx, v)) screen().onSlider(slider, v);
        } else {
            screen().onMouseDrag(mx, my);
        }
        updateCursor(mx, my);
        afterEvent();
    }

    /* Mouse moved with no button held */
    void mouseMove(int mx, int my) {
        ui.setMouse(mx, my);
        screen().onMouseMove(mx, my);
        updateCursor(mx, my);
        glutPostRedisplay();
    }

    /* ============================================================
     * TIMER (~60 Hz)
     * ============================================================ */
    void timerTick() {
        int now = glutGet(GLUT_ELAPSED_TIME);
        int dt = std::min(now - lastTickMs, 250);   // ignore long stalls
        lastTickMs = now;
        if (screen().tick(dt)) glutPostRedisplay();
    }

    static const int TICK_MS = 16;
};

/* ============================================================
 * GLOBAL APPLICATION INSTANCE
 * (Required because GLUT uses C-style function pointers for callbacks)
 * ============================================================ */
Application* g_app = nullptr;

void displayCallback()              { if (g_app) g_app->display(); }
void reshapeCallback(int w, int h)  { if (g_app) g_app->reshape(w, h); }
void keyboardCallback(unsigned char key, int x, int y) {
    if (g_app) g_app->keyboard(key, x, y);
}
void specialCallback(int key, int x, int y) {
    if (g_app) g_app->specialKey(key, x, y);
}
void mouseCallback(int btn, int state, int x, int y) {
    if (g_app) g_app->mouseClick(btn, state, x, y);
}
void motionCallback(int x, int y) {
    if (g_app) g_app->mouseDrag(x, y);
}
void passiveMotionCallback(int x, int y) {
    if (g_app) g_app->mouseMove(x, y);
}
void timerCallback(int /*value*/) {
    if (g_app) {
        g_app->timerTick();
        glutTimerFunc(Application::TICK_MS, timerCallback, 0);
    }
}

#endif // GLIDE_APPLICATION_H
