/*
 * StepAnimator.h
 * --------------
 * Controls playback through a list of pre-computed algorithm steps.
 *
 * 'revealed' is how many steps are visible:
 *   0          -> nothing drawn yet
 *   k          -> steps 0 .. k-1 drawn, step k-1 is the CURRENT step
 *   total      -> algorithm complete
 *
 * Timing: the application calls tick(dtMs) about 60 times per second
 * with the elapsed milliseconds. The animator accumulates time and
 * advances one step every 'delayMs'. Because the delay is read on every
 * tick, moving the speed slider takes effect immediately.
 */

#ifndef GLIDE_STEP_ANIMATOR_H
#define GLIDE_STEP_ANIMATOR_H

#include <algorithm>
#include <cmath>

/* Settings shared by all screens (speed slider, pixel grid toggle ...) */
struct Settings {
    int stepDelayMs;        // milliseconds between animation steps
    bool showPixelGrid;     // draw pixels as grid cells
    bool showIdeal;         // draw the ideal (mathematical) line / circle

    Settings() : stepDelayMs(500), showPixelGrid(true), showIdeal(true) {}

    /* The speed slider works on a 0..1 scale, mapped exponentially
     * so both slow (2 s) and fast (30 ms) speeds are easy to pick. */
    static constexpr float SLOWEST_MS = 2000.0f;
    static constexpr float FASTEST_MS = 30.0f;

    float speedSliderValue() const {
        return logf(SLOWEST_MS / (float)stepDelayMs) / logf(SLOWEST_MS / FASTEST_MS);
    }
    void setSpeedFromSlider(float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        stepDelayMs = (int)(SLOWEST_MS * powf(FASTEST_MS / SLOWEST_MS, t) + 0.5f);
    }
};

class StepAnimator {
private:
    int revealed;
    int total;
    bool playing;
    int accumMs;

public:
    StepAnimator() : revealed(0), total(0), playing(false), accumMs(0) {}

    /* New data: start from the beginning (optionally playing) */
    void reset(int totalSteps, bool autoplay) {
        total = std::max(0, totalSteps);
        revealed = 0;
        accumMs = 0;
        playing = autoplay && total > 0;
        if (playing) revealed = 1;   // show the first step straight away
    }

    /* New data, but jump straight to the finished result */
    void resetToEnd(int totalSteps) {
        total = std::max(0, totalSteps);
        revealed = total;
        playing = false;
        accumMs = 0;
    }

    /* Data changed size but keep the position where possible */
    void setTotal(int totalSteps) {
        total = std::max(0, totalSteps);
        revealed = std::min(revealed, total);
        if (revealed >= total) playing = false;
    }

    int getRevealed() const { return revealed; }
    int getTotal() const { return total; }
    bool isPlaying() const { return playing; }
    bool isComplete() const { return total > 0 && revealed >= total; }

    /* Index of the current step, or -1 if nothing revealed */
    int currentIndex() const { return revealed > 0 ? revealed - 1 : -1; }

    void next()   { playing = false; if (revealed < total) revealed++; }
    void prev()   { playing = false; if (revealed > 0) revealed--; }
    void toStart(){ playing = false; revealed = 0; }
    void toEnd()  { playing = false; revealed = total; }

    /* Jump so that step 'index' (0-based) is the current one */
    void jumpTo(int index) {
        playing = false;
        revealed = std::max(0, std::min(total, index + 1));
    }

    void play() {
        if (total == 0) return;
        if (revealed >= total) revealed = 0;   // finished: restart from the top
        playing = true;
        accumMs = 0;
        if (revealed == 0) revealed = 1;
    }
    void pause() { playing = false; }
    void togglePlay() { if (playing) pause(); else play(); }

    /* Advance time. Returns true if the visible step changed. */
    bool tick(int dtMs, int delayMs) {
        if (!playing) return false;
        accumMs += dtMs;
        bool changed = false;
        while (accumMs >= delayMs && playing) {
            accumMs -= delayMs;
            if (revealed < total) {
                revealed++;
                changed = true;
            }
            if (revealed >= total) {
                playing = false;
                changed = true;
            }
        }
        return changed;
    }
};

#endif // GLIDE_STEP_ANIMATOR_H
