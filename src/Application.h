/*
 * Application.h
 * -------------
 * Main application class for GLIDE.
 *
 * Manages application state, GLUT callbacks, rendering, and user interaction.
 * This is the central hub that connects all other components.
 *
 * APPLICATION STATES:
 *   MAIN_MENU         - Title screen with navigation
 *   GRAPH_PLOTTER     - Graph plotting environment
 *   ALGORITHM_EXPLORER - DDA/Bresenham line visualization
 *   CIRCLE_EXPLORER   - DDA/Bresenham circle visualization
 *   ABOUT_HELP        - Help and controls reference
 *
 * The Renderer class handles all OpenGL drawing.
 * The Application class handles state management and input.
 */

#ifndef GLIDE_APPLICATION_H
#define GLIDE_APPLICATION_H

#include "Common/Point.h"
#include "Common/AlgorithmStep.h"
#include "Common/MathUtils.h"
#include "Algorithms/DrawingAlgorithm.h"
#include "Algorithms/DDALine.h"
#include "Algorithms/BresenhamLine.h"
#include "Algorithms/DDACircle.h"
#include "Algorithms/BresenhamCircle.h"
#include "Graph/GraphFunction.h"
#include "Graph/GraphPlotter.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

/* ============================================================
 * COLOR PALETTE
 * ============================================================
 * Defines the color scheme used throughout the application.
 * Uses a dark theme for better visibility of plotted points.
 */
namespace Colors {
    /* Background */
    const float BG_R = 0.10f, BG_G = 0.10f, BG_B = 0.12f;

    /* Panels / UI Background */
    const float PANEL_R = 0.14f, PANEL_G = 0.14f, PANEL_B = 0.17f;

    /* Header */
    const float HEADER_R = 0.08f, HEADER_G = 0.08f, HEADER_B = 0.10f;

    /* Grid lines */
    const float GRID_R = 0.22f, GRID_G = 0.22f, GRID_B = 0.25f;

    /* Axes */
    const float AXIS_R = 0.5f, AXIS_G = 0.5f, AXIS_B = 0.55f;

    /* Primary accent (bright cyan) */
    const float ACCENT_R = 0.0f, ACCENT_G = 0.85f, ACCENT_B = 0.95f;

    /* Secondary accent (orange) */
    const float ACCENT2_R = 1.0f, ACCENT2_G = 0.6f, ACCENT2_B = 0.2f;

    /* Success (green) */
    const float GREEN_R = 0.3f, GREEN_G = 0.9f, GREEN_B = 0.4f;

    /* Warning (yellow) */
    const float YELLOW_R = 1.0f, YELLOW_G = 0.9f, YELLOW_B = 0.3f;

    /* Error (red) */
    const float RED_R = 1.0f, RED_G = 0.3f, RED_B = 0.3f;

    /* Text colors */
    const float TEXT_R = 0.9f, TEXT_G = 0.9f, TEXT_B = 0.92f;
    const float DIM_R = 0.55f, DIM_G = 0.55f, DIM_B = 0.6f;

    /* Button */
    const float BTN_R = 0.2f, BTN_G = 0.2f, BTN_B = 0.25f;
    const float BTN_HOVER_R = 0.28f, BTN_HOVER_G = 0.28f, BTN_HOVER_B = 0.35f;
    const float BTN_ACTIVE_R = 0.0f, BTN_ACTIVE_G = 0.5f, BTN_ACTIVE_B = 0.6f;

    /* Pixel grid highlight */
    const float PIXEL_R = 0.0f, PIXEL_G = 0.7f, PIXEL_B = 0.8f;
    const float PIXEL_SYM_R = 0.9f, PIXEL_SYM_G = 0.4f, PIXEL_SYM_B = 0.6f;
}

/* ============================================================
 * APPLICATION STATES
 * ============================================================ */
enum AppState {
    STATE_MAIN_MENU,
    STATE_GRAPH_PLOTTER,
    STATE_ALGORITHM_EXPLORER,
    STATE_CIRCLE_EXPLORER,
    STATE_ABOUT_HELP
};

/* Sub-states for algorithm explorer */
enum AlgoMode {
    ALGO_DDA_LINE,
    ALGO_BRESENHAM_LINE
};

/* Sub-states for circle explorer */
enum CircleMode {
    CIRCLE_DDA,
    CIRCLE_BRESENHAM
};

/* Animation state */
enum AnimState {
    ANIM_STOPPED,
    ANIM_PLAYING,
    ANIM_PAUSED
};

/* Graph view mode */
enum GraphViewMode {
    GRAPH_NORMAL,       // Just plot the result
    GRAPH_STEP_BY_STEP, // Animate segment by segment
    GRAPH_EXPLAIN       // Show algorithm details for each segment
};

/* ============================================================
 * SIMPLE BUTTON STRUCTURE
 * ============================================================ */
struct Button {
    int x, y, w, h;
    std::string label;
    bool hovered;
    bool active;        // toggle state
    int id;             // unique identifier

    Button() : x(0), y(0), w(0), h(0), hovered(false), active(false), id(0) {}
    Button(int x, int y, int w, int h, const std::string& label, int id = 0)
        : x(x), y(y), w(w), h(h), label(label), hovered(false), active(false), id(id) {}

    bool contains(int mx, int my) const {
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }
};

/* ============================================================
 * RENDERER CLASS
 * ============================================================
 * Handles ALL OpenGL drawing. Algorithm classes never call OpenGL.
 * This separation is important for understanding and viva explanation.
 */
class Renderer {
public:
    /* Draw text using GLUT bitmap font */
    static void drawText(float x, float y, const std::string& text,
                         void* font = GLUT_BITMAP_HELVETICA_12) {
        glRasterPos2f(x, y);
        for (char c : text) {
            if (c == '\n') {
                y += 15;
                glRasterPos2f(x, y);
            } else {
                glutBitmapCharacter(font, c);
            }
        }
    }

    /* Draw large text */
    static void drawLargeText(float x, float y, const std::string& text) {
        drawText(x, y, text, GLUT_BITMAP_HELVETICA_18);
    }

    /* Draw small text */
    static void drawSmallText(float x, float y, const std::string& text) {
        drawText(x, y, text, GLUT_BITMAP_HELVETICA_10);
    }

    /* Draw a filled rectangle */
    static void drawRect(float x, float y, float w, float h) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    /* Draw a rectangle outline */
    static void drawRectOutline(float x, float y, float w, float h) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    /* Draw a button with label */
    static void drawButton(const Button& btn) {
        /* Button background */
        if (btn.active) {
            glColor3f(Colors::BTN_ACTIVE_R, Colors::BTN_ACTIVE_G, Colors::BTN_ACTIVE_B);
        } else if (btn.hovered) {
            glColor3f(Colors::BTN_HOVER_R, Colors::BTN_HOVER_G, Colors::BTN_HOVER_B);
        } else {
            glColor3f(Colors::BTN_R, Colors::BTN_G, Colors::BTN_B);
        }
        drawRect((float)btn.x, (float)btn.y, (float)btn.w, (float)btn.h);

        /* Button border */
        if (btn.active) {
            glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        } else {
            glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        }
        drawRectOutline((float)btn.x, (float)btn.y, (float)btn.w, (float)btn.h);

        /* Button label (centered) */
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        int textW = 0;
        for (char c : btn.label) {
            textW += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, c);
        }
        float tx = btn.x + (btn.w - textW) / 2.0f;
        float ty = btn.y + btn.h / 2.0f + 4.0f;
        drawText(tx, ty, btn.label);
    }

    /* Draw a single pixel point (for algorithm visualization) */
    static void drawPixel(float x, float y, float size = 3.0f) {
        glPointSize(size);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }

    /* Draw a horizontal slider (track + thumb) */
    static void drawSlider(float x, float y, float w, float h,
                           float value, float minVal, float maxVal,
                           bool invertRatio = false) {
        /* Track background */
        glColor3f(0.12f, 0.12f, 0.18f);
        drawRect(x, y, w, h);

        /* Track border */
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        drawRectOutline(x, y, w, h);

        /* Ratio calculation */
        float ratio = (value - minVal) / (maxVal - minVal);
        if (invertRatio) ratio = 1.0f - ratio;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        /* Filled portion up to thumb */
        float fillW = w * ratio;
        glColor4f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B, 0.45f);
        drawRect(x, y, fillW, h);

        /* Thumb (draggable handle) */
        float thumbW = 10.0f;
        float thumbH = h + 8.0f;
        float thumbX = x + fillW - thumbW / 2.0f;
        if (thumbX < x) thumbX = x;
        if (thumbX + thumbW > x + w) thumbX = x + w - thumbW;
        float thumbY = y - 4.0f;

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        drawRect(thumbX, thumbY, thumbW, thumbH);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRectOutline(thumbX, thumbY, thumbW, thumbH);
    }

    /* Draw a pixel as a filled square (for pixel grid mode) */
    static void drawPixelSquare(float x, float y, float cellSize) {
        drawRect(x - cellSize/2, y - cellSize/2, cellSize, cellSize);
    }

    /* Draw the coordinate grid and axes */
    static void drawGrid(const CoordinateSystem& cs) {
        float minWorldX = cs.getVisibleMinX();
        float maxWorldX = cs.getVisibleMaxX();
        float minWorldY = cs.getVisibleMinY();
        float maxWorldY = cs.getVisibleMaxY();

        /* Determine grid spacing based on zoom level */
        float gridUnit = 1.0f;
        float pixelsPerUnit = cs.zoom * cs.scaleX;

        /* Adjust grid spacing so it's not too dense or sparse */
        if (pixelsPerUnit > 200) gridUnit = 0.25f;
        else if (pixelsPerUnit > 100) gridUnit = 0.5f;
        else if (pixelsPerUnit > 40) gridUnit = 1.0f;
        else if (pixelsPerUnit > 15) gridUnit = 2.0f;
        else if (pixelsPerUnit > 6) gridUnit = 5.0f;
        else gridUnit = 10.0f;

        /* Draw vertical grid lines */
        glColor3f(Colors::GRID_R, Colors::GRID_G, Colors::GRID_B);
        float startX = floorf(minWorldX / gridUnit) * gridUnit;
        for (float wx = startX; wx <= maxWorldX; wx += gridUnit) {
            float sx = cs.worldToScreenX(wx);
            if (sx >= cs.plotAreaX && sx <= cs.plotAreaX + cs.plotAreaW) {
                glBegin(GL_LINES);
                glVertex2f(sx, (float)cs.plotAreaY);
                glVertex2f(sx, (float)(cs.plotAreaY + cs.plotAreaH));
                glEnd();
            }
        }

        /* Draw horizontal grid lines */
        float startY = floorf(minWorldY / gridUnit) * gridUnit;
        for (float wy = startY; wy <= maxWorldY; wy += gridUnit) {
            float sy = cs.worldToScreenY(wy);
            if (sy >= cs.plotAreaY && sy <= cs.plotAreaY + cs.plotAreaH) {
                glBegin(GL_LINES);
                glVertex2f((float)cs.plotAreaX, sy);
                glVertex2f((float)(cs.plotAreaX + cs.plotAreaW), sy);
                glEnd();
            }
        }

        /* Draw axes (thicker, brighter) */
        glColor3f(Colors::AXIS_R, Colors::AXIS_G, Colors::AXIS_B);
        glLineWidth(2.0f);

        /* X-axis */
        float yAxisScreen = cs.worldToScreenY(0);
        if (yAxisScreen >= cs.plotAreaY && yAxisScreen <= cs.plotAreaY + cs.plotAreaH) {
            glBegin(GL_LINES);
            glVertex2f((float)cs.plotAreaX, yAxisScreen);
            glVertex2f((float)(cs.plotAreaX + cs.plotAreaW), yAxisScreen);
            glEnd();
        }

        /* Y-axis */
        float xAxisScreen = cs.worldToScreenX(0);
        if (xAxisScreen >= cs.plotAreaX && xAxisScreen <= cs.plotAreaX + cs.plotAreaW) {
            glBegin(GL_LINES);
            glVertex2f(xAxisScreen, (float)cs.plotAreaY);
            glVertex2f(xAxisScreen, (float)(cs.plotAreaY + cs.plotAreaH));
            glEnd();
        }

        glLineWidth(1.0f);

        /* Draw tick labels */
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        for (float wx = startX; wx <= maxWorldX; wx += gridUnit) {
            if (MathUtils::absf(wx) < 0.001f) continue; // Skip origin label
            float sx = cs.worldToScreenX(wx);
            if (sx >= cs.plotAreaX + 20 && sx <= cs.plotAreaX + cs.plotAreaW - 20) {
                float labelY = yAxisScreen + 14;
                if (labelY < cs.plotAreaY + 10) labelY = cs.plotAreaY + 14;
                if (labelY > cs.plotAreaY + cs.plotAreaH - 5)
                    labelY = cs.plotAreaY + cs.plotAreaH - 5;

                std::ostringstream oss;
                if (gridUnit >= 1.0f) oss << (int)wx;
                else oss << std::fixed << std::setprecision(1) << wx;
                drawSmallText(sx - 8, labelY, oss.str());
            }
        }
        for (float wy = startY; wy <= maxWorldY; wy += gridUnit) {
            if (MathUtils::absf(wy) < 0.001f) continue;
            float sy = cs.worldToScreenY(wy);
            if (sy >= cs.plotAreaY + 10 && sy <= cs.plotAreaY + cs.plotAreaH - 10) {
                float labelX = xAxisScreen + 5;
                if (labelX < cs.plotAreaX + 5) labelX = cs.plotAreaX + 5;
                if (labelX > cs.plotAreaX + cs.plotAreaW - 30)
                    labelX = cs.plotAreaX + cs.plotAreaW - 30;

                std::ostringstream oss;
                if (gridUnit >= 1.0f) oss << (int)wy;
                else oss << std::fixed << std::setprecision(1) << wy;
                drawSmallText(labelX, sy + 4, oss.str());
            }
        }

        /* Origin label */
        if (xAxisScreen >= cs.plotAreaX && xAxisScreen <= cs.plotAreaX + cs.plotAreaW &&
            yAxisScreen >= cs.plotAreaY && yAxisScreen <= cs.plotAreaY + cs.plotAreaH) {
            glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
            drawSmallText(xAxisScreen + 5, yAxisScreen + 14, "0");
        }
    }

    /* Draw a pixel grid overlay (for algorithm visualization) */
    static void drawPixelGrid(const CoordinateSystem& cs, float cellSize) {
        if (cellSize < 8.0f) return; // Too small to be useful

        glColor4f(Colors::GRID_R, Colors::GRID_G, Colors::GRID_B, 0.3f);

        /* Vertical lines */
        float startX = cs.worldToScreenX(floorf(cs.getVisibleMinX()));
        for (float x = startX; x <= cs.plotAreaX + cs.plotAreaW; x += cellSize) {
            if (x >= cs.plotAreaX) {
                glBegin(GL_LINES);
                glVertex2f(x, (float)cs.plotAreaY);
                glVertex2f(x, (float)(cs.plotAreaY + cs.plotAreaH));
                glEnd();
            }
        }

        /* Horizontal lines */
        float startY = cs.worldToScreenY(floorf(cs.getVisibleMaxY()));
        for (float y = startY; y <= cs.plotAreaY + cs.plotAreaH; y += cellSize) {
            if (y >= cs.plotAreaY) {
                glBegin(GL_LINES);
                glVertex2f((float)cs.plotAreaX, y);
                glVertex2f((float)(cs.plotAreaX + cs.plotAreaW), y);
                glEnd();
            }
        }
    }
};

