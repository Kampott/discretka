#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <set>

using namespace std;

// Структура для представления узла
struct Node {
    int x, y;
    double g, h;

    Node(int x, int y, double g, double h) : x(x), y(y), g(g), h(h) {}

    double f() const {
        return g + h;
    }

    bool operator<(const Node& other) const {
        return f() > other.f(); // для приоритетной очереди (меньшее f имеет больший приоритет)
    }

    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }
};

// Хэш-функция для unordered_map
struct NodeHash {
    size_t operator()(const pair<int, int>& p) const {
        return hash<int>()(p.first) ^ hash<int>()(p.second);
    }
};

// Восстановление пути
vector<pair<int, int>> reconstruct_path(const unordered_map<pair<int, int>, pair<int, int>, NodeHash>& came_from,
    const pair<int, int>& current) {
    vector<pair<int, int>> path;
    pair<int, int> node = current;
    while (came_from.find(node) != came_from.end()) {
        path.push_back(node);
        node = came_from.at(node);
    }
    reverse(path.begin(), path.end());
    return path;
}

// Евклидово расстояние
double heuristic(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Проверка, разрешен ли переход между двумя клетками
bool can_move(const pair<int, int>& from, const pair<int, int>& to,
    const set<pair<pair<int, int>, pair<int, int>>>& walls) {
    return walls.find({ from, to }) == walls.end() && walls.find({ to, from }) == walls.end();
}

// Алгоритм А*
vector<pair<int, int>> a_star(const vector<vector<bool>>& grid,
    pair<int, int> start, pair<int, int> goal,
    const vector<pair<int, int>>& directions,
    const set<pair<pair<int, int>, pair<int, int>>>& walls) {
    priority_queue<Node> open_set;
    unordered_map<pair<int, int>, double, NodeHash> g_score;
    unordered_map<pair<int, int>, double, NodeHash> f_score;
    unordered_map<pair<int, int>, pair<int, int>, NodeHash> came_from;

    open_set.emplace(start.first, start.second, 0.0, heuristic(start.first, start.second, goal.first, goal.second));
    g_score[start] = 0.0;
    f_score[start] = heuristic(start.first, start.second, goal.first, goal.second);

    set<pair<int, int>> closed_set;

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();
        pair<int, int> current_pos = { current.x, current.y };

        if (current_pos == goal) {
            return reconstruct_path(came_from, goal);
        }

        closed_set.insert(current_pos);

        for (const auto& dir : directions) {
            int nx = current.x + dir.first;
            int ny = current.y + dir.second;
            pair<int, int> neighbor = { nx, ny };

            if (nx < 0 || ny < 0 || nx >= grid.size() || ny >= grid[0].size() || !grid[nx][ny] ||
                closed_set.find(neighbor) != closed_set.end() ||
                !can_move(current_pos, neighbor, walls)) {
                continue;
            }

            double tentative_g_score = g_score[current_pos] + 1.0; 

            if (g_score.find(neighbor) == g_score.end() || tentative_g_score < g_score[neighbor]) {
                came_from[neighbor] = current_pos;
                g_score[neighbor] = tentative_g_score;
                f_score[neighbor] = tentative_g_score + heuristic(nx, ny, goal.first, goal.second);

                open_set.emplace(nx, ny, g_score[neighbor], heuristic(nx, ny, goal.first, goal.second));
            }
        }
    }

    return {}; // если путь не найден
}

int main() {
    setlocale(LC_ALL, "Russian");
    ifstream infile("input.txt");
    if (!infile) {
        cerr << "Ошибка: не удалось открыть файл input.txt" << endl;
        return 1;
    }

    int rows, cols, start_x, start_y, goal_x, goal_y;
    infile >> rows >> cols >> start_x >> start_y >> goal_x >> goal_y;

    vector<vector<bool>> grid(rows, vector<bool>(cols, true));
    set<pair<pair<int, int>, pair<int, int>>> walls;
    string line;

    while (getline(infile, line)) {
        istringstream iss(line);
        int x1, y1, x2, y2;
        if (iss >> x1 >> y1 >> x2 >> y2) {
            walls.insert({ {x1, y1}, {x2, y2} });
        }
    }

    vector<pair<int, int>> directions = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} }; // четыре направления
                                         //{1, 1}, {1, -1}, {-1, 1}, {-1, -1} }; // или добавить диагонали

    vector<pair<int, int>> path = a_star(grid, { start_x, start_y }, { goal_x, goal_y }, directions, walls);

    if (path.empty()) {
        cout << "Путь не найден" << endl;
    }
    else {
        cout << "Найденный путь:" << endl;
        for (const auto& p : path) {
            cout << "(" << p.first << ", " << p.second << ")" << endl;
        }
    }

    return 0;
}
