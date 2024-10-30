#include "Grid.h"
#include <algorithm>
#include <climits>

Grid::Grid(int w, int h) : width(w), height(h) {
    walls.resize(width, std::vector<int>(height, 0));
    costs.resize(width, std::vector<int>(height, 1));
}

bool Grid::isInside(const Point& p) const {
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
}

bool Grid::hasWall(const Point& p) const {
    return isInside(p) && walls[p.x][p.y] == 1;
}

int Grid::getCost(const Point& to) const {
    return isInside(to) ? costs[to.x][to.y] : INT_MAX;
}

void Grid::addWall(const Point& p) {
    if (isInside(p)) walls[p.x][p.y] = 1;
}

void Grid::setCost(const Point& p, int cost) {
    if (isInside(p)) costs[p.x][p.y] = cost;
}

std::vector<Point> Grid::getNeighbors(const Point& p) const {
    std::vector<Point> neighbors;
    std::vector<Point> directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

    for (const auto& dir : directions) {
        Point next = { p.x + dir.x, p.y + dir.y };
        if (isInside(next) && !hasWall(next))
            neighbors.push_back(next);
    }

    return neighbors;
}

std::vector<Point> Grid::findPath(const Point& start, const Point& goal, int& pathCost) {
    std::priority_queue<std::pair<int, Point>, std::vector<std::pair<int, Point>>, std::greater<>> frontier;
    frontier.push({ 0, start });

    std::vector<Point> previous(width * height, { -1, -1 });
    std::vector<int> costSoFar(width * height, INT_MAX);

    costSoFar[start.x + start.y * width] = 0;
    previous[start.x + start.y * width] = start;

    while (!frontier.empty()) {
        Point current = frontier.top().second;
        frontier.pop();

        if (current == goal) break;

        for (const auto& next : getNeighbors(current)) {
            int new_cost = costSoFar[current.x + current.y * width] + getCost(next);

            if (new_cost < costSoFar[next.x + next.y * width]) {
                costSoFar[next.x + next.y * width] = new_cost;
                int priority = new_cost + std::abs(goal.x - next.x) + std::abs(goal.y - next.y);
                frontier.push({ priority, next });
                previous[next.x + next.y * width] = current;
            }
        }
    }

    std::vector<Point> path;
    if (previous[goal.x + goal.y * width] == Point(-1, -1)) {
        pathCost = -1;
        return path; // ѕуть не найден
    }

    for (Point at = goal; at != start; at = previous[at.x + at.y * width]) {
        path.push_back(at);
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());
    pathCost = costSoFar[goal.x + goal.y * width];

    return path;
}

void Grid::displayPath(const std::vector<Point>& path) const {
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            Point p = { x, y };
            if (std::find(path.begin(), path.end(), p) != path.end())
                std::cout << "X ";
            else if (hasWall(p))
                std::cout << "# ";
            else
                std::cout << ". ";
        }
        std::cout << '\n';
    }
}