/* ============================================================
 * APPLICATION CLASS
 * ============================================================
 * The main application. Manages state, rendering, and interaction.
 */
class Application {
private:
    /* Application state */
    AppState currentState;
    int windowWidth;
    int windowHeight;

    /* Graph plotter */
    GraphPlotter plotter;
    std::string inputExpression;        // Current text in the input field
    bool inputActive;                   // Whether the input field is focused
    GraphViewMode graphViewMode;
    int graphAnimSegment;               // Current segment being animated
    int graphAnimStep;                  // Current step within segment

    /* Algorithm explorer */
    AlgoMode currentAlgo;
    DDALine ddaLine;
    BresenhamLine bresenhamLine;
    Point algoStartPoint;
    Point algoEndPoint;
    bool settingStartPoint;             // true = next click sets start
    CoordinateSystem algoCoordSystem;
    int algoCurrentStep;
    AnimState algoAnimState;
    int animSpeed;                      // milliseconds per step
    bool showPixelGrid;

    /* Split view: show both DDA and Bresenham side by side */
    bool splitView;
    int algoCurrentStep2;               // Step counter for second algorithm in split view

    /* Circle explorer */
    CircleMode currentCircle;
    DDACircle ddaCircle;
    BresenhamCircle bresenhamCircle;
    Point circleCenter;
    float circleRadius;
    int circleCurrentStep;
    AnimState circleAnimState;
    CoordinateSystem circleCoordSystem;

    /* Quiz mode */
    bool quizActive;
    int quizCorrectAnswer;              // index 0-2
    std::string quizQuestion;
    std::string quizOptions[3];
    int quizSelectedAnswer;             // -1 = none
    bool quizAnswered;

    /* UI Elements */
    std::vector<Button> buttons;
    int hoveredButton;

    /* How It Works panel */
    bool showHowItWorks;

    /* Slider dragging state */
    bool draggingSlider;

public:
    Application() :
        currentState(STATE_MAIN_MENU),
        windowWidth(1200), windowHeight(800),
        inputActive(false),
        graphViewMode(GRAPH_NORMAL),
        graphAnimSegment(0), graphAnimStep(0),
        currentAlgo(ALGO_DDA_LINE),
        settingStartPoint(true),
        algoCurrentStep(0),
        algoAnimState(ANIM_STOPPED),
        animSpeed(800),
        showPixelGrid(true),
        splitView(false),
        algoCurrentStep2(0),
        currentCircle(CIRCLE_BRESENHAM),
        circleRadius(8.0f),
        circleCurrentStep(0),
        circleAnimState(ANIM_STOPPED),
        quizActive(false),
        quizCorrectAnswer(0),
        quizSelectedAnswer(-1),
        quizAnswered(false),
        hoveredButton(-1),
        showHowItWorks(false),
        draggingSlider(false)
    {
        inputExpression = "sin(x)";
        algoStartPoint = Point(2, 2);
        algoEndPoint = Point(12, 8);
        circleCenter = Point(0, 0);

        /* Set up coordinate systems for explorers.
         * plotAreaY = 100 leaves room for 55px header + 40px toolbar + 5px padding */
        algoCoordSystem.scaleX = 30.0f;
        algoCoordSystem.scaleY = 30.0f;
        algoCoordSystem.zoom = 1.5f;
        algoCoordSystem.plotAreaY = 100;

        circleCoordSystem.scaleX = 25.0f;
        circleCoordSystem.scaleY = 25.0f;
        circleCoordSystem.zoom = 1.5f;
        circleCoordSystem.plotAreaY = 100;
    }

    /* ============================================================
     * INITIALIZATION
     * ============================================================ */
    void init(int argc, char** argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
        glutInitWindowSize(windowWidth, windowHeight);
        glutInitWindowPosition(100, 50);
        glutCreateWindow("GLIDE - Graphics Learning & Interactive Demonstration Environment");

        /* OpenGL setup */
        glClearColor(Colors::BG_R, Colors::BG_G, Colors::BG_B, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POINT_SMOOTH);

        setupProjection();
    }

