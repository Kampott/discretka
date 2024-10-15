#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stack>

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
Node* buildFanoRecursive(vector<Node*>& nodes, int start, int end) {
    if (start == end) {
        return nodes[start];
    }

    // Находим точку разделения
    int split = start;
    double total_prob = 0.0, left_prob = 0.0;
    for (int i = start; i <= end; ++i) {
        total_prob += nodes[i]->probability;
    }

    // Ищем позицию, где сумма вероятностей будет наиболее сбалансированной
    for (int i = start; i <= end; ++i) {
        left_prob += nodes[i]->probability;
        if (left_prob >= total_prob / 2) {
            split = i;
            break;
        }
    }

    Node* parent = new Node('\0', total_prob); // Создаем родительский узел
    parent->left = buildFanoRecursive(nodes, start, split);   // Левая часть
    parent->right = buildFanoRecursive(nodes, split + 1, end); // Правая часть

    return parent;
}
// Функция для построения дерева Фано
Node* buildFanoTree(const vector<pair<char, double>>& probabilities) {
    vector<Node*> nodes;

    // Создаем узлы для каждого символа
    for (const auto& pair : probabilities) {
        nodes.push_back(new Node(pair.first, pair.second));
    }

    // Сортируем узлы по вероятности в порядке убывания
    sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) {
        return a->probability > b->probability;
    });

    // Рекурсивно строим дерево Фано
    return buildFanoRecursive(nodes, 0, nodes.size() - 1);
}

// Функция для создания кодов из дерева
void generateCodes(Node* root, unordered_map<char, string>& codes) {
    if (!root) return;

    stack<pair<Node*, string>> s;
    s.push({root, ""});

    while (!s.empty()) {
        auto [node, code] = s.top(); s.pop();

        if (node->symbol != '\0') { // Если это листовой узел
            codes[node->symbol] = code;
        }

        if (node->right) {
            s.push({node->right, code + "1"});
        }

        if (node->left) {
            s.push({node->left, code + "0"});
        }
    }
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
            if (pair.first == '\n') outTreeFile << "RETURN" << "\t" << pair.second << endl;
            else outTreeFile << pair.first << "\t" << pair.second << endl;
        }
    }

    outTreeFile.close();
}

// Функция для кодирования текста
vector<uint8_t> encode(const string& text, unordered_map<char, string>& codes) {
    vector<uint8_t> encodedBytes;
    string encodedString;
    int length = text.size() - 1; // Длина входного текста

    // Кодируем текст в битовую строку
    for (char c : text) {
        if (codes.find(c) != codes.end()) {
            encodedString += codes[c];
        }
    }

    // Записываем длину текста (в количестве символов) в первые 4 байта
    for (size_t i = 0; i < 4; ++i) {
        encodedBytes.push_back((length >> (24 - 8 * i)) & 0xFF);
    }

    // Записываем закодированные биты в байты
    for (size_t i = 0; i < encodedString.size(); i += 8) {
        uint8_t byte = 0;
        size_t bitsToRead = min((size_t)8, encodedString.size() - i); // Сколько бит нужно прочитать

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

string decode(const vector<uint8_t>& encodedBytes, const unordered_map<string, char>& codes) {
    string decodedString;
    string currentCode;

    // Считываем длину закодированной строки из первых 4 байтов
    size_t targetLength = 0;
    for (int i = 0; i < 4; ++i) {
        targetLength = (targetLength << 8) | encodedBytes[i];
    }

    int decodedLength = 0; // Считаем, сколько символов мы декодировали
    int bitCount = (encodedBytes.size() - 4) * 8; // Количество бит, доступное для декодирования
    // Проходим по всем битам, считывая их из байтов, начиная с 5-го байта
    for (size_t i = 4 * 8; i < bitCount + 4 * 8; ++i) {
        int byteIndex = i / 8; // Индекс байта, содержащего текущий бит
        int bitPosition = 7 - (i % 8); // Позиция бита в байте (от 7 до 0)

        currentCode += ((encodedBytes[byteIndex] >> bitPosition) & 1) ? '1' : '0';

        // Проверяем, найден ли текущий код
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
            decodedLength++; // Увеличиваем количество декодированных символов
            currentCode.clear(); // Очищаем текущий код после успешного декодирования
            // Если достигли целевой длины, завершаем
            if (decodedLength == targetLength) {
                break;
            }
        } else if (currentCode.size() > 20) { // Заменил maxCodeLength на 20 для примера
            currentCode.clear(); // Очищаем текущий код, если он слишком длинный
        }
    }

    // Проверяем, остались ли биты в currentCode после завершения цикла
    if (!currentCode.empty() && currentCode.size() <= 20) { // Проверка на 20 тоже
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
    generateCodes(root, codes);

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

        // Проверяем на наличие символа RETURN и заменяем его
        if (codeLine.find("RETURN") != string::npos) {
            // Если найден "RETURN", заменяем его на символ перевода строки '\n'
            codeLine.replace(codeLine.find("RETURN"), 6, "\n"); // "RETURN" имеет длину 6 символов
        }

        size_t pos = codeLine.find('\t'); // Ищем табуляцию как разделитель
        if (pos != string::npos) {
            char symbol = codeLine.substr(0, pos)[0];
            cout << symbol << endl;
            string code = codeLine.substr(pos + 1);
            codes[code] = symbol; // Сохраняем код и соответствующий символ
            // Обновляем максимальную длину кода
            maxCodeLength = max(maxCodeLength, static_cast<int>(code.length()));
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
    setlocale(LC_ALL, "Russian");
    string inputFile;  
    string encodedFile = "encoded.bin"; 
    string decodedFile = "decoded.txt"; 
    string treeFile = "tree.txt";
    int regime;
    do{
    inputFile = "";
    
    do{
    cout << "выберите режим: 1 - кодировка, 2 - декодировка, 0 - выход" << endl;
    cin >> regime;
    if(regime == 1 || regime == 2 || regime == 0){
        break;
    }
    } while(true);
    if(regime == 1){
        do{
        cout << "введите название файла для кодировки" << endl;
        cin >> inputFile;
        }while(inputFile == "");
        encodedFile = "encoded.bin";
        fanoEncoding(inputFile, encodedFile, treeFile);
    }
    else if(regime == 2){
    do{
        cout << "введите название файла для кодировки" << endl;
        cin >> inputFile;
    }while(inputFile == "");
    fanoDecoding(encodedFile, treeFile, decodedFile);
    }
    else if(regime == 0){
        break;
    }
    }while(regime != 0);
    return 0;
}
