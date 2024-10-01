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
    
    if (root->symbol != '\0' && root->symbol != '\n') { // Если это листовой узел
        codes[root->symbol] = code;
    }
    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// Функция для записи кодов в файл
void writeCodesToFile(const unordered_map<char, string>& codes, const string& treeFile) {
    ofstream outTreeFile(treeFile, ios::trunc);
    if (!outTreeFile) {
        cerr << "Не удалось открыть файл для записи кодов." << endl;
        return;
    }

   

    for (const auto& pair : codes) {
       if (!pair.second.empty()) { // Проверяем, что код не пустой
           outTreeFile << pair.first << "\t" << pair.second << endl;
       }
   }
    
    outTreeFile.close();
}

// Функция для кодирования текста
vector<uint8_t> encode(const string& text, unordered_map<char, string>& codes) {
    vector<uint8_t> encodedBytes;
    string encodedString;

    // Кодируем текст в битовую строку
    for (char c : text) {
        if (codes.find(c) != codes.end()) {
            encodedString += codes[c];
        }
    }
    cout << encodedString << endl;
for (size_t i = 0; i < encodedString.size(); i += 8) {
    uint8_t byte = 0;
    unsigned long long eight = 8;
    size_t bitsToRead = min(eight, encodedString.size() - i); // Сколько бит нужно прочитать

    for (size_t j = 0; j < bitsToRead; ++j) {
        byte = (byte << 1) | (encodedString[i + j] - '0');
    }

    if (bitsToRead < 8) {
        byte <<= (8 - bitsToRead); // Сдвигаем влево, чтобы освободить место для нулей
    }

    encodedBytes.push_back(byte);
}

    return encodedBytes;
}

// Функция для декодирования байтов

string decode(const vector<uint8_t>& encodedBytes, int bitCount, const unordered_map<string, char>& codes, int maxCodeLength) {
    string decodedString;
    string currentCode;
    int allbits = 0;
    int bitIndex = 0; // Индекс текущего бита в байтовом массиве
    cout << bitCount;
    // Проходим по всем битам, считывая их из байтов
    for (int i = 0; i < bitCount; ++i) {
        allbits++;
        int byteIndex = i / 8; // Индекс байта, содержащего текущий бит
        int bitPosition = 7 - (i % 8); // Позиция бита в байте (от 7 до 0)

        currentCode += ((encodedBytes[byteIndex] >> bitPosition) & 1) ? '1' : '0';

        // Проверяем, найден ли текущий код
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
            //cout << currentCode << endl;
            currentCode.clear(); // Очищаем текущий код после успешного декодирования
        } 
        // Если текущий код превышает максимальную длину, очищаем его
        else if (currentCode.size() > maxCodeLength) {
            currentCode.clear(); // Очищаем текущий код, если он слишком длинный
        }
    }
    cout << allbits << endl;
    // Проверяем, остались ли биты в currentCode после завершения цикла
    if (!currentCode.empty() && currentCode.size() <= maxCodeLength) {
        // Не добавляем лишний символ, если код не соответствует ни одному символу
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
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

    // Считываем биты из файла
    vector<uint8_t> encodedBytes((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // Получаем количество бит из последнего байта
    int bitCount = 8 * encodedBytes.size(); // общее количество бит
    cout << encodedBytes.size() << endl;
    int lastByteBits = bitCount % 8; // количество бит в последнем байте

    if (lastByteBits > 0) {
        // Если в последнем байте есть биты, то используем его
        bitCount -= 8 - lastByteBits; // Убираем из общего количества бит количество неиспользуемых бит в последнем байте
    }

    unordered_map<string, char> codes; // Для декодирования
    ifstream treeFileIn(treeFile);
    if (!treeFileIn) {
        cerr << "Не удалось открыть файл с кодами." << endl;
        return;
    }

    string codeLine;
    int maxCodeLength = 0; // Переменная для хранения максимальной длины кода

    while (getline(treeFileIn, codeLine)) {
        if (codeLine.empty()) continue; // Пропускаем пустые строки
        size_t pos = codeLine.find('\t'); // Ищем табуляцию как разделитель
        if (pos != string::npos) {
            char symbol = codeLine.substr(0, pos)[0];
            string code = codeLine.substr(pos + 1);
            codes[code] = symbol; // Сохраняем код и соответствующий символ
            // Обновляем максимальную длину кода
            maxCodeLength = max(maxCodeLength, static_cast<int>(code.length()));
        }
    }

    treeFileIn.close();

    // Декодируем строку
    string decodedString = decode(encodedBytes, bitCount, codes, maxCodeLength);

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