    void setupProjection() {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        /* Use screen coordinates: (0,0) is top-left */
        gluOrtho2D(0, windowWidth, windowHeight, 0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    /* ============================================================
     * WINDOW RESIZE
     * ============================================================ */
    void reshape(int w, int h) {
        windowWidth = w;
        windowHeight = h;
        glViewport(0, 0, w, h);
        setupProjection();

        /* Update plot areas */
        plotter.getCoordSystem().screenWidth = w;
        plotter.getCoordSystem().screenHeight = h;
        plotter.getCoordSystem().plotAreaW = w - 350;
        plotter.getCoordSystem().plotAreaH = h - 160;

        algoCoordSystem.screenWidth = w;
        algoCoordSystem.screenHeight = h;
        algoCoordSystem.plotAreaX = 10;
        algoCoordSystem.plotAreaY = 100;
        algoCoordSystem.plotAreaW = w - 380;
        algoCoordSystem.plotAreaH = h - 160;

        circleCoordSystem.screenWidth = w;
        circleCoordSystem.screenHeight = h;
        circleCoordSystem.plotAreaX = 10;
        circleCoordSystem.plotAreaY = 100;
        circleCoordSystem.plotAreaW = w - 380;
        circleCoordSystem.plotAreaH = h - 160;
    }

    /* ============================================================
     * MAIN RENDER DISPATCH
     * ============================================================ */
    void display() {
        glClear(GL_COLOR_BUFFER_BIT);

        switch (currentState) {
            case STATE_MAIN_MENU:       renderMainMenu(); break;
            case STATE_GRAPH_PLOTTER:   renderGraphPlotter(); break;
            case STATE_ALGORITHM_EXPLORER: renderAlgorithmExplorer(); break;
            case STATE_CIRCLE_EXPLORER: renderCircleExplorer(); break;
            case STATE_ABOUT_HELP:      renderAboutHelp(); break;
        }

        glutSwapBuffers();
    }

    /* ============================================================
     * MAIN MENU
     * ============================================================ */
    void renderMainMenu() {
        buttons.clear();

        /* Header background */
        glColor3f(Colors::HEADER_R, Colors::HEADER_G, Colors::HEADER_B);
        Renderer::drawRect(0, 0, (float)windowWidth, 130);

        /* Title */
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(windowWidth/2.0f - 40, 40, "GLIDE");

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(windowWidth/2.0f - 195, 65,
            "Graphics Learning & Interactive Demonstration Environment");

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(windowWidth/2.0f - 135, 90,
            "An Educational Computer Graphics Project");
        Renderer::drawSmallText(windowWidth/2.0f - 150, 110,
            "DDA Line | Bresenham Line | DDA Circle | Bresenham Circle");

        /* Menu buttons */
        int btnW = 300, btnH = 50;
        int startY = 180;
        int centerX = windowWidth / 2 - btnW / 2;
        int spacing = 70;

        buttons.push_back(Button(centerX, startY, btnW, btnH, "Graph Plotter", 1));
        buttons.push_back(Button(centerX, startY + spacing, btnW, btnH, "Algorithm Explorer (Lines)", 2));
        buttons.push_back(Button(centerX, startY + spacing * 2, btnW, btnH, "Circle Explorer", 3));
        buttons.push_back(Button(centerX, startY + spacing * 3, btnW, btnH, "About / Help", 4));
        buttons.push_back(Button(centerX, startY + spacing * 4, btnW, btnH, "Exit", 5));

        for (auto& btn : buttons) {
            Renderer::drawButton(btn);
        }

        /* Footer */
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(10, (float)windowHeight - 10,
            "Algorithm implementations are based on the algorithms taught in the provided course notes.");
    }

    /* ============================================================
     * GRAPH PLOTTER
     * ============================================================ */
    void renderGraphPlotter() {
        buttons.clear();
        CoordinateSystem& cs = plotter.getCoordSystem();

        /* Header */
        glColor3f(Colors::HEADER_R, Colors::HEADER_G, Colors::HEADER_B);
        Renderer::drawRect(0, 0, (float)windowWidth, 55);

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(10, 22, "GLIDE");
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(70, 22, "Graph Plotter");

        /* Back button */
        buttons.push_back(Button(windowWidth - 90, 10, 80, 30, "< Back", 100));
        Renderer::drawButton(buttons.back());

        /* Input area */
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(0, 55, (float)windowWidth, 45);

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawText(10, 78, "y = ");

        /* Input field */
        float inputX = 40;
        float inputW = 350;
        if (inputActive) {
            glColor3f(Colors::ACCENT_R * 0.3f, Colors::ACCENT_G * 0.3f, Colors::ACCENT_B * 0.3f);
        } else {
            glColor3f(0.18f, 0.18f, 0.22f);
        }
        Renderer::drawRect(inputX, 60, inputW, 28);
        glColor3f(inputActive ? Colors::ACCENT_R : Colors::DIM_R,
                  inputActive ? Colors::ACCENT_G : Colors::DIM_G,
                  inputActive ? Colors::ACCENT_B : Colors::DIM_B);
        Renderer::drawRectOutline(inputX, 60, inputW, 28);

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        std::string displayExpr = inputExpression;
        if (inputActive) displayExpr += "|";
        Renderer::drawText(inputX + 5, 80, displayExpr);

        /* Control buttons */
        int bx = 400;
        buttons.push_back(Button(bx, 62, 60, 26, "Plot", 101));
        buttons.push_back(Button(bx + 65, 62, 50, 26, "DDA", 102));
        buttons.push_back(Button(bx + 120, 62, 80, 26, "Bresenham", 103));

        /* Mark active algorithm */
        buttons[buttons.size() - 2].active = plotter.isUsingDDA();
        buttons[buttons.size() - 1].active = !plotter.isUsingDDA();

        buttons.push_back(Button(bx + 210, 62, 60, 26, "Step", 104));
        buttons.push_back(Button(bx + 275, 62, 70, 26, "Explain", 105));
        buttons.push_back(Button(bx + 350, 62, 80, 26, "How It Works", 106));

        buttons[buttons.size() - 3].active = (graphViewMode == GRAPH_STEP_BY_STEP);
        buttons[buttons.size() - 2].active = (graphViewMode == GRAPH_EXPLAIN);

        for (size_t i = 1; i < buttons.size(); i++) {
            Renderer::drawButton(buttons[i]);
        }

        /* Plot area background */
        glColor3f(Colors::BG_R, Colors::BG_G, Colors::BG_B);
        Renderer::drawRect((float)cs.plotAreaX, (float)cs.plotAreaY,
                          (float)cs.plotAreaW, (float)cs.plotAreaH);

        /* Set clipping to plot area */
        glEnable(GL_SCISSOR_TEST);
        glScissor(cs.plotAreaX, windowHeight - cs.plotAreaY - cs.plotAreaH,
                 cs.plotAreaW, cs.plotAreaH);

        /* Draw grid and axes */
        Renderer::drawGrid(cs);

        /* Draw the graph if it exists */
        if (plotter.graphExists()) {
            if (graphViewMode == GRAPH_NORMAL) {
                /* Normal mode: draw all raster points */
                glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
                glPointSize(2.0f);
                glBegin(GL_POINTS);
                for (const auto& pt : plotter.getAllRasterPoints()) {
                    glVertex2f(pt.x, pt.y);
                }
                glEnd();
            }
            else if (graphViewMode == GRAPH_STEP_BY_STEP ||
                     graphViewMode == GRAPH_EXPLAIN) {
                /* Animated mode: draw up to current segment/step */
                const auto& segs = plotter.getSegments();

                /* Draw completed segments */
                glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
                glPointSize(2.0f);
                glBegin(GL_POINTS);
                for (int s = 0; s < graphAnimSegment && s < (int)segs.size(); s++) {
                    for (const auto& pt : segs[s].rasterPoints) {
                        glVertex2f(pt.x, pt.y);
                    }
                }
                glEnd();

                /* Draw current segment up to current step */
                if (graphAnimSegment < (int)segs.size()) {
                    const auto& curSeg = segs[graphAnimSegment];
                    glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
                    glPointSize(3.0f);
                    glBegin(GL_POINTS);
                    for (int st = 0; st <= graphAnimStep && st < (int)curSeg.steps.size(); st++) {
                        for (const auto& pt : curSeg.steps[st].plottedPoints) {
                            glVertex2f(pt.x, pt.y);
                        }
                    }
                    glEnd();

                    /* Highlight current pixel */
                    if (graphAnimStep < (int)curSeg.steps.size()) {
                        const auto& step = curSeg.steps[graphAnimStep];
                        if (!step.plottedPoints.empty()) {
                            glColor3f(Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);
                            glPointSize(7.0f);
                            glBegin(GL_POINTS);
                            glVertex2f(step.plottedPoints[0].x, step.plottedPoints[0].y);
                            glEnd();
                        }
                    }

                    /* Draw segment endpoints */
                    glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
                    glPointSize(6.0f);
                    glBegin(GL_POINTS);
                    glVertex2f(curSeg.screenStart.x, curSeg.screenStart.y);
                    glVertex2f(curSeg.screenEnd.x, curSeg.screenEnd.y);
                    glEnd();
                }
            }
        }

        glDisable(GL_SCISSOR_TEST);

        /* Right info panel */
        float panelX = (float)(cs.plotAreaX + cs.plotAreaW + 5);
        float panelW = windowWidth - panelX;
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(panelX, (float)cs.plotAreaY, panelW, (float)cs.plotAreaH);

        float infoY = cs.plotAreaY + 20.0f;

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawText(panelX + 10, infoY, "Function Info");
        infoY += 20;

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(panelX + 10, infoY, "y = " + plotter.getFunction().getExpression());
        infoY += 18;
        Renderer::drawSmallText(panelX + 10, infoY,
            "Algorithm: " + std::string(plotter.isUsingDDA() ? "DDA" : "Bresenham"));
        infoY += 18;
        Renderer::drawSmallText(panelX + 10, infoY,
            "Samples: " + std::to_string(plotter.getSampleCount()));
        infoY += 18;

        if (plotter.graphExists()) {
            Renderer::drawSmallText(panelX + 10, infoY,
                "Segments: " + std::to_string(plotter.getSegments().size()));
            infoY += 18;
            Renderer::drawSmallText(panelX + 10, infoY,
                "Total pixels: " + std::to_string(plotter.getAllRasterPoints().size()));
            infoY += 25;
        }

        /* Show current step details in explain mode */
        if ((graphViewMode == GRAPH_STEP_BY_STEP || graphViewMode == GRAPH_EXPLAIN) &&
            plotter.graphExists()) {
            const auto& segs = plotter.getSegments();
            if (graphAnimSegment < (int)segs.size()) {
                const auto& curSeg = segs[graphAnimSegment];

                glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
                Renderer::drawText(panelX + 10, infoY, "Current Segment");
                infoY += 18;

                glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
                std::ostringstream segInfo;
                segInfo << "Seg " << (graphAnimSegment + 1) << " / " << segs.size();
                Renderer::drawSmallText(panelX + 10, infoY, segInfo.str());
                infoY += 15;

                segInfo.str("");
                segInfo << "P1 = " << curSeg.worldStart.toString();
                Renderer::drawSmallText(panelX + 10, infoY, segInfo.str());
                infoY += 15;

                segInfo.str("");
                segInfo << "P2 = " << curSeg.worldEnd.toString();
                Renderer::drawSmallText(panelX + 10, infoY, segInfo.str());
                infoY += 20;

                if (graphAnimStep < (int)curSeg.steps.size() && graphViewMode == GRAPH_EXPLAIN) {
                    const auto& step = curSeg.steps[graphAnimStep];

                    glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
                    Renderer::drawText(panelX + 10, infoY, "Step Details");
                    infoY += 18;

                    glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);

                    /* Display the explanation text line by line */
                    std::istringstream iss(step.explanation);
                    std::string line;
                    while (std::getline(iss, line) && infoY < cs.plotAreaY + cs.plotAreaH - 10) {
                        Renderer::drawSmallText(panelX + 10, infoY, line);
                        infoY += 14;
                    }
                }
            }

            /* Navigation buttons for step mode */
            int navY = cs.plotAreaY + cs.plotAreaH + 10;
            buttons.push_back(Button((int)panelX + 10, navY, 60, 28, "< Prev", 110));
            buttons.push_back(Button((int)panelX + 75, navY, 60, 28, "Next >", 111));
            buttons.push_back(Button((int)panelX + 140, navY, 60, 28, "Reset", 112));
            for (size_t i = buttons.size() - 3; i < buttons.size(); i++) {
                Renderer::drawButton(buttons[i]);
            }
        }

        /* How It Works panel */
        if (showHowItWorks) {
            renderHowItWorks();
        }

        /* Bottom bar: zoom/pan controls */
        float bottomY = (float)(windowHeight - 55);
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(0, bottomY, (float)windowWidth, 55);

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(10, bottomY + 15,
            "Zoom: +/- or scroll | Pan: Arrow keys | R: Reset view | Click plot area for input");
        Renderer::drawSmallText(10, bottomY + 32,
            "Samples: [ decrease, ] increase | Current zoom: " +
            std::to_string(plotter.getCoordSystem().zoom).substr(0, 4));

        /* Preset function buttons */
        int presetX = 300;
        std::string presets[] = {"x*x", "sin(x)", "cos(x)", "2*x+1", "x^3", "sqrt(abs(x))"};
        for (int i = 0; i < 6; i++) {
            buttons.push_back(Button(presetX + i * 75, (int)bottomY + 10, 70, 26, presets[i], 120 + i));
            Renderer::drawButton(buttons.back());
        }
    }

    /* ============================================================
     * HOW IT WORKS PANEL
     * ============================================================ */
    void renderHowItWorks() {
        /* Semi-transparent overlay */
        glColor4f(0, 0, 0, 0.7f);
        Renderer::drawRect(0, 0, (float)windowWidth, (float)windowHeight);

        /* Panel */
        float pw = 600, ph = 450;
        float px = (windowWidth - pw) / 2;
        float py = (windowHeight - ph) / 2;

        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(px, py, pw, ph);
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawRectOutline(px, py, pw, ph);

        float ty = py + 30;
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(px + 20, ty, "How Is This Graph Drawn?");
        ty += 35;

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        std::string lines[] = {
            "1. You enter a mathematical function (e.g., y = sin(x))",
            "",
            "2. GLIDE evaluates the function at many sampled x values,",
            "   generating a series of (x, y) coordinate pairs.",
            "",
            "3. These mathematical coordinates are converted to screen",
            "   coordinates using the current zoom, pan, and scale.",
            "",
            "4. Consecutive sample points are connected by running the",
            "   selected rasterization algorithm (DDA or Bresenham).",
            "",
            "5. Each algorithm call generates a series of integer pixel",
            "   positions that approximate the line segment.",
            "",
            "6. OpenGL renders these pixels on screen using GL_POINTS.",
            "",
            "7. Repeating this for all consecutive pairs of sample points",
            "   builds the complete graph curve.",
            "",
            "This is the central idea of GLIDE: the graph is not drawn",
            "by OpenGL's built-in line drawing. Instead, the classical",
            "rasterization algorithms produce every pixel."
        };

        for (const auto& line : lines) {
            Renderer::drawSmallText(px + 25, ty, line);
            ty += 16;
        }

        /* Close button */
        buttons.push_back(Button((int)px + (int)pw - 80, (int)py + (int)ph - 40,
                                 60, 28, "Close", 199));
        Renderer::drawButton(buttons.back());
    }

    /* ============================================================
     * ALGORITHM EXPLORER
     * ============================================================ */
    /*
     * Helper: draw one algorithm's visualization in a given plot region.
     * Used by both single-view and split-view modes.
     */
    void drawAlgoVisualization(const CoordinateSystem& cs,
                              const std::vector<AlgorithmStep>& steps,
                              int currentStep,
                              float plotR, float plotG, float plotB,
                              float highlightR, float highlightG, float highlightB) {
        if (steps.empty()) return;

        float cellSize = cs.zoom * cs.scaleX;
        bool useCells = showPixelGrid && cellSize >= 8.0f;

        /* Draw completed steps */
        for (int i = 0; i < currentStep && i < (int)steps.size(); i++) {
            const auto& step = steps[i];
            for (const auto& pt : step.plottedPoints) {
                float sx = cs.worldToScreenX(pt.x);
                float sy = cs.worldToScreenY(pt.y);
                if (useCells) {
                    glColor4f(plotR, plotG, plotB, 0.6f);
                    Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                } else {
                    glColor3f(plotR, plotG, plotB);
                    Renderer::drawPixel(sx, sy, 4.0f);
                }
            }
        }

        /* Draw current step (highlighted) */
        if (currentStep > 0 && currentStep <= (int)steps.size()) {
            const auto& step = steps[currentStep - 1];
            for (const auto& pt : step.plottedPoints) {
                float sx = cs.worldToScreenX(pt.x);
                float sy = cs.worldToScreenY(pt.y);
                if (useCells) {
                    glColor3f(highlightR, highlightG, highlightB);
                    Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                    glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
                    Renderer::drawSmallText(sx - 10, sy - cellSize/2 - 5,
                        pt.toPixelString());
                } else {
                    glColor3f(highlightR, highlightG, highlightB);
                    Renderer::drawPixel(sx, sy, 8.0f);
                }
            }
        }
    }

    /*
     * Helper: draw the info panel for one algorithm.
     * Returns the Y position after the last line drawn.
     */
    float drawAlgoInfoPanel(float panelX, float panelW, float startY, float maxY,
                            const std::string& algoName, const std::string& desc,
                            const std::vector<AlgorithmStep>& steps, int currentStep,
                            bool showDecisionVar) {
        float infoY = startY;

        /* Algorithm name */
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawText(panelX + 10, infoY, algoName);
        infoY += 22;

        /* Algorithm description (word wrap) */
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        std::string word;
        std::istringstream descStream(desc);
        std::string curLine;
        int maxChars = (int)(panelW / 7);  // approximate chars per line
        if (maxChars < 20) maxChars = 20;
        while (descStream >> word) {
            if ((int)(curLine.length() + word.length() + 1) > maxChars) {
                Renderer::drawSmallText(panelX + 10, infoY, curLine);
                infoY += 14;
                curLine = word;
            } else {
                if (!curLine.empty()) curLine += " ";
                curLine += word;
            }
        }
        if (!curLine.empty()) {
            Renderer::drawSmallText(panelX + 10, infoY, curLine);
            infoY += 14;
        }
        infoY += 8;

        /* Parameters */
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawSmallText(panelX + 10, infoY,
            "Start: " + algoStartPoint.toPixelString());
        infoY += 16;
        Renderer::drawSmallText(panelX + 10, infoY,
            "End: " + algoEndPoint.toPixelString());
        infoY += 18;

        /* Step info */
        if (!steps.empty()) {
            glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
            std::ostringstream stepLabel;
            stepLabel << "Step " << currentStep << " / " << steps.size();
            Renderer::drawText(panelX + 10, infoY, stepLabel.str());
            infoY += 20;

            if (currentStep > 0 && currentStep <= (int)steps.size()) {
                const auto& step = steps[currentStep - 1];
                glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);

                std::istringstream iss(step.explanation);
                std::string line;
                while (std::getline(iss, line) && infoY < maxY - 40) {
                    Renderer::drawSmallText(panelX + 10, infoY, line);
                    infoY += 14;
                }
                infoY += 6;

                if (showDecisionVar) {
                    glColor3f(Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);
                    std::ostringstream dv;
                    dv << "e = " << (int)step.decisionVariable;
                    Renderer::drawText(panelX + 10, infoY, dv.str());
                    infoY += 16;
                    glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
                    Renderer::drawSmallText(panelX + 10, infoY, step.decision);
                    infoY += 14;
                }
            }
        }
        return infoY;
    }

