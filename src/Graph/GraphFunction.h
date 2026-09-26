/*
 * GraphFunction.h
 * ---------------
 * Simple mathematical expression parser and evaluator.
 *
 * This implements a recursive descent parser that handles:
 *   - Numbers (integers and decimals)
 *   - Variable x
 *   - Operators: + - * / ^
 *   - Parentheses: ( )
 *   - Functions: sin, cos, tan, sqrt, abs
 *
 * GRAMMAR (Backus-Naur Form):
 *   expression  = term (('+' | '-') term)*
 *   term        = power (('*' | '/') power)*
 *   power       = unary ('^' unary)*
 *   unary       = '-' unary | primary
 *   primary     = NUMBER | 'x' | FUNCTION '(' expression ')' | '(' expression ')'
 *   FUNCTION    = 'sin' | 'cos' | 'tan' | 'sqrt' | 'abs'
 *
 * HOW THE PARSER WORKS:
 * The parser reads the expression string character by character.
 * It maintains a current position (pos) in the string.
 * Each grammar rule is a function that reads part of the expression
 * and returns the computed value.
 *
 * Operator precedence is handled by the grammar structure:
 *   Lowest:  + -
 *   Middle:  * /
 *   Higher:  ^
 *   Highest: unary -, functions, parentheses
 *
 * This is NOT a production-grade parser. It is intentionally simple
 * so that it can be understood and explained in a viva.
 */

#ifndef GLIDE_GRAPH_FUNCTION_H
#define GLIDE_GRAPH_FUNCTION_H

#include <string>
#include <cmath>
#include <vector>
#include <stdexcept>
#include "../Common/Point.h"
#include "../Common/MathUtils.h"

class GraphFunction {
private:
    std::string expression;     // The original expression string
    std::string cleanExpr;      // Cleaned expression (whitespace removed)
    size_t pos;                 // Current position in the string during parsing
    float xValue;               // Current value of 'x' during evaluation
    bool valid;                 // Whether the expression was parsed successfully
    std::string errorMessage;   // Error message if parsing failed

    /*
     * TOKENIZER HELPERS
     * These functions read the next token from the expression string.
     */

    /* Skip whitespace characters */
    void skipWhitespace() {
        while (pos < cleanExpr.length() && cleanExpr[pos] == ' ') {
            pos++;
        }
    }

    /* Peek at the current character without consuming it */
    char peek() const {
        if (pos < cleanExpr.length()) return cleanExpr[pos];
        return '\0';
    }

    /* Consume and return the current character */
    char consume() {
        if (pos < cleanExpr.length()) return cleanExpr[pos++];
        return '\0';
    }

    /* Check if current position matches a string (for function names) */
    bool match(const std::string& s) {
        if (pos + s.length() <= cleanExpr.length()) {
            if (cleanExpr.substr(pos, s.length()) == s) {
                pos += s.length();
                return true;
            }
        }
        return false;
    }

    /*
     * RECURSIVE DESCENT PARSER
     * Each function corresponds to one grammar rule.
     */

    /*
     * expression = term (('+' | '-') term)*
     *
     * Handles addition and subtraction (lowest precedence).
     */
    float parseExpression() {
        float result = parseTerm();

        while (peek() == '+' || peek() == '-') {
            char op = consume();
            float right = parseTerm();
            if (op == '+') result += right;
            else           result -= right;
        }
        return result;
    }

    /*
     * term = power (('*' | '/') power)*
     *
     * Handles multiplication and division.
     */
    float parseTerm() {
        float result = parsePower();

        while (peek() == '*' || peek() == '/') {
            char op = consume();
            float right = parsePower();
            if (op == '*') {
                result *= right;
            } else {
                if (right == 0.0f) {
                    result = std::numeric_limits<float>::quiet_NaN();
                } else {
                    result /= right;
                }
            }
        }
        return result;
    }

    /*
     * power = unary ('^' unary)*
     *
     * Handles exponentiation (right-associative).
     */
    float parsePower() {
        float base = parseUnary();

        if (peek() == '^') {
            consume();
            float exponent = parsePower(); // Right-associative: recurse
            return powf(base, exponent);
        }
        return base;
    }

    /*
     * unary = '-' unary | primary
     *
     * Handles unary minus (negation).
     */
    float parseUnary() {
        if (peek() == '-') {
            consume();
            return -parseUnary();
        }
        return parsePrimary();
    }

