#ifndef GLIDE_POLYGON_FILL_H
#define GLIDE_POLYGON_FILL_H

#include "BresenhamLine.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace PolygonFill {

constexpr int WIDTH = 24;
constexpr int HEIGHT = 18;

struct Cell {
    int x;
    int y;
};

enum class Colour { Background, Boundary, Patch, Exterior };
enum class Method { Boundary4, Boundary8, Flood4, Flood8, Scanline };

using Grid = std::array<std::array<Colour, WIDTH>, HEIGHT>;

struct Step {
    int row;
    std::vector<Cell> cells;
};

using Steps = std::vector<Step>;

inline bool contains(Cell cell) {
    return cell.x >= 0 && cell.x < WIDTH && cell.y >= 0 && cell.y < HEIGHT;
}

inline void drawOutline(Grid& grid, const std::vector<Cell>& vertices) {
    for (size_t i = 0; i < vertices.size(); ++i) {
        const Cell a = vertices[i];
        const Cell b = vertices[(i + 1) % vertices.size()];
        BresenhamLine edge(Point(a.x, a.y), Point(b.x, b.y));
        edge.execute();
        for (const Point& pixel : edge.getAllPoints()) {
            const Cell cell{pixel.pixelX(), pixel.pixelY()};
            if (contains(cell)) grid[cell.y][cell.x] = Colour::Boundary;
        }
    }
}

inline Steps seedFill(const Grid& grid, Cell seed, Method method) {
    if (!contains(seed)) return {};
    const Colour target = grid[seed.y][seed.x];
    const bool boundaryFill = method == Method::Boundary4 || method == Method::Boundary8;
    const bool includeDiagonals = method == Method::Boundary8 || method == Method::Flood8;
    constexpr std::array<Cell, 8> directions{{
        {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, 1}, {-1, -1}, {1, -1}
    }};
    std::array<std::array<bool, WIDTH>, HEIGHT> visited{};
    std::vector<Cell> pending;
    auto discover = [&](Cell cell) {
        if (!contains(cell) || visited[cell.y][cell.x]) return;
        const Colour colour = grid[cell.y][cell.x];
        if (boundaryFill ? colour == Colour::Boundary : colour != target) return;
        visited[cell.y][cell.x] = true;
        pending.push_back(cell);
    };

    discover(seed);
    Steps steps;
    // Work through the queue in order so the animation spreads from the seed.
    for (size_t head = 0; head < pending.size(); ++head) {
        const Cell cell = pending[head];
        steps.push_back({cell.y, {cell}});
        for (size_t i = 0; i < (includeDiagonals ? directions.size() : 4); ++i) {
            discover({cell.x + directions[i].x, cell.y + directions[i].y});
        }
    }
    return steps;
}

inline Steps scanlineFill(const std::vector<Cell>& vertices, const Grid& grid) {
    Steps steps;
    std::vector<double> crossings;
    crossings.reserve(vertices.size());
    for (int y = 0; y < HEIGHT; ++y) {
        crossings.clear();
        for (size_t i = 0; i < vertices.size(); ++i) {
            Cell a = vertices[i];
            Cell b = vertices[(i + 1) % vertices.size()];
            if (a.y > b.y) std::swap(a, b);
            // Skip horizontal edges and count each shared vertex only once.
            if (y < a.y || y >= b.y) continue;
            crossings.push_back(a.x + double(y - a.y) * (b.x - a.x) / (b.y - a.y));
        }
        std::sort(crossings.begin(), crossings.end());
        Step step{y, {}};
        for (size_t i = 1; i < crossings.size(); i += 2) {
            const int first = std::max(0, static_cast<int>(std::ceil(crossings[i - 1])));
            const int end = std::min(WIDTH, static_cast<int>(std::ceil(crossings[i])));
            for (int x = first; x < end; ++x) {
                if (grid[y][x] == Colour::Background) step.cells.push_back({x, y});
            }
        }
        if (!step.cells.empty()) steps.push_back(std::move(step));
    }
    return steps;
}

} // namespace PolygonFill

#endif // GLIDE_POLYGON_FILL_H