    void renderAlgorithmExplorer() {
        buttons.clear();
        CoordinateSystem& cs = algoCoordSystem;

        /* Header */
        glColor3f(Colors::HEADER_R, Colors::HEADER_G, Colors::HEADER_B);
        Renderer::drawRect(0, 0, (float)windowWidth, 55);

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(10, 22, "GLIDE");
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(70, 22, "Algorithm Explorer");

        buttons.push_back(Button(windowWidth - 90, 10, 80, 30, "< Back", 100));
        Renderer::drawButton(buttons.back());

        /* Algorithm selection toolbar */
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(0, 55, (float)windowWidth, 40);

        buttons.push_back(Button(10, 62, 90, 26, "DDA Line", 201));
        buttons.push_back(Button(106, 62, 120, 26, "Bresenham Line", 202));
        buttons.push_back(Button(232, 62, 90, 26,
            splitView ? "Single View" : "Split View", 206));
        buttons.push_back(Button(328, 62, 80, 26,
            showPixelGrid ? "Grid: ON" : "Grid: OFF", 203));

        /* Active algorithm visual highlight */
        if (!splitView) {
            buttons[1].active = (currentAlgo == ALGO_DDA_LINE);
            buttons[2].active = (currentAlgo == ALGO_BRESENHAM_LINE);
            buttons[3].active = false;
        } else {
            buttons[1].active = false;
            buttons[2].active = false;
            buttons[3].active = true;
        }
        buttons[4].active = showPixelGrid;

        for (size_t i = 1; i < buttons.size(); i++) {
            Renderer::drawButton(buttons[i]);
        }

        /* Speed slider */
        float trackX = 505.0f, trackY = 68.0f, trackW = 130.0f, trackH = 14.0f;
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawSmallText(420, 78, "Speed:");
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(468, 78, "Slow");
        Renderer::drawSlider(trackX, trackY, trackW, trackH,
                             (float)animSpeed, 100.0f, 2000.0f, true);
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(trackX + trackW + 8, 78, "Fast");
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawSmallText(trackX + trackW + 38, 78, std::to_string(animSpeed) + "ms");

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(trackX + trackW + 95, 78, "Click plot to set points");

        if (splitView) {
            /* ==================== SPLIT VIEW ==================== */
            /* Both algorithms displayed side-by-side with synchronized animation */
            int splitPlotH = windowHeight - 340;
            if (splitPlotH < 220) splitPlotH = 220;
            int halfW = (windowWidth - 30) / 2;

            /* --- LEFT PLOT (DDA) --- */
            int leftPlotX = 10;
            glColor3f(Colors::BG_R, Colors::BG_G, Colors::BG_B);
            Renderer::drawRect((float)leftPlotX, (float)cs.plotAreaY,
                              (float)halfW, (float)splitPlotH);

            CoordinateSystem leftCS = cs;
            leftCS.plotAreaX = leftPlotX;
            leftCS.plotAreaW = halfW;
            leftCS.plotAreaH = splitPlotH;

            glEnable(GL_SCISSOR_TEST);
            glScissor(leftPlotX, windowHeight - cs.plotAreaY - splitPlotH,
                     halfW, splitPlotH);

            Renderer::drawGrid(leftCS);
            if (showPixelGrid) {
                Renderer::drawPixelGrid(leftCS, leftCS.zoom * leftCS.scaleX);
            }

            drawAlgoVisualization(leftCS, ddaLine.getSteps(), algoCurrentStep,
                Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B,
                Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);

            /* Endpoints on left */
            glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
            Renderer::drawPixel(leftCS.worldToScreenX(algoStartPoint.x),
                               leftCS.worldToScreenY(algoStartPoint.y), 6.0f);
            glColor3f(Colors::RED_R, Colors::RED_G, Colors::RED_B);
            Renderer::drawPixel(leftCS.worldToScreenX(algoEndPoint.x),
                               leftCS.worldToScreenY(algoEndPoint.y), 6.0f);

            glDisable(GL_SCISSOR_TEST);

            /* DDA title label */
            glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
            Renderer::drawText((float)(leftPlotX + 8), (float)(cs.plotAreaY + 18), "DDA Line (Floating-point increments)");

            /* --- RIGHT PLOT (Bresenham) --- */
            int rightPlotX = leftPlotX + halfW + 10;
            glColor3f(Colors::BG_R, Colors::BG_G, Colors::BG_B);
            Renderer::drawRect((float)rightPlotX, (float)cs.plotAreaY,
                              (float)halfW, (float)splitPlotH);

            CoordinateSystem rightCS = cs;
            rightCS.plotAreaX = rightPlotX;
            rightCS.plotAreaW = halfW;
            rightCS.plotAreaH = splitPlotH;

            glEnable(GL_SCISSOR_TEST);
            glScissor(rightPlotX, windowHeight - cs.plotAreaY - splitPlotH,
                     halfW, splitPlotH);

            Renderer::drawGrid(rightCS);
            if (showPixelGrid) {
                Renderer::drawPixelGrid(rightCS, rightCS.zoom * rightCS.scaleX);
            }

            drawAlgoVisualization(rightCS, bresenhamLine.getSteps(), algoCurrentStep2,
                Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B,
                Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);

            /* Endpoints on right */
            glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
            Renderer::drawPixel(rightCS.worldToScreenX(algoStartPoint.x),
                               rightCS.worldToScreenY(algoStartPoint.y), 6.0f);
            glColor3f(Colors::RED_R, Colors::RED_G, Colors::RED_B);
            Renderer::drawPixel(rightCS.worldToScreenX(algoEndPoint.x),
                               rightCS.worldToScreenY(algoEndPoint.y), 6.0f);

            glDisable(GL_SCISSOR_TEST);

            /* Bresenham title label */
            glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
            Renderer::drawText((float)(rightPlotX + 8), (float)(cs.plotAreaY + 18), "Bresenham Line (Integer decision variable)");

            /* Divider line */
            glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
            glBegin(GL_LINES);
            glVertex2f((float)(leftPlotX + halfW + 5), (float)cs.plotAreaY);
            glVertex2f((float)(leftPlotX + halfW + 5), (float)(cs.plotAreaY + splitPlotH));
            glEnd();

            /* Info panels below the plot area (side by side) */
            float infoPanelY = (float)(cs.plotAreaY + splitPlotH + 8);
            float infoPanelH = (float)(windowHeight - infoPanelY - 55);

            /* Left info (DDA) */
            glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
            Renderer::drawRect((float)leftPlotX, infoPanelY, (float)halfW, infoPanelH);
            drawAlgoInfoPanel((float)leftPlotX, (float)halfW, infoPanelY + 10,
                infoPanelY + infoPanelH, "DDA Line", ddaLine.getDescription(),
                ddaLine.getSteps(), algoCurrentStep, false);

            /* Right info (Bresenham) */
            glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
            Renderer::drawRect((float)rightPlotX, infoPanelY, (float)halfW, infoPanelH);
            drawAlgoInfoPanel((float)rightPlotX, (float)halfW, infoPanelY + 10,
                infoPanelY + infoPanelH, "Bresenham Line", bresenhamLine.getDescription(),
                bresenhamLine.getSteps(), algoCurrentStep2, true);

            /* Navigation buttons at very bottom */
            int navY = windowHeight - 48;
            glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
            Renderer::drawRect(0, (float)navY - 5, (float)windowWidth, 53);

            buttons.push_back(Button(10, navY, 80, 30, "< Previous", 210));
            buttons.push_back(Button(95, navY, 70, 30, "Next >", 211));
            buttons.push_back(Button(170, navY, 60, 30, "Play", 212));
            buttons.push_back(Button(235, navY, 60, 30, "Pause", 213));
            buttons.push_back(Button(300, navY, 60, 30, "Reset", 214));
            buttons.push_back(Button(380, navY, 80, 30, "Run Algo", 215));

            buttons[buttons.size() - 4].active = (algoAnimState == ANIM_PLAYING);
            buttons[buttons.size() - 3].active = (algoAnimState == ANIM_PAUSED);

            for (size_t i = buttons.size() - 6; i < buttons.size(); i++) {
                Renderer::drawButton(buttons[i]);
            }

            glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
            Renderer::drawSmallText(480, (float)(navY + 20),
                settingStartPoint ? "Click either plot to set START" : "Click either plot to set END");

        } else {
            /* ==================== SINGLE VIEW ==================== */
            /* Plot area */
            glColor3f(Colors::BG_R, Colors::BG_G, Colors::BG_B);
            Renderer::drawRect((float)cs.plotAreaX, (float)cs.plotAreaY,
                              (float)cs.plotAreaW, (float)cs.plotAreaH);

            glEnable(GL_SCISSOR_TEST);
            glScissor(cs.plotAreaX, windowHeight - cs.plotAreaY - cs.plotAreaH,
                     cs.plotAreaW, cs.plotAreaH);

            Renderer::drawGrid(cs);

            if (showPixelGrid) {
                float cellSize = cs.zoom * cs.scaleX;
                Renderer::drawPixelGrid(cs, cellSize);
            }

            /* Get current algorithm steps */
            const std::vector<AlgorithmStep>* currentSteps = nullptr;
            bool isBresenham = false;
            if (currentAlgo == ALGO_DDA_LINE) {
                currentSteps = &ddaLine.getSteps();
            } else {
                currentSteps = &bresenhamLine.getSteps();
                isBresenham = true;
            }

            if (currentSteps) {
                drawAlgoVisualization(cs, *currentSteps, algoCurrentStep,
                    Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B,
                    Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);
            }

            /* Draw endpoint markers */
            float sx1 = cs.worldToScreenX(algoStartPoint.x);
            float sy1 = cs.worldToScreenY(algoStartPoint.y);
            float sx2 = cs.worldToScreenX(algoEndPoint.x);
            float sy2 = cs.worldToScreenY(algoEndPoint.y);

            glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
            Renderer::drawPixel(sx1, sy1, 8.0f);
            glColor3f(Colors::RED_R, Colors::RED_G, Colors::RED_B);
            Renderer::drawPixel(sx2, sy2, 8.0f);

            glColor3f(Colors::GREEN_R, Colors::GREEN_G, Colors::GREEN_B);
            Renderer::drawSmallText(sx1 + 8, sy1 - 5, "Start " + algoStartPoint.toPixelString());
            glColor3f(Colors::RED_R, Colors::RED_G, Colors::RED_B);
            Renderer::drawSmallText(sx2 + 8, sy2 - 5, "End " + algoEndPoint.toPixelString());

            glDisable(GL_SCISSOR_TEST);

            /* Right info panel */
            float panelX = (float)(cs.plotAreaX + cs.plotAreaW + 5);
            float panelW = windowWidth - panelX;
            glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
            Renderer::drawRect(panelX, (float)cs.plotAreaY, panelW, (float)cs.plotAreaH);

            std::string algoName = isBresenham ? "Bresenham Line" : "DDA Line";
            std::string desc = isBresenham ? bresenhamLine.getDescription() : ddaLine.getDescription();

            drawAlgoInfoPanel(panelX, panelW, cs.plotAreaY + 20.0f,
                (float)(cs.plotAreaY + cs.plotAreaH), algoName, desc,
                currentSteps ? *currentSteps : std::vector<AlgorithmStep>(),
                algoCurrentStep, isBresenham);

            /* Navigation buttons */
            int navY = cs.plotAreaY + cs.plotAreaH + 10;
            buttons.push_back(Button(10, navY, 80, 30, "< Previous", 210));
            buttons.push_back(Button(95, navY, 70, 30, "Next >", 211));
            buttons.push_back(Button(170, navY, 60, 30, "Play", 212));
            buttons.push_back(Button(235, navY, 60, 30, "Pause", 213));
            buttons.push_back(Button(300, navY, 60, 30, "Reset", 214));
            buttons.push_back(Button(380, navY, 80, 30, "Run Algo", 215));
            buttons.push_back(Button(470, navY, 60, 30, "Quiz", 216));

            buttons[buttons.size() - 5].active = (algoAnimState == ANIM_PLAYING);
            buttons[buttons.size() - 4].active = (algoAnimState == ANIM_PAUSED);

            for (size_t i = buttons.size() - 7; i < buttons.size(); i++) {
                Renderer::drawButton(buttons[i]);
            }

            glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
            Renderer::drawSmallText(550, (float)(navY + 20),
                settingStartPoint ? "Click to set START point" : "Click to set END point");
        }

        /* Quiz overlay */
        if (quizActive) {
            renderQuiz();
        }
    }

