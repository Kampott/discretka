#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stack>

using namespace std;

// ��������� ��� ���� ������
struct Node {
    char symbol;
    double probability;
    Node* left;
    Node* right;

    Node(char s, double p) : symbol(s), probability(p), left(nullptr), right(nullptr) {}
};

// ������� ��������� ��� ���������� ����� �� �����������
bool compare(Node* a, Node* b) {
    return a->probability < b->probability;
}

// ������� ��� ���������� ������ ����
Node* buildFanoTree(const vector<pair<char, double>>& probabilities) {
    vector<Node*> nodes;

    // ������� ���� ��� ������� �������
    for (const auto& pair : probabilities) {
        nodes.push_back(new Node(pair.first, pair.second));
    }

    while (nodes.size() > 1) {
        // ��������� ���� �� �����������
        sort(nodes.begin(), nodes.end(), compare);

        // ���������� ��� ���� � ���������� ������������
        Node* left = nodes[0];
        Node* right = nodes[1];
        Node* parent = new Node('\0', left->probability + right->probability);
        parent->left = left;
        parent->right = right;

        // ������� �������������� ���� � ��������� ������������
        nodes.erase(nodes.begin());
        nodes.erase(nodes.begin());
        nodes.push_back(parent);
    }

    return nodes[0]; // ���������� ������ ������
}

// ������� ��� �������� ����� �� ������
void generateCodes(Node* root, unordered_map<char, string>& codes) {
    if (!root) return;

    stack<pair<Node*, string>> s;
    s.push({root, ""});

    while (!s.empty()) {
        auto [node, code] = s.top(); s.pop();

        if (node->symbol != '\0') { // ���� ��� �������� ����
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

// ������� ��� ������ ����� � ����
void writeCodesToFile(const unordered_map<char, string>& codes, const string& treeFile) {
    ofstream outTreeFile(treeFile, ios::trunc);
    if (!outTreeFile) {
        cerr << "�� ������� ������� ���� ��� ������ �����." << endl;
        return;
    }

   

    for (const auto& pair : codes) {
       if (!pair.second.empty()) { // ���������, ��� ��� �� ������
           if(pair.first == '\n') outTreeFile << "RETURN" << "\t" << pair.second << endl;
           else outTreeFile << pair.first << "\t" << pair.second << endl;
       }
   }
    
    outTreeFile.close();
}

// ������� ��� ����������� ������
vector<uint8_t> encode(const string& text, unordered_map<char, string>& codes) {
    vector<uint8_t> encodedBytes;
    string encodedString;
    int length = text.size()-1; // ����� �������� ������

    // �������� ����� � ������� ������
    for (char c : text) {
        if (codes.find(c) != codes.end()) {
            encodedString += codes[c];
        }
    }

    // ���������� ����� ������ (� ���������� ��������) � ������ 4 �����
    for (size_t i = 0; i < 4; ++i) {
        encodedBytes.push_back((length >> (24 - 8 * i)) & 0xFF);
    }

    // ���������� �������������� ���� � �����
    for (size_t i = 0; i < encodedString.size(); i += 8) {
        uint8_t byte = 0;
        size_t bitsToRead = min((size_t)8, encodedString.size() - i); // ������� ��� ����� ���������

        for (size_t j = 0; j < bitsToRead; ++j) {
            byte = (byte << 1) | (encodedString[i + j] - '0');
        }

        if (bitsToRead < 8) {
            byte <<= (8 - bitsToRead); // �������� �����, ����� ���������� ����� ��� �����
        }

        encodedBytes.push_back(byte);
    }

    return encodedBytes;
}

// ������� ��� ������������� ������

string decode(const vector<uint8_t>& encodedBytes, const unordered_map<string, char>& codes) {
    string decodedString;
    string currentCode;

    // ��������� ����� �������������� ������ �� ������ 4 ������
    size_t targetLength = 0;
    for (int i = 0; i < 4; ++i) {
        targetLength = (targetLength << 8) | encodedBytes[i];
    }

    int decodedLength = 0; // �������, ������� �������� �� ������������
    int bitCount = (encodedBytes.size() - 4) * 8; // ���������� ���, ��������� ��� �������������
    // �������� �� ���� �����, �������� �� �� ������, ������� � 5-�� �����
    for (size_t i = 4 * 8; i < bitCount + 4 * 8; ++i) {
        int byteIndex = i / 8; // ������ �����, ����������� ������� ���
        int bitPosition = 7 - (i % 8); // ������� ���� � ����� (�� 7 �� 0)

        currentCode += ((encodedBytes[byteIndex] >> bitPosition) & 1) ? '1' : '0';

        // ���������, ������ �� ������� ���
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
            decodedLength++; // ����������� ���������� �������������� ��������
            currentCode.clear(); // ������� ������� ��� ����� ��������� �������������
            // ���� �������� ������� �����, ���������
            if (decodedLength == targetLength) {
                break;
            }
        } 
        // ���� ������� ��� ��������� ������������ �����, ������� ���
        else if (currentCode.size() > 20) { // ������� maxCodeLength �� 20 ��� �������
            currentCode.clear(); // ������� ������� ���, ���� �� ������� �������
        }
    }

    // ���������, �������� �� ���� � currentCode ����� ���������� �����
    if (!currentCode.empty() && currentCode.size() <= 20) { // �������� �� 20 ����
        // �� ��������� ������ ������, ���� ��� �� ������������� �� ������ �������
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
        }
    }

    return decodedString;
}
   




// ������� ��� ��������� ������������ �������� �� ������
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

// �������� ������� ��������� ����
void fanoEncoding(const string& inputFile, const string& outputFile, const string& treeFile) {
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "�� ������� ������� ������� ����." << endl;
        return;
    }

    string text = "", line;
    while (getline(inFile, line)) {
        text += line + '\n'; // ���������� ������
    }
    inFile.close();

    // �������� ����������� ��������
    unordered_map<char, double> probabilitiesMap = getProbabilities(text);
    
    // ����������� � ������ ���
    vector<pair<char, double>> probabilities(probabilitiesMap.begin(), probabilitiesMap.end());

    // ������ ������ ����
    Node* root = buildFanoTree(probabilities);

    // ���������� ����
    unordered_map<char, string> codes;
    generateCodes(root, codes);

    // �������� �����
    vector<uint8_t> encodedBytes = encode(text, codes);

    // ���������� �������������� ����� � ����
    ofstream outFile(outputFile, ios::binary);
    outFile.write(reinterpret_cast<char*>(encodedBytes.data()), encodedBytes.size());
    outFile.close();

    // ���������� ���� � ��������� ����
    writeCodesToFile(codes, treeFile);
}

