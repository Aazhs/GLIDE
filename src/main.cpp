/*
 * main.cpp
 * --------
 * Entry point for GLIDE: Graphics Learning & Interactive Demonstration Environment.
 *
 * This file sets up the GLUT window and connects the Application class
 * to GLUT's callback system. GLUT requires C-style function pointers,
 * so we use a global Application pointer (g_app) as a bridge.
 *
 * The Application class handles all the actual logic and rendering.
 *
 * Algorithm implementations are based on the algorithms taught in
 * the provided course notes.
 */

#include "Application.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) {
    /* Seed random number generator (used for quiz) */
    srand(static_cast<unsigned>(time(nullptr)));

    /* Create the application */
    Application app;
    g_app = &app;

    /* Initialize GLUT and create the window */
    app.init(argc, argv);

    /* Register GLUT callbacks.
     * These C-style callback functions forward to the Application class.
     * See Application.h for the callback definitions. */
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutSpecialFunc(specialCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutPassiveMotionFunc(passiveMotionCallback);

    /* Start the ~60 Hz timer that drives all animations.
     * It re-registers itself to keep running. */
    glutTimerFunc(Application::TICK_MS, timerCallback, 0);

    std::cout << "======================================================" << std::endl;
    std::cout << "  GLIDE - Graphics Learning & Interactive              " << std::endl;
    std::cout << "          Demonstration Environment                    " << std::endl;
    std::cout << "======================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "  Algorithms:" << std::endl;
    std::cout << "    1. DDA Line Drawing" << std::endl;
    std::cout << "    2. Bresenham Line Drawing" << std::endl;
    std::cout << "    3. DDA Circle Drawing" << std::endl;
    std::cout << "    4. Bresenham Circle Drawing" << std::endl;
    std::cout << "    5. Midpoint Circle Drawing" << std::endl;
    std::cout << "    6. Boundary Fill (4- and 8-connected)" << std::endl;
    std::cout << "    7. Flood Fill (4- and 8-connected)" << std::endl;
    std::cout << "    8. Scanline Polygon Fill" << std::endl;
    std::cout << "    6. Boundary Fill (4- and 8-connected)" << std::endl;
    std::cout << "    7. Flood Fill (4- and 8-connected)" << std::endl;
    std::cout << "    8. Scanline Polygon Fill" << std::endl;
    std::cout << std::endl;
    std::cout << "  Algorithm implementations are based on the" << std::endl;
    std::cout << "  algorithms taught in the provided course notes." << std::endl;
    std::cout << "======================================================" << std::endl;

    /* Enter the GLUT event loop.
     * This never returns - the program exits when the user
     * closes the window or selects Exit from the menu. */
    glutMainLoop();

    return 0;
}