    /* ============================================================
     * CIRCLE EXPLORER
     * ============================================================ */
    void renderCircleExplorer() {
        buttons.clear();
        CoordinateSystem& cs = circleCoordSystem;

        /* Header */
        glColor3f(Colors::HEADER_R, Colors::HEADER_G, Colors::HEADER_B);
        Renderer::drawRect(0, 0, (float)windowWidth, 55);

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(10, 22, "GLIDE");
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(70, 22, "Circle Explorer");

        buttons.push_back(Button(windowWidth - 90, 10, 80, 30, "< Back", 100));
        Renderer::drawButton(buttons.back());

        /* Mode selection */
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(0, 55, (float)windowWidth, 40);

        buttons.push_back(Button(10, 62, 100, 26, "DDA Circle", 301));
        buttons.push_back(Button(116, 62, 135, 26, "Bresenham Circle", 302));
        buttons.push_back(Button(257, 62, 40, 26, "R +", 303));
        buttons.push_back(Button(303, 62, 40, 26, "R -", 304));
        buttons.push_back(Button(349, 62, 80, 26,
            showPixelGrid ? "Grid: ON" : "Grid: OFF", 305));

        buttons[1].active = (currentCircle == CIRCLE_DDA);
        buttons[2].active = (currentCircle == CIRCLE_BRESENHAM);
        buttons[buttons.size() - 1].active = showPixelGrid;

        for (size_t i = 1; i < buttons.size(); i++) {
            Renderer::drawButton(buttons[i]);
        }

        /* Speed slider */
        float trackX = 525.0f, trackY = 68.0f, trackW = 130.0f, trackH = 14.0f;
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawSmallText(440, 78, "Speed:");
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(488, 78, "Slow");
        Renderer::drawSlider(trackX, trackY, trackW, trackH,
                             (float)animSpeed, 100.0f, 2000.0f, true);
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(trackX + trackW + 8, 78, "Fast");
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawSmallText(trackX + trackW + 38, 78, std::to_string(animSpeed) + "ms");

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        std::ostringstream rStr;
        rStr << "Radius: " << circleRadius;
        Renderer::drawSmallText(trackX + trackW + 90, 78, rStr.str());

        /* Plot area */
        glColor3f(Colors::BG_R, Colors::BG_G, Colors::BG_B);
        Renderer::drawRect((float)cs.plotAreaX, (float)cs.plotAreaY,
                          (float)cs.plotAreaW, (float)cs.plotAreaH);

        glEnable(GL_SCISSOR_TEST);
        glScissor(cs.plotAreaX, windowHeight - cs.plotAreaY - cs.plotAreaH,
                 cs.plotAreaW, cs.plotAreaH);

        Renderer::drawGrid(cs);

        if (showPixelGrid) {
            float cellSize = cs.zoom * cs.scaleX;
            Renderer::drawPixelGrid(cs, cellSize);
        }

        /* Draw reference circle (thin, dim) */
        glColor4f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B, 0.4f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 120; i++) {
            float angle = 2.0f * MathUtils::PI * i / 120.0f;
            float wx = circleCenter.x + circleRadius * cosf(angle);
            float wy = circleCenter.y + circleRadius * sinf(angle);
            glVertex2f(cs.worldToScreenX(wx), cs.worldToScreenY(wy));
        }
        glEnd();

        /* Get current algorithm steps */
        const std::vector<AlgorithmStep>* currentSteps = nullptr;
        if (currentCircle == CIRCLE_DDA) {
            currentSteps = &ddaCircle.getSteps();
        } else {
            currentSteps = &bresenhamCircle.getSteps();
        }

        /* Draw algorithm visualization */
        if (currentSteps && !currentSteps->empty()) {
            float cellSize = cs.zoom * cs.scaleX;
            bool useCells = showPixelGrid && cellSize >= 8.0f;

            for (int i = 0; i < circleCurrentStep && i < (int)currentSteps->size(); i++) {
                const auto& step = (*currentSteps)[i];

                /* For Bresenham circle, show symmetric points differently */
                if (currentCircle == CIRCLE_BRESENHAM && !step.symmetricPoints.empty()) {
                    /* First point (calculated) in primary color */
                    if (!step.symmetricPoints.empty()) {
                        float sx = cs.worldToScreenX(step.symmetricPoints[0].x);
                        float sy = cs.worldToScreenY(step.symmetricPoints[0].y);
                        if (useCells) {
                            glColor4f(Colors::PIXEL_R, Colors::PIXEL_G, Colors::PIXEL_B, 0.7f);
                            Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                        } else {
                            glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
                            Renderer::drawPixel(sx, sy, 4.0f);
                        }
                    }

                    /* Symmetric points in secondary color */
                    for (size_t s = 1; s < step.symmetricPoints.size(); s++) {
                        float sx = cs.worldToScreenX(step.symmetricPoints[s].x);
                        float sy = cs.worldToScreenY(step.symmetricPoints[s].y);
                        if (useCells) {
                            glColor4f(Colors::PIXEL_SYM_R, Colors::PIXEL_SYM_G,
                                     Colors::PIXEL_SYM_B, 0.5f);
                            Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                        } else {
                            glColor3f(Colors::PIXEL_SYM_R, Colors::PIXEL_SYM_G,
                                     Colors::PIXEL_SYM_B);
                            Renderer::drawPixel(sx, sy, 3.0f);
                        }
                    }
                } else {
                    /* DDA circle: all points same color */
                    for (const auto& pt : step.plottedPoints) {
                        float sx = cs.worldToScreenX(pt.x);
                        float sy = cs.worldToScreenY(pt.y);
                        if (useCells) {
                            glColor4f(Colors::PIXEL_R, Colors::PIXEL_G, Colors::PIXEL_B, 0.6f);
                            Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                        } else {
                            glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
                            Renderer::drawPixel(sx, sy, 3.0f);
                        }
                    }
                }
            }

            /* Current step highlighted */
            if (circleCurrentStep > 0 && circleCurrentStep <= (int)currentSteps->size()) {
                const auto& step = (*currentSteps)[circleCurrentStep - 1];

                if (currentCircle == CIRCLE_BRESENHAM) {
                    /* Highlight all 8 symmetric points at current step */
                    for (size_t s = 0; s < step.symmetricPoints.size(); s++) {
                        float sx = cs.worldToScreenX(step.symmetricPoints[s].x);
                        float sy = cs.worldToScreenY(step.symmetricPoints[s].y);
                        if (s == 0) {
                            glColor3f(Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);
                        } else {
                            glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
                        }
                        if (useCells) {
                            Renderer::drawPixelSquare(sx, sy, cellSize - 2);
                        } else {
                            Renderer::drawPixel(sx, sy, 7.0f);
                        }
                    }
                } else {
                    for (const auto& pt : step.plottedPoints) {
                        float sx = cs.worldToScreenX(pt.x);
                        float sy = cs.worldToScreenY(pt.y);
                        glColor3f(Colors::YELLOW_R, Colors::YELLOW_G, Colors::YELLOW_B);
                        Renderer::drawPixel(sx, sy, 7.0f);
                    }
                }
            }
        }

