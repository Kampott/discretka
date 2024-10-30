#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include "Point.h"

class Grid {
    int width, height;
    std::vector<std::vector<int>> walls;
    std::vector<std::vector<int>> costs;

public:
    Grid(int w, int h);

    bool isInside(const Point& p) const;
    bool hasWall(const Point& p) const;
    int getCost(const Point& to) const;

    void addWall(const Point& p);
    void setCost(const Point& p, int cost);

    std::vector<Point> getNeighbors(const Point& p) const;
    std::vector<Point> findPath(const Point& start, const Point& goal, int& pathCost);

    void displayPath(const std::vector<Point>& path) const;
};