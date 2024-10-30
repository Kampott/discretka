#include <iostream>
#include <fstream>
#include <sstream>
#include "Grid.h"
#include "Point.h"

void createSampleFile(const std::string& filename) {
    std::ofstream file(filename);

    file << "width: 20\n";
    file << "height: 20\n";
    file << "start: 0 3\n";
    file << "finish: 0 4\n";
    file << "walls: 1 2 3 4\n";
    file << "costs: 0 1 2\n";

    file.close();
}

Point parsePoint(std::istringstream& stream) {
    int x, y;
    stream >> x >> y;
    return { x, y };
}

Grid loadGridFromFile(const std::string& filename, Point& start, Point& finish) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл.");
    }

    std::string line;
    int width = 0, height = 0;
    std::vector<Point> walls;
    std::vector<std::pair<Point, int>> costs;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string key;
        stream >> key;

        if (key == "width:") stream >> width;
        else if (key == "height:") stream >> height;
        else if (key == "start:") start = parsePoint(stream);
        else if (key == "finish:") finish = parsePoint(stream);
        else if (key == "walls:") {
            int x, y;
            while (stream >> x >> y) {
                if (stream.fail()) break; // Если не удалось считать, выходим из цикла
                walls.push_back({ x, y });
            }
        }
        else if (key == "costs:") {
            int x, y, cost;
            while (stream >> x >> y >> cost) {
                if (stream.fail()) break; // Если не удалось считать, выходим из цикла
                costs.push_back({ {x, y}, cost });
            }
        }
    }

    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Неверные размеры сетки.");
    }

    Grid graph(width, height);
    for (const auto& wall : walls) {
        graph.addWall(wall);
    }
    for (const auto& costPair : costs) {
        graph.setCost(costPair.first, costPair.second);
    }

    return graph;
}

void savePathToFile(const std::vector<Point>& path, int cost) {
    std::ofstream file("output.txt");
    file << "Стоимость пути: " << cost << "\n";
    for (const auto& p : path)
        file << "(" << p.x << ", " << p.y << ")\n";
}

int main() {
    std::string filename = "input.txt";

    std::ifstream infile(filename);
    if (!infile.good()) {
        createSampleFile(filename);
        return 0;
    }

    Point start, finish;
    Grid graph = loadGridFromFile(filename, start, finish);
    int pathCost = 0;
    std::vector<Point> path = graph.findPath(start, finish, pathCost);
    if (pathCost == -1) {
        std::cout << "Путь не найден.\n";
    }
    else {
        graph.displayPath(path);
        savePathToFile(path, pathCost);
    }

    return 0;
}