        /* Draw center marker */
        float cx = cs.worldToScreenX(circleCenter.x);
        float cy = cs.worldToScreenY(circleCenter.y);
        glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
        Renderer::drawPixel(cx, cy, 6.0f);

        glDisable(GL_SCISSOR_TEST);

        /* Right info panel */
        float panelX = (float)(cs.plotAreaX + cs.plotAreaW + 5);
        float panelW = windowWidth - panelX;
        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(panelX, (float)cs.plotAreaY, panelW, (float)cs.plotAreaH);

        float infoY = cs.plotAreaY + 20.0f;

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        std::string circleName = (currentCircle == CIRCLE_DDA) ?
            "DDA Circle" : "Bresenham Circle";
        Renderer::drawText(panelX + 10, infoY, circleName);
        infoY += 22;

        /* Description */
        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        std::string cDesc = (currentCircle == CIRCLE_DDA) ?
            ddaCircle.getDescription() : bresenhamCircle.getDescription();
        std::istringstream descStream2(cDesc);
        std::string cword;
        std::string cline;
        while (descStream2 >> cword) {
            if (cline.length() + cword.length() + 1 > 42) {
                Renderer::drawSmallText(panelX + 10, infoY, cline);
                infoY += 14;
                cline = cword;
            } else {
                if (!cline.empty()) cline += " ";
                cline += cword;
            }
        }
        if (!cline.empty()) {
            Renderer::drawSmallText(panelX + 10, infoY, cline);
            infoY += 14;
        }
        infoY += 10;

        /* Parameters */
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawSmallText(panelX + 10, infoY,
            "Center: " + circleCenter.toPixelString());
        infoY += 16;
        std::ostringstream rLabel;
        rLabel << "Radius: " << circleRadius;
        Renderer::drawSmallText(panelX + 10, infoY, rLabel.str());
        infoY += 20;