// ������� ��� ������������� ������
void fanoDecoding(const string& encodedFile, const string& treeFile, const string& outputFile) {
    ifstream inFile(encodedFile, ios::binary);
    if (!inFile) {
        cerr << "�� ������� ������� ������� ����." << endl;
        return;
    }

    // ��������� ���� �� �����
    vector<uint8_t> encodedBytes((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // �������� ���������� ��� �� ���������� �����
    int bitCount = 8 * encodedBytes.size(); // ����� ���������� ���
    int lastByteBits = bitCount % 8; // ���������� ��� � ��������� �����
    

    if (lastByteBits > 0) {
        // ���� � ��������� ����� ���� ����, �� ���������� ���
        bitCount -= 8 - lastByteBits; // ������� �� ������ ���������� ��� ���������� �������������� ��� � ��������� �����
    }

    unordered_map<string, char> codes; // ��� �������������
    ifstream treeFileIn(treeFile);
    if (!treeFileIn) {
        cerr << "�� ������� ������� ���� � ������." << endl;
        return;
    }

    string codeLine;
    int maxCodeLength = 0; // ���������� ��� �������� ������������ ����� ����

    while (getline(treeFileIn, codeLine)) {
        if (codeLine.empty()) continue; // ���������� ������ ������

        // ��������� �� ������� ������� RETURN � �������� ���
        if (codeLine.find("RETURN") != string::npos) {
            // ���� ������ "RETURN", �������� ��� �� ������ �������� ������ '\n'
            codeLine.replace(codeLine.find("RETURN"), 6, "\n"); // "RETURN" ����� ����� 6 ��������
        }

        size_t pos = codeLine.find('\t'); // ���� ��������� ��� �����������
        if (pos != string::npos) {
            char symbol = codeLine.substr(0, pos)[0];
            cout << symbol << endl;
            string code = codeLine.substr(pos + 1);
            codes[code] = symbol; // ��������� ��� � ��������������� ������
            // ��������� ������������ ����� ����
            maxCodeLength = max(maxCodeLength, static_cast<int>(code.length()));
        }
    }

    treeFileIn.close();

    // ���������� ������
    string decodedString = decode(encodedBytes, codes);

    // ���������� �������������� ������ � ����
    ofstream outFile(outputFile);

    if (!outFile) {
        cerr << "�� ������� ������� �������� ����." << endl;
        return;
    }

    outFile << decodedString;
    outFile.close();
}

// ������� �������
int main() {
    string inputFile;  
    string encodedFile = "encoded.bin"; 
    string decodedFile = "decoded.txt"; 
    string treeFile = "tree.txt";
    int regime;
    do{
    inputFile = "";
    
    do{
    cout << "�������� �����: 1 - ���������, 2 - �����������, 0 - �����" << endl;
    cin >> regime;
    if(regime == 1 || regime == 2 || regime == 0){
        break;
    }
    } while(true);
    if(regime == 1){
        do{
        cout << "������� �������� ����� ��� ���������" << endl;
        cin >> inputFile;
        }while(inputFile == "");
        encodedFile = "encoded.bin";
        fanoEncoding(inputFile, encodedFile, treeFile);
    }
    else if(regime == 2){
    do{
        cout << "������� �������� ����� ��� ���������" << endl;
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
