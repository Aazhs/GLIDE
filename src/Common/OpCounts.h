/*
 * OpCounts.h
 * ----------
 * Counts the arithmetic operations an algorithm performs, so GLIDE can
 * show WHY Bresenham is cheaper than DDA even when they plot the same
 * pixels.
 *
 * Every algorithm records the operations of its setup phase and of each
 * loop iteration. Plotting the 8 mirrored points of a circle is counted
 * too (16 integer additions), since every circle algorithm pays for it.
 *
 * COST MODEL
 * A single number makes the comparison easy to read. Each operation is
 * weighted by its approximate latency in CPU cycles on a simple
 * processor without heavy floating-point hardware (the setting these
 * algorithms were designed for):
 *
 *   integer add / subtract / compare / shift   1
 *   float add / subtract                        3
 *   float multiply                              4
 *   float divide                               12
 *   round float -> int                          6
 *
 * These weights are a teaching model, not a measurement. The explorers
 * also show REAL measured timings from this machine (see Benchmark.h).
 */

#ifndef GLIDE_OP_COUNTS_H
#define GLIDE_OP_COUNTS_H

#include <string>

namespace CostModel {
    const float INT_OP    = 1.0f;
    const float FLOAT_ADD = 3.0f;
    const float FLOAT_MUL = 4.0f;
    const float FLOAT_DIV = 12.0f;
    const float ROUND     = 6.0f;
}

struct OpCounts {
    int intAdd;     // integer add / subtract
    int intShift;   // multiply by a power of two (2*dx, 4*x ...)
    int compare;    // comparisons / branch tests
    int floatAdd;   // floating-point add / subtract
    int floatMul;   // floating-point multiply
    int floatDiv;   // floating-point divide
    int rounds;     // float -> int rounding

    OpCounts() : intAdd(0), intShift(0), compare(0), floatAdd(0),
                 floatMul(0), floatDiv(0), rounds(0) {}

    OpCounts& operator+=(const OpCounts& o) {
        intAdd += o.intAdd;     intShift += o.intShift;  compare += o.compare;
        floatAdd += o.floatAdd; floatMul += o.floatMul;  floatDiv += o.floatDiv;
        rounds += o.rounds;
        return *this;
    }

    int integerOps() const { return intAdd + intShift + compare; }
    int floatOps() const   { return floatAdd + floatMul + floatDiv; }

    /* Weighted cost in model "cycles" */
    float cost() const {
        return CostModel::INT_OP * integerOps() +
               CostModel::FLOAT_ADD * floatAdd +
               CostModel::FLOAT_MUL * floatMul +
               CostModel::FLOAT_DIV * floatDiv +
               CostModel::ROUND * rounds;
    }

    /* Compact description such as "2 round, 2 fadd, 1 cmp" */
    std::string summary() const {
        std::string s;
        auto add = [&](int n, const char* name) {
            if (n == 0) return;
            if (!s.empty()) s += ", ";
            s += std::to_string(n) + " " + name;
        };
        add(floatDiv, "fdiv");
        add(floatMul, "fmul");
        add(floatAdd, "fadd");
        add(rounds, "round");
        add(intAdd, "add");
        add(intShift, "shift");
        add(compare, "cmp");
        return s.empty() ? "none" : s;
    }
};

#endif // GLIDE_OP_COUNTS_H