        /* Step info */
        if (currentSteps && !currentSteps->empty()) {
            glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
            std::ostringstream stepLabel;
            stepLabel << "Step " << circleCurrentStep << " / " << currentSteps->size();
            Renderer::drawText(panelX + 10, infoY, stepLabel.str());
            infoY += 22;

            if (circleCurrentStep > 0 && circleCurrentStep <= (int)currentSteps->size()) {
                const auto& step = (*currentSteps)[circleCurrentStep - 1];
                glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);

                std::istringstream iss(step.explanation);
                std::string line;
                while (std::getline(iss, line) && infoY < cs.plotAreaY + cs.plotAreaH - 10) {
                    Renderer::drawSmallText(panelX + 10, infoY, line);
                    infoY += 14;
                }
            }
        }

        /* Navigation buttons */
        int navY = cs.plotAreaY + cs.plotAreaH + 10;
        buttons.push_back(Button(10, navY, 80, 30, "< Previous", 310));
        buttons.push_back(Button(95, navY, 70, 30, "Next >", 311));
        buttons.push_back(Button(170, navY, 60, 30, "Play", 312));
        buttons.push_back(Button(235, navY, 60, 30, "Pause", 313));
        buttons.push_back(Button(300, navY, 60, 30, "Reset", 314));
        buttons.push_back(Button(380, navY, 80, 30, "Run Algo", 315));

        buttons[buttons.size() - 5].active = (circleAnimState == ANIM_PLAYING);
        buttons[buttons.size() - 4].active = (circleAnimState == ANIM_PAUSED);

        for (size_t i = buttons.size() - 6; i < buttons.size(); i++) {
            Renderer::drawButton(buttons[i]);
        }
    }

    /* ============================================================
     * QUIZ MODE
     * ============================================================ */
    void renderQuiz() {
        /* Semi-transparent overlay */
        glColor4f(0, 0, 0, 0.7f);
        Renderer::drawRect(0, 0, (float)windowWidth, (float)windowHeight);

        float pw = 450, ph = 300;
        float px = (windowWidth - pw) / 2;
        float py = (windowHeight - ph) / 2;

        glColor3f(Colors::PANEL_R, Colors::PANEL_G, Colors::PANEL_B);
        Renderer::drawRect(px, py, pw, ph);
        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawRectOutline(px, py, pw, ph);

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawText(px + 20, py + 30, "Quiz: What is the next pixel?");

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(px + 20, py + 60, quizQuestion);

        for (int i = 0; i < 3; i++) {
            Button optBtn((int)px + 20, (int)py + 90 + i * 45, 400, 35,
                         quizOptions[i], 250 + i);

            if (quizAnswered) {
                if (i == quizCorrectAnswer) {
                    optBtn.active = true;
                } else if (i == quizSelectedAnswer && i != quizCorrectAnswer) {
                    // Wrong answer - will be drawn with red border
                }
            }
            Renderer::drawButton(optBtn);
            buttons.push_back(optBtn);

            if (quizAnswered && i == quizSelectedAnswer && i != quizCorrectAnswer) {
                glColor3f(Colors::RED_R, Colors::RED_G, Colors::RED_B);
                Renderer::drawRectOutline((float)optBtn.x, (float)optBtn.y,
                                         (float)optBtn.w, (float)optBtn.h);
            }
        }

        if (quizAnswered) {
            glColor3f(quizSelectedAnswer == quizCorrectAnswer ?
                Colors::GREEN_R : Colors::RED_R,
                quizSelectedAnswer == quizCorrectAnswer ?
                Colors::GREEN_G : Colors::RED_G,
                quizSelectedAnswer == quizCorrectAnswer ?
                Colors::GREEN_B : Colors::RED_B);
            Renderer::drawText(px + 20, py + ph - 30,
                quizSelectedAnswer == quizCorrectAnswer ?
                "Correct!" : "Incorrect. The highlighted answer is correct.");
        }

        buttons.push_back(Button((int)px + (int)pw - 80, (int)py + (int)ph - 40,
                                 60, 28, "Close", 260));
        Renderer::drawButton(buttons.back());
    }

    /* ============================================================
     * ABOUT / HELP
     * ============================================================ */
    void renderAboutHelp() {
        buttons.clear();

        /* Header */
        glColor3f(Colors::HEADER_R, Colors::HEADER_G, Colors::HEADER_B);
        Renderer::drawRect(0, 0, (float)windowWidth, 55);

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(10, 22, "GLIDE");
        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(70, 22, "About & Help");

        buttons.push_back(Button(windowWidth - 90, 10, 80, 30, "< Back", 100));
        Renderer::drawButton(buttons.back());

        float y = 80;

        glColor3f(Colors::ACCENT_R, Colors::ACCENT_G, Colors::ACCENT_B);
        Renderer::drawLargeText(30, y, "GLIDE - Graphics Learning & Interactive Demonstration Environment");
        y += 30;

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        Renderer::drawText(30, y,
            "An educational OpenGL application demonstrating classical computer graphics algorithms.");
        y += 30;

        glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
        Renderer::drawText(30, y, "Algorithms Implemented:");
        y += 20;

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        std::string algos[] = {
            "1. DDA Line Drawing - Incremental calculation using slope",
            "2. Bresenham Line Drawing - Integer arithmetic with decision variable",
            "3. DDA Circle Drawing - Parametric increments with dx=-y*dtheta, dy=x*dtheta",
            "4. Bresenham Circle Drawing - Integer decision variable with 8-way symmetry"
        };
        for (const auto& a : algos) {
            Renderer::drawSmallText(50, y, a);
            y += 18;
        }
        y += 15;

        glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
        Renderer::drawText(30, y, "Keyboard Controls:");
        y += 20;

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        std::string controls[] = {
            "1-5          Menu navigation (main menu)",
            "+/-          Zoom in/out",
            "Arrow keys   Pan the view",
            "R            Reset view",
            "N            Next step",
            "P            Previous step",
            "Space        Play/Pause animation",
            "S            Stop animation / Reset steps",
            "G            Toggle pixel grid",
            "[ / ]        Decrease/increase sample count",
            "Enter        Plot function / Run algorithm",
            "Escape       Back to main menu / Close popup",
            "Q            Toggle quiz mode"
        };
        for (const auto& c : controls) {
            Renderer::drawSmallText(50, y, c);
            y += 16;
        }
        y += 15;

        glColor3f(Colors::ACCENT2_R, Colors::ACCENT2_G, Colors::ACCENT2_B);
        Renderer::drawText(30, y, "Mouse Controls:");
        y += 20;

        glColor3f(Colors::TEXT_R, Colors::TEXT_G, Colors::TEXT_B);
        std::string mouseControls[] = {
            "Left click       Select points / Click buttons",
            "Scroll           Zoom in/out",
            "Click input      Activate expression input field"
        };
        for (const auto& m : mouseControls) {
            Renderer::drawSmallText(50, y, m);
            y += 16;
        }
        y += 15;

        glColor3f(Colors::DIM_R, Colors::DIM_G, Colors::DIM_B);
        Renderer::drawSmallText(30, y,
            "Algorithm implementations are based on the algorithms taught in the provided course notes.");
    }

    /* ============================================================
     * KEYBOARD INPUT
     * ============================================================ */
    void keyboard(unsigned char key, int /*mx*/, int /*my*/) {
        /* Handle text input for graph plotter */
        if (currentState == STATE_GRAPH_PLOTTER && inputActive) {
            if (key == 13 || key == 10) { // Enter
                plotGraph();
                inputActive = false;
            } else if (key == 27) { // Escape
                inputActive = false;
            } else if (key == 8 || key == 127) { // Backspace/Delete
                if (!inputExpression.empty()) {
                    inputExpression.pop_back();
                }
            } else if (key >= 32 && key < 127) {
                inputExpression += static_cast<char>(key);
            }
            glutPostRedisplay();
            return;
        }

        switch (key) {
            case 27: // Escape
                if (showHowItWorks) {
                    showHowItWorks = false;
                } else if (quizActive) {
                    quizActive = false;
                } else {
                    currentState = STATE_MAIN_MENU;
                    stopAnimations();
                }
                break;

            /* Main menu shortcuts */
            case '1':
                if (currentState == STATE_MAIN_MENU) {
                    currentState = STATE_GRAPH_PLOTTER;
                }
                break;
            case '2':
                if (currentState == STATE_MAIN_MENU) {
                    currentState = STATE_ALGORITHM_EXPLORER;
                    runCurrentLineAlgorithm();
                }
                break;
            case '3':
                if (currentState == STATE_MAIN_MENU) {
                    currentState = STATE_CIRCLE_EXPLORER;
                    runCurrentCircleAlgorithm();
                }
                break;
            case '4':
                if (currentState == STATE_MAIN_MENU) {
                    currentState = STATE_ABOUT_HELP;
                }
                break;
            case '5':
                if (currentState == STATE_MAIN_MENU) {
                    exit(0);
                }
                break;

            /* Zoom */
            case '+': case '=':
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().zoomIn();
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.zoomIn();
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.zoomIn();
                }
                break;
            case '-': case '_':
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().zoomOut();
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.zoomOut();
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.zoomOut();
                }
                break;

            /* Reset view */
            case 'r': case 'R':
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().reset();
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.reset();
                    algoCoordSystem.zoom = 1.5f;
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.reset();
                    circleCoordSystem.zoom = 1.5f;
                }
                break;

            /* Step navigation */
            case 'n': case 'N':
                nextStep();
                break;
            case 'p': case 'P':
                prevStep();
                break;

            /* Play/Pause */
            case ' ':
                togglePlayPause();
                break;

            /* Stop/Reset animation */
            case 's': case 'S':
                resetAnimation();
                break;

            /* Toggle pixel grid */
            case 'g': case 'G':
                showPixelGrid = !showPixelGrid;
                break;

            /* Enter: execute */
            case 13: case 10:
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    runCurrentLineAlgorithm();
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    runCurrentCircleAlgorithm();
                }
                break;

            /* Sample count */
            case '[':
                plotter.setSampleCount(plotter.getSampleCount() - 50);
                if (plotter.graphExists()) plotter.computeGraph();
                break;
            case ']':
                plotter.setSampleCount(plotter.getSampleCount() + 50);
                if (plotter.graphExists()) plotter.computeGraph();
                break;

            /* Quiz toggle */
            case 'q': case 'Q':
                if (currentState == STATE_ALGORITHM_EXPLORER) {
                    generateQuiz();
                }
                break;
        }

        glutPostRedisplay();
    }

    /* Special keys (arrow keys, etc.) */
    void specialKey(int key, int /*mx*/, int /*my*/) {
        float panAmount = 30.0f;

        /*
         * Arrow key panning:
         * LEFT  → pan(-panAmount, 0) → moves left
         * RIGHT → pan(panAmount, 0)  → moves right
         * UP    → pan(0, panAmount)  → moves up
         * DOWN  → pan(0, -panAmount) → moves down
         */
        switch (key) {
            case GLUT_KEY_LEFT:
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().pan(-panAmount, 0);
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.pan(-panAmount, 0);
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.pan(-panAmount, 0);
                }
                break;
            case GLUT_KEY_RIGHT:
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().pan(panAmount, 0);
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.pan(panAmount, 0);
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.pan(panAmount, 0);
                }
                break;
            case GLUT_KEY_UP:
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().pan(0, panAmount);
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.pan(0, panAmount);
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.pan(0, panAmount);
                }
                break;
            case GLUT_KEY_DOWN:
                if (currentState == STATE_GRAPH_PLOTTER) {
                    plotter.getCoordSystem().pan(0, -panAmount);
                    if (plotter.graphExists()) plotter.computeGraph();
                } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                    algoCoordSystem.pan(0, -panAmount);
                } else if (currentState == STATE_CIRCLE_EXPLORER) {
                    circleCoordSystem.pan(0, -panAmount);
                }
                break;
        }

        glutPostRedisplay();
    }

    /* ============================================================
     * MOUSE INPUT
     * ============================================================ */
    void mouseClick(int button, int state, int mx, int my) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            /* Check speed slider (in Algorithm Explorer or Circle Explorer toolbar) */
            float tx, ty, tw, th;
            if (getSliderTrack(tx, ty, tw, th)) {
                if (mx >= tx - 10 && mx <= tx + tw + 10 &&
                    my >= ty - 8 && my <= ty + th + 8) {
                    draggingSlider = true;
                    updateSliderFromMouse(mx);
                    glutPostRedisplay();
                    return;
                }
            }

            /* Check all buttons */
            for (auto& btn : buttons) {
                if (btn.contains(mx, my)) {
                    handleButtonClick(btn.id);
                    glutPostRedisplay();
                    return;
                }
            }

            /* Check if clicking in plot area for point selection */
            if (currentState == STATE_ALGORITHM_EXPLORER) {
                CoordinateSystem& cs = algoCoordSystem;
                if (splitView) {
                    /* In split view, user can click in either the left (DDA) or right (Bresenham) plot */
                    int splitPlotH = windowHeight - 340;
                    if (splitPlotH < 220) splitPlotH = 220;
                    int halfW = (windowWidth - 30) / 2;
                    int leftPlotX = 10;
                    int rightPlotX = leftPlotX + halfW + 10;
                    float worldX = 0, worldY = 0;
                    bool clickedInPlot = false;

                    if (mx >= leftPlotX && mx <= leftPlotX + halfW &&
                        my >= cs.plotAreaY && my <= cs.plotAreaY + splitPlotH) {
                        CoordinateSystem leftCS = cs;
                        leftCS.plotAreaX = leftPlotX;
                        leftCS.plotAreaW = halfW;
                        leftCS.plotAreaH = splitPlotH;
                        worldX = leftCS.screenToWorldX((float)mx);
                        worldY = leftCS.screenToWorldY((float)my);
                        clickedInPlot = true;
                    } else if (mx >= rightPlotX && mx <= rightPlotX + halfW &&
                               my >= cs.plotAreaY && my <= cs.plotAreaY + splitPlotH) {
                        CoordinateSystem rightCS = cs;
                        rightCS.plotAreaX = rightPlotX;
                        rightCS.plotAreaW = halfW;
                        rightCS.plotAreaH = splitPlotH;
                        worldX = rightCS.screenToWorldX((float)mx);
                        worldY = rightCS.screenToWorldY((float)my);
                        clickedInPlot = true;
                    }

                    if (clickedInPlot) {
                        Point clicked(roundf(worldX), roundf(worldY));
                        if (settingStartPoint) {
                            algoStartPoint = clicked;
                            settingStartPoint = false;
                        } else {
                            algoEndPoint = clicked;
                            settingStartPoint = true;
                            runCurrentLineAlgorithm();
                        }
                        glutPostRedisplay();
                    }
                } else {
                    /* Single view plot click */
                    if (mx >= cs.plotAreaX && mx <= cs.plotAreaX + cs.plotAreaW &&
                        my >= cs.plotAreaY && my <= cs.plotAreaY + cs.plotAreaH) {
                        float worldX = cs.screenToWorldX((float)mx);
                        float worldY = cs.screenToWorldY((float)my);
                        Point clicked(roundf(worldX), roundf(worldY));

                        if (settingStartPoint) {
                            algoStartPoint = clicked;
                            settingStartPoint = false;
                        } else {
                            algoEndPoint = clicked;
                            settingStartPoint = true;
                            runCurrentLineAlgorithm();
                        }
                        glutPostRedisplay();
                    }
                }
            }
            else if (currentState == STATE_CIRCLE_EXPLORER) {
                CoordinateSystem& cs = circleCoordSystem;
                if (mx >= cs.plotAreaX && mx <= cs.plotAreaX + cs.plotAreaW &&
                    my >= cs.plotAreaY && my <= cs.plotAreaY + cs.plotAreaH) {
                    float worldX = cs.screenToWorldX((float)mx);
                    float worldY = cs.screenToWorldY((float)my);
                    circleCenter = Point(roundf(worldX), roundf(worldY));
                    runCurrentCircleAlgorithm();
                    glutPostRedisplay();
                }
            }
            else if (currentState == STATE_GRAPH_PLOTTER) {
                /* Check if clicking on input field */
                if (mx >= 40 && mx <= 390 && my >= 60 && my <= 88) {
                    inputActive = true;
                } else {
                    inputActive = false;
                }
                glutPostRedisplay();
            }
        }

        /* Scroll wheel for zoom */
        if (button == 3 || button == 4) { // Scroll up / down
            if (currentState == STATE_GRAPH_PLOTTER) {
                if (button == 3) plotter.getCoordSystem().zoomIn();
                else plotter.getCoordSystem().zoomOut();
                if (plotter.graphExists()) plotter.computeGraph();
            } else if (currentState == STATE_ALGORITHM_EXPLORER) {
                if (button == 3) algoCoordSystem.zoomIn();
                else algoCoordSystem.zoomOut();
            } else if (currentState == STATE_CIRCLE_EXPLORER) {
                if (button == 3) circleCoordSystem.zoomIn();
                else circleCoordSystem.zoomOut();
            }
            glutPostRedisplay();
        }

        /* Mouse up: stop slider dragging */
        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            draggingSlider = false;
        }
    }

    /* Called when mouse moves with button held (active drag) */
    void mouseDrag(int mx, int /*my*/) {
        if (draggingSlider) {
            updateSliderFromMouse(mx);
            glutPostRedisplay();
        }
    }

    void mouseMove(int mx, int my) {
        hoveredButton = -1;
        for (auto& btn : buttons) {
            btn.hovered = btn.contains(mx, my);
            if (btn.hovered) hoveredButton = btn.id;
        }
        glutPostRedisplay();
    }

    /* ============================================================
     * BUTTON CLICK HANDLER
     * ============================================================ */
    void handleButtonClick(int id) {
        switch (id) {
            /* Main menu */
            case 1: currentState = STATE_GRAPH_PLOTTER; break;
            case 2:
                currentState = STATE_ALGORITHM_EXPLORER;
                runCurrentLineAlgorithm();
                break;
            case 3:
                currentState = STATE_CIRCLE_EXPLORER;
                runCurrentCircleAlgorithm();
                break;
            case 4: currentState = STATE_ABOUT_HELP; break;
            case 5: exit(0); break;

            /* Common back button */
            case 100:
                currentState = STATE_MAIN_MENU;
                stopAnimations();
                break;

            /* Graph plotter controls */
            case 101: plotGraph(); break;
            case 102:
                plotter.setAlgorithm(true);  // DDA
                if (plotter.graphExists()) plotGraph();
                break;
            case 103:
                plotter.setAlgorithm(false); // Bresenham
                if (plotter.graphExists()) plotGraph();
                break;
            case 104:
                graphViewMode = (graphViewMode == GRAPH_STEP_BY_STEP) ?
                    GRAPH_NORMAL : GRAPH_STEP_BY_STEP;
                graphAnimSegment = 0;
                graphAnimStep = 0;
                break;
            case 105:
                graphViewMode = (graphViewMode == GRAPH_EXPLAIN) ?
                    GRAPH_NORMAL : GRAPH_EXPLAIN;
                graphAnimSegment = 0;
                graphAnimStep = 0;
                break;
            case 106: showHowItWorks = !showHowItWorks; break;
            case 110: graphPrevStep(); break;
            case 111: graphNextStep(); break;
            case 112:
                graphAnimSegment = 0;
                graphAnimStep = 0;
                break;

            /* Preset functions */
            case 120: inputExpression = "x*x"; plotGraph(); break;
            case 121: inputExpression = "sin(x)"; plotGraph(); break;
            case 122: inputExpression = "cos(x)"; plotGraph(); break;
            case 123: inputExpression = "2*x+1"; plotGraph(); break;
            case 124: inputExpression = "x^3"; plotGraph(); break;
            case 125: inputExpression = "sqrt(abs(x))"; plotGraph(); break;

            case 199: showHowItWorks = false; break;

            /* Algorithm explorer controls */
            case 201:
                splitView = false;
                currentAlgo = ALGO_DDA_LINE;
                runCurrentLineAlgorithm();
                break;
            case 202:
                splitView = false;
                currentAlgo = ALGO_BRESENHAM_LINE;
                runCurrentLineAlgorithm();
                break;
            case 203: showPixelGrid = !showPixelGrid; break;
            case 206:
                splitView = !splitView;
                runCurrentLineAlgorithm();
                break;

            case 210: prevStep(); break;
            case 211: nextStep(); break;
            case 212: startPlay(); break;
            case 213: pausePlay(); break;
            case 214: resetAnimation(); break;
            case 215: runCurrentLineAlgorithm(); break;
            case 216: generateQuiz(); break;

            /* Quiz options */
            case 250: case 251: case 252:
                if (!quizAnswered) {
                    quizSelectedAnswer = id - 250;
                    quizAnswered = true;
                }
                break;
            case 260: quizActive = false; break;

            /* Circle explorer controls */
            case 301:
                currentCircle = CIRCLE_DDA;
                runCurrentCircleAlgorithm();
                break;
            case 302:
                currentCircle = CIRCLE_BRESENHAM;
                runCurrentCircleAlgorithm();
                break;
            case 303:
                circleRadius += 1.0f;
                if (circleRadius > 50) circleRadius = 50;
                runCurrentCircleAlgorithm();
                break;
            case 304:
                circleRadius -= 1.0f;
                if (circleRadius < 1) circleRadius = 1;
                runCurrentCircleAlgorithm();
                break;
            case 305: showPixelGrid = !showPixelGrid; break;

            case 310: circlePrevStep(); break;
            case 311: circleNextStep(); break;
            case 312: startCirclePlay(); break;
            case 313: pauseCirclePlay(); break;
            case 314: resetCircleAnimation(); break;
            case 315: runCurrentCircleAlgorithm(); break;
        }
    }

    /* ============================================================
     * GRAPH OPERATIONS
     * ============================================================ */
    void plotGraph() {
        plotter.setFunction(inputExpression);
        if (plotter.getFunction().isValid()) {
            plotter.computeGraph();
            graphAnimSegment = 0;
            graphAnimStep = 0;
        }
    }

    void graphNextStep() {
        if (!plotter.graphExists()) return;
        const auto& segs = plotter.getSegments();
        if (segs.empty()) return;

        if (graphAnimSegment < (int)segs.size()) {
            graphAnimStep++;
            if (graphAnimStep >= (int)segs[graphAnimSegment].steps.size()) {
                graphAnimSegment++;
                graphAnimStep = 0;
            }
        }
    }

    void graphPrevStep() {
        if (!plotter.graphExists()) return;
        const auto& segs = plotter.getSegments();
        if (segs.empty()) return;

        if (graphAnimStep > 0) {
            graphAnimStep--;
        } else if (graphAnimSegment > 0) {
            graphAnimSegment--;
            graphAnimStep = (int)segs[graphAnimSegment].steps.size() - 1;
        }
    }

    /* ============================================================
     * ALGORITHM OPERATIONS
     * ============================================================ */
    void runCurrentLineAlgorithm() {
        algoCurrentStep = 0;
        algoCurrentStep2 = 0;
        algoAnimState = ANIM_STOPPED;

        if (splitView) {
            /* In split view, run BOTH algorithms for comparison */
            ddaLine.setEndpoints(algoStartPoint, algoEndPoint);
            ddaLine.execute();
            bresenhamLine.setEndpoints(algoStartPoint, algoEndPoint);
            bresenhamLine.execute();
        } else if (currentAlgo == ALGO_DDA_LINE) {
            ddaLine.setEndpoints(algoStartPoint, algoEndPoint);
            ddaLine.execute();
        } else {
            bresenhamLine.setEndpoints(algoStartPoint, algoEndPoint);
            bresenhamLine.execute();
        }
    }

    void runCurrentCircleAlgorithm() {
        circleCurrentStep = 0;
        circleAnimState = ANIM_STOPPED;

        if (currentCircle == CIRCLE_DDA) {
            ddaCircle.setParameters(circleCenter, circleRadius);
            ddaCircle.execute();
        } else {
            bresenhamCircle.setParameters(circleCenter, circleRadius);
            bresenhamCircle.execute();
        }
    }

    /* ============================================================
     * ANIMATION CONTROLS
     * ============================================================ */
    void nextStep() {
        if (currentState == STATE_ALGORITHM_EXPLORER) {
            if (splitView) {
                /* Advance both algorithms together */
                if (algoCurrentStep < (int)ddaLine.getSteps().size()) {
                    algoCurrentStep++;
                }
                if (algoCurrentStep2 < (int)bresenhamLine.getSteps().size()) {
                    algoCurrentStep2++;
                }
            } else {
                const auto* steps = (currentAlgo == ALGO_DDA_LINE) ?
                    &ddaLine.getSteps() : &bresenhamLine.getSteps();
                if (algoCurrentStep < (int)steps->size()) {
                    algoCurrentStep++;
                }
            }
        }
    }

    void prevStep() {
        if (currentState == STATE_ALGORITHM_EXPLORER) {
            if (splitView) {
                if (algoCurrentStep > 0) algoCurrentStep--;
                if (algoCurrentStep2 > 0) algoCurrentStep2--;
            } else {
                if (algoCurrentStep > 0) {
                    algoCurrentStep--;
                }
            }
        }
    }

    void circleNextStep() {
        const auto* steps = (currentCircle == CIRCLE_DDA) ?
            &ddaCircle.getSteps() : &bresenhamCircle.getSteps();
        if (circleCurrentStep < (int)steps->size()) {
            circleCurrentStep++;
        }
    }

    void circlePrevStep() {
        if (circleCurrentStep > 0) {
            circleCurrentStep--;
        }
    }

    void startPlay() {
        algoAnimState = ANIM_PLAYING;
    }

    void pausePlay() {
        algoAnimState = ANIM_PAUSED;
    }

    void startCirclePlay() {
        circleAnimState = ANIM_PLAYING;
    }

    void pauseCirclePlay() {
        circleAnimState = ANIM_PAUSED;
    }

    void togglePlayPause() {
        if (currentState == STATE_ALGORITHM_EXPLORER) {
            if (algoAnimState == ANIM_PLAYING) {
                algoAnimState = ANIM_PAUSED;
            } else {
                algoAnimState = ANIM_PLAYING;
            }
        } else if (currentState == STATE_CIRCLE_EXPLORER) {
            if (circleAnimState == ANIM_PLAYING) {
                circleAnimState = ANIM_PAUSED;
            } else {
                circleAnimState = ANIM_PLAYING;
            }
        }
    }

    void resetAnimation() {
        if (currentState == STATE_ALGORITHM_EXPLORER) {
            algoCurrentStep = 0;
            algoCurrentStep2 = 0;
            algoAnimState = ANIM_STOPPED;
        }
    }

    void resetCircleAnimation() {
        circleCurrentStep = 0;
        circleAnimState = ANIM_STOPPED;
    }

    void stopAnimations() {
        algoAnimState = ANIM_STOPPED;
        circleAnimState = ANIM_STOPPED;
    }

    /* Timer callback for animation */
    void timerTick() {
        if (currentState == STATE_ALGORITHM_EXPLORER && algoAnimState == ANIM_PLAYING) {
            if (splitView) {
                /* Advance both algorithms in split view */
                bool ddaDone = algoCurrentStep >= (int)ddaLine.getSteps().size();
                bool bresDone = algoCurrentStep2 >= (int)bresenhamLine.getSteps().size();
                if (!ddaDone) algoCurrentStep++;
                if (!bresDone) algoCurrentStep2++;
                if (ddaDone && bresDone) {
                    algoAnimState = ANIM_PAUSED;
                }
            } else {
                const auto* steps = (currentAlgo == ALGO_DDA_LINE) ?
                    &ddaLine.getSteps() : &bresenhamLine.getSteps();
                if (algoCurrentStep < (int)steps->size()) {
                    algoCurrentStep++;
                } else {
                    algoAnimState = ANIM_PAUSED;
                }
            }
            glutPostRedisplay();
        }

        if (currentState == STATE_CIRCLE_EXPLORER && circleAnimState == ANIM_PLAYING) {
            const auto* steps = (currentCircle == CIRCLE_DDA) ?
                &ddaCircle.getSteps() : &bresenhamCircle.getSteps();
            if (circleCurrentStep < (int)steps->size()) {
                circleCurrentStep++;
            } else {
                circleAnimState = ANIM_PAUSED;
            }
            glutPostRedisplay();
        }
    }

    int getAnimSpeed() const { return animSpeed; }

    /* Get slider track bounds for current state */
    bool getSliderTrack(float& trackX, float& trackY, float& trackW, float& trackH) const {
        if (currentState == STATE_ALGORITHM_EXPLORER) {
            trackX = 505.0f;
            trackY = 68.0f;
            trackW = 130.0f;
            trackH = 14.0f;
            return true;
        } else if (currentState == STATE_CIRCLE_EXPLORER) {
            trackX = 525.0f;
            trackY = 68.0f;
            trackW = 130.0f;
            trackH = 14.0f;
            return true;
        }
        return false;
    }

    /* Update animation speed from mouse X position on slider */
    void updateSliderFromMouse(int mx) {
        float tx, ty, tw, th;
        if (!getSliderTrack(tx, ty, tw, th)) return;
        float ratio = ((float)mx - tx) / tw;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        // Left is Slow (2000ms), Right is Fast (100ms)
        animSpeed = (int)(2000.0f - ratio * 1900.0f);
        // Snap to nearest 50ms
        animSpeed = ((animSpeed + 25) / 50) * 50;
        if (animSpeed < 100) animSpeed = 100;
        if (animSpeed > 2000) animSpeed = 2000;
    }

    /* ============================================================
     * QUIZ GENERATION
     * ============================================================ */
    void generateQuiz() {
        const std::vector<AlgorithmStep>* steps = nullptr;
        if (currentAlgo == ALGO_DDA_LINE) {
            steps = &ddaLine.getSteps();
        } else {
            steps = &bresenhamLine.getSteps();
        }

        if (!steps || steps->size() < 3) return;

        /* Pick a random step */
        int stepIdx = algoCurrentStep;
        if (stepIdx < 1) stepIdx = 1;
        if (stepIdx >= (int)steps->size() - 1) stepIdx = (int)steps->size() - 2;

        const auto& currentStep = (*steps)[stepIdx];
        const auto& nextStepData = (*steps)[stepIdx + 1];

        quizQuestion = "Current pixel: " + currentStep.plottedPoints[0].toPixelString();
        quizQuestion += "\nWhat is the next pixel?";

        /* Correct answer */
        Point correct = nextStepData.plottedPoints[0];
        quizCorrectAnswer = rand() % 3;

        /* Generate wrong answers by offsetting */
        Point wrong1(correct.x + 1, correct.y);
        Point wrong2(correct.x, correct.y + 1);

        int idx = 0;
        for (int i = 0; i < 3; i++) {
            if (i == quizCorrectAnswer) {
                quizOptions[i] = "A) " + correct.toPixelString();
                quizOptions[i][0] = 'A' + i;
            } else {
                if (idx == 0) {
                    quizOptions[i] = std::string(1, 'A' + i) + ") " + wrong1.toPixelString();
                } else {
                    quizOptions[i] = std::string(1, 'A' + i) + ") " + wrong2.toPixelString();
                }
                idx++;
            }
        }

        quizSelectedAnswer = -1;
        quizAnswered = false;
        quizActive = true;
    }
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
    if (g_app) {
        g_app->mouseDrag(x, y);
        g_app->mouseMove(x, y);
    }
}
void passiveMotionCallback(int x, int y) {
    if (g_app) g_app->mouseMove(x, y);
}
void timerCallback(int /*value*/) {
    if (g_app) {
        g_app->timerTick();
        glutTimerFunc(g_app->getAnimSpeed(), timerCallback, 0);
    }
}

#endif // GLIDE_APPLICATION_H