    /*
     * primary = NUMBER | 'x' | FUNCTION '(' expression ')' | '(' expression ')'
     *
     * Handles numbers, the variable x, function calls, and parenthesized expressions.
     */
    float parsePrimary() {
        skipWhitespace();

        /* Check for 'pi' and 'e' constants */
        if (match("pi")) {
            return MathUtils::PI;
        }

        /* Check for function calls: sin, cos, tan, sqrt, abs */
        if (match("sin")) {
            expect('(');
            float arg = parseExpression();
            expect(')');
            return sinf(arg);
        }
        if (match("cos")) {
            expect('(');
            float arg = parseExpression();
            expect(')');
            return cosf(arg);
        }
        if (match("tan")) {
            expect('(');
            float arg = parseExpression();
            expect(')');
            return tanf(arg);
        }
        if (match("sqrt")) {
            expect('(');
            float arg = parseExpression();
            expect(')');
            if (arg < 0) return std::numeric_limits<float>::quiet_NaN();
            return sqrtf(arg);
        }
        if (match("abs")) {
            expect('(');
            float arg = parseExpression();
            expect(')');
            return fabsf(arg);
        }

        /* Check for variable 'x' */
        if (peek() == 'x' || peek() == 'X') {
            consume();
            return xValue;
        }

        /* Check for parenthesized expression */
        if (peek() == '(') {
            consume();
            float result = parseExpression();
            expect(')');
            return result;
        }

        /* Must be a number */
        return parseNumber();
    }

    /*
     * Parse a numeric literal (integer or decimal).
     */
    float parseNumber() {
        skipWhitespace();
        size_t start = pos;

        /* Read digits before decimal point */
        while (pos < cleanExpr.length() &&
               (isdigit(cleanExpr[pos]) || cleanExpr[pos] == '.')) {
            pos++;
        }

        if (pos == start) {
            throw std::runtime_error(
                "Expected number at position " + std::to_string(pos)
            );
        }

        return std::stof(cleanExpr.substr(start, pos - start));
    }

    /* Expect and consume a specific character */
    void expect(char ch) {
        skipWhitespace();
        if (peek() != ch) {
            throw std::runtime_error(
                "Expected '" + std::string(1, ch) +
                "' at position " + std::to_string(pos)
            );
        }
        consume();
    }

public:
    GraphFunction() : pos(0), xValue(0), valid(false) {}

    GraphFunction(const std::string& expr) : pos(0), xValue(0), valid(false) {
        setExpression(expr);
    }

    /*
     * Set a new expression to evaluate.
     * Validates the expression by trying to evaluate it at x=0.
     */
    void setExpression(const std::string& expr) {
        expression = expr;
        cleanExpr = expr;
        valid = false;
        errorMessage = "";

        /* Quick validation: try evaluating at x=0 */
        try {
            evaluate(0.0f);
            valid = true;
        } catch (const std::exception& e) {
            errorMessage = e.what();
            valid = false;
        }
    }

    /*
     * Evaluate the expression for a given value of x.
     *
     * Returns the computed y value.
     * Throws std::runtime_error if the expression is malformed.
     */
    float evaluate(float x) {
        pos = 0;
        xValue = x;
        float result = parseExpression();
        return result;
    }

    /*
     * Generate a set of (x, y) sample points by evaluating the function
     * at evenly-spaced x values within a given range.
     *
     * Points with invalid y values (NaN, infinity) are skipped.
     */
    std::vector<Point> generatePoints(float xMin, float xMax, int sampleCount) {
        std::vector<Point> points;

        if (sampleCount < 2) sampleCount = 2;
        float step = (xMax - xMin) / static_cast<float>(sampleCount - 1);

        for (int i = 0; i < sampleCount; i++) {
            float x = xMin + step * i;
            try {
                float y = evaluate(x);

                /* Skip invalid values - don't try to plot NaN or infinity */
                if (MathUtils::isValidFloat(y) && fabsf(y) < 10000.0f) {
                    points.push_back(Point(x, y));
                } else {
                    /* Insert an invalid sentinel to break the line */
                    points.push_back(Point(x, std::numeric_limits<float>::quiet_NaN()));
                }
            } catch (...) {
                points.push_back(Point(x, std::numeric_limits<float>::quiet_NaN()));
            }
        }

        return points;
    }

    bool isValid() const { return valid; }
    std::string getExpression() const { return expression; }
    std::string getError() const { return errorMessage; }
};

#endif // GLIDE_GRAPH_FUNCTION_H
