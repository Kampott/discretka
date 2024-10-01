#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>

using namespace std;

// Структура для узла дерева
struct Node {
    char symbol;
    double probability;
    Node* left;
    Node* right;

    Node(char s, double p) : symbol(s), probability(p), left(nullptr), right(nullptr) {}
};

// Функция сравнения для сортировки узлов по вероятности
bool compare(Node* a, Node* b) {
    return a->probability < b->probability;
}

// Функция для построения дерева Фано
Node* buildFanoTree(const vector<pair<char, double>>& probabilities) {
    vector<Node*> nodes;

    // Создаем узлы для каждого символа
    for (const auto& pair : probabilities) {
        nodes.push_back(new Node(pair.first, pair.second));
    }

    while (nodes.size() > 1) {
        // Сортируем узлы по вероятности
        sort(nodes.begin(), nodes.end(), compare);

        // Объединяем два узла с наименьшей вероятностью
        Node* left = nodes[0];
        Node* right = nodes[1];
        Node* parent = new Node('\0', left->probability + right->probability);
        parent->left = left;
        parent->right = right;

        // Удаляем использованные узлы и добавляем родительский
        nodes.erase(nodes.begin());
        nodes.erase(nodes.begin());
        nodes.push_back(parent);
    }

    return nodes[0]; // Возвращаем корень дерева
}

// Функция для создания кодов из дерева
void generateCodes(Node* root, const string& code, unordered_map<char, string>& codes) {
    if (!root) return;
    
    if (root->symbol != '\0') { // Если это листовой узел
        codes[root->symbol] = code;
    }
    
    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// Функция для записи кодов в файл
void writeCodesToFile(const unordered_map<char, string>& codes, const string& treeFile) {
    ofstream outTreeFile(treeFile);
    for (const auto& pair : codes) {
        outTreeFile << pair.first << "_" << pair.second << endl;
    }
    outTreeFile.close();
}

// Функция для кодирования текста
vector<uint8_t> encode(const string& text, unordered_map<char, string>& codes) {
    vector<uint8_t> encodedBytes;
    string encodedString;

    for (char c : text) {
        encodedString += codes[c];
    }

    // Преобразование битовой строки в байты
    for (size_t i = 0; i < encodedString.size(); i += 8) {
        uint8_t byte = 0;
        for (size_t j = 0; j < 8 && (i + j) < encodedString.size(); ++j) {
            byte = (byte << 1) | (encodedString[i + j] - '0');
        }
        encodedBytes.push_back(byte);
    }

    return encodedBytes;
}

// Функция для декодирования байтов
string decode(const vector<uint8_t>& encodedBytes, const unordered_map<string, char>& codes) {
    string decodedString;
    string currentCode;

    for (uint8_t byte : encodedBytes) {
        for (int i = 7; i >= 0; --i) {
            currentCode += ((byte >> i) & 1) ? '1' : '0';
            if (codes.find(currentCode) != codes.end()) {
                decodedString += codes.at(currentCode);
                currentCode.clear();
            }
        }
    }

    return decodedString;
}

// Функция для получения вероятностей символов из текста
unordered_map<char, double> getProbabilities(const string& text) {
    unordered_map<char, int> frequency;
    for (char c : text) {
        frequency[c]++;
    }

    unordered_map<char, double> probabilities;
    for (const auto& pair : frequency) {
        probabilities[pair.first] = static_cast<double>(pair.second) / text.size();
    }

    return probabilities;
}

// Основная функция алгоритма Фано
void fanoEncoding(const string& inputFile, const string& outputFile, const string& treeFile) {
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "Не удалось открыть входной файл." << endl;
        return;
    }

    string text = "", line;
    while (getline(inFile, line)) {
        text += line + '\n'; // Объединяем строки
    }
    inFile.close();

    // Получаем вероятности символов
    unordered_map<char, double> probabilitiesMap = getProbabilities(text);
    
    // Преобразуем в вектор пар
    vector<pair<char, double>> probabilities(probabilitiesMap.begin(), probabilitiesMap.end());

    // Строим дерево Фано
    Node* root = buildFanoTree(probabilities);

    // Генерируем коды
    unordered_map<char, string> codes;
    generateCodes(root, "", codes);

    // Кодируем текст
    vector<uint8_t> encodedBytes = encode(text, codes);

    // Записываем закодированные байты в файл
    ofstream outFile(outputFile, ios::binary);
    outFile.write(reinterpret_cast<char*>(encodedBytes.data()), encodedBytes.size());
    outFile.close();

    // Записываем коды в текстовый файл
    writeCodesToFile(codes, treeFile);
}

// Функция для декодирования текста
void fanoDecoding(const string& encodedFile, const string& treeFile, const string& outputFile) {
    ifstream inFile(encodedFile, ios::binary);
    if (!inFile) {
        cerr << "Не удалось открыть входной файл." << endl;
        return;
    }

    vector<uint8_t> encodedBytes((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    unordered_map<string, char> codes; // Для декодирования
    ifstream treeFileIn(treeFile);
    string codeLine;
    
    while (getline(treeFileIn, codeLine)) {
        size_t pos = codeLine.find('_');
        if (pos != string::npos) {
            char symbol = codeLine.substr(0, pos)[0];
            string code = codeLine.substr(pos + 1);
            codes[code] = symbol; // Сохраняем код и соответствующий символ
        }
    }
    
    treeFileIn.close();

    // Декодируем строку
    string decodedString = decode(encodedBytes, codes);

    // Записываем декодированную строку в файл
    ofstream outFile(outputFile);
    
    if (!outFile) {
        cerr << "Не удалось открыть выходной файл." << endl;
        return;
    }
    
    outFile << decodedString;
    outFile.close();
}

// Главная функция
int main() {
    string inputFile = "input.txt";    
    string encodedFile = "encoded.bin"; 
    string decodedFile = "decoded.txt"; 
    string treeFile = "tree.txt"; 

    fanoEncoding(inputFile, encodedFile, treeFile);
    fanoDecoding(encodedFile, treeFile, decodedFile);

    return 0;
}
