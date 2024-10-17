#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stack>

using namespace std;

// ˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜ ˜˜˜˜˜˜
struct Node {
    char symbol;
    double probability;
    Node* left;
    Node* right;

    Node(char s, double p) : symbol(s), probability(p), left(nullptr), right(nullptr) {}
};


bool compare(Node* a, Node* b) {
    return a->probability < b->probability;
}
int med(int start, int end, vector<Node*>& nodes) {
    double left_sum = 0.0;
    for (int i = start; i <= end; ++i) {
        left_sum += nodes[i]->probability;
    }

    double right_sum = 0.0;
    int m = end;
    while (m > start && abs(left_sum - right_sum) >= abs(nodes[m]->probability)) {
        left_sum -= nodes[m]->probability;
        right_sum += nodes[m]->probability;
        m--;
    }
    return m; 
}

Node* buildFanoIterative(vector<Node*>& nodes) {
    stack<tuple<int, int, Node*>> stack;
    
    double totalProb = 0.0;
    for (auto node : nodes) {
        totalProb += node->probability;
    }
    Node* root = new Node('\0', totalProb); 
    stack.push(make_tuple(0, nodes.size() - 1, root));

    while (!stack.empty()) {
        auto [start, end, parent] = stack.top();
        stack.pop();

        if (start == end) {
            parent->left = nodes[start]; 
            continue;
        }

        int split = med(start, end, nodes);

        Node* leftChild = new Node('\0', 0.0);
        Node* rightChild = new Node('\0', 0.0);


        for (int i = start; i <= split; ++i) {
            leftChild->probability += nodes[i]->probability;
        }
        for (int i = split + 1; i <= end; ++i) {
            rightChild->probability += nodes[i]->probability;
        }

        parent->left = leftChild; 
        parent->right = rightChild; 

        stack.push(make_tuple(start, split, leftChild));
        stack.push(make_tuple(split + 1, end, rightChild));
    }

    return root; 
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜
Node* buildFanoTree(const vector<pair<char, double>>& probabilities) {
    vector<Node*> nodes;

    // ˜˜˜˜˜˜˜ ˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
    for (const auto& pair : probabilities) {
        nodes.push_back(new Node(pair.first, pair.second));
    }

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜
    sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) {
        return a->probability > b->probability;
    });

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜
    return buildFanoIterative(nodes);
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜
void generateCodes(Node* root, unordered_map<char, string>& codes) {
    if (!root) return;

    stack<pair<Node*, string>> s;
    s.push({root, ""});

    while (!s.empty()) {
        auto [node, code] = s.top(); s.pop();

        if (node->symbol != '\0') { // ˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜
            codes[node->symbol] = code;
            cout << node->symbol << " " << code << " " << node->probability << endl;
        }

        if (node->right) {
            s.push({node->right, code + "1"});
        }

        if (node->left) {
            s.push({node->left, code + "0"});
        }
    }
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜ ˜ ˜˜˜˜
void writeCodesToFile(const unordered_map<char, string>& codes, const string& treeFile) {
    ofstream outTreeFile(treeFile, ios::trunc);
    if (!outTreeFile) {
        cerr << "˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜." << endl;
        return;
    }

    for (const auto& pair : codes) {
        if (!pair.second.empty()) { // ˜˜˜˜˜˜˜˜˜, ˜˜˜ ˜˜˜ ˜˜ ˜˜˜˜˜˜
            if (pair.first == '\n') outTreeFile << "RETURN" << "\t" << pair.second << endl;
            //else if (pair.first == '\t') outTreeFile << "TAB" << "\t" << pair.second << endl;
            else outTreeFile << pair.first << "\t" << pair.second << endl;
        }
    }

    outTreeFile.close();
}
void printFanoTree(Node* root) {
    if (root) {
        if(root->symbol != '\0'){
        cout << root->symbol << "N" << root->probability << endl;
        // Print the left and right children
        printFanoTree(root->left);
        printFanoTree(root->right);
        }
    }
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
vector<uint8_t> encode(const string& text, unordered_map<char, string>& codes) {
    vector<uint8_t> encodedBytes;
    string encodedString;
    int length = text.size() - 1; // ˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    for (char c : text) {
        if (codes.find(c) != codes.end()) {
            encodedString += codes[c];
        }
    }

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜ (˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜) ˜ ˜˜˜˜˜˜ 4 ˜˜˜˜˜
    for (size_t i = 0; i < 4; ++i) {
        encodedBytes.push_back((length >> (24 - 8 * i)) & 0xFF);
    }

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜
    for (size_t i = 0; i < encodedString.size(); i += 8) {
        uint8_t byte = 0;
        size_t bitsToRead = min((size_t)8, encodedString.size() - i); // ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜

        for (size_t j = 0; j < bitsToRead; ++j) {
            byte = (byte << 1) | (encodedString[i + j] - '0');
        }

        if (bitsToRead < 8) {
            byte <<= (8 - bitsToRead); // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜, ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜
        }

        encodedBytes.push_back(byte);
    }

    return encodedBytes;
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜

string decode(const vector<uint8_t>& encodedBytes, const unordered_map<string, char>& codes) {
    string decodedString;
    string currentCode;

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜ 4 ˜˜˜˜˜˜
    size_t targetLength = 0;
    for (int i = 0; i < 4; ++i) {
        targetLength = (targetLength << 8) | encodedBytes[i];
    }

    int decodedLength = 0; // ˜˜˜˜˜˜˜, ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜
    int bitCount = (encodedBytes.size() - 4) * 8; // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜, ˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜
    // ˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜ ˜˜˜˜˜, ˜˜˜˜˜˜˜˜ ˜˜ ˜˜ ˜˜˜˜˜˜, ˜˜˜˜˜˜˜ ˜ 5-˜˜ ˜˜˜˜˜
    for (size_t i = 4 * 8; i < bitCount + 4 * 8; ++i) {
        int byteIndex = i / 8; // ˜˜˜˜˜˜ ˜˜˜˜˜, ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜
        int bitPosition = 7 - (i % 8); // ˜˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜ (˜˜ 7 ˜˜ 0)

        currentCode += ((encodedBytes[byteIndex] >> bitPosition) & 1) ? '1' : '0';

        // ˜˜˜˜˜˜˜˜˜, ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜ ˜˜˜
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
            decodedLength++; // ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜
            currentCode.clear(); // ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜
            // ˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜, ˜˜˜˜˜˜˜˜˜
            if (decodedLength == targetLength) {
                break;
            }
        } else if (currentCode.size() > 20) { // ˜˜˜˜˜˜˜ maxCodeLength ˜˜ 20 ˜˜˜ ˜˜˜˜˜˜˜
            currentCode.clear(); // ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜, ˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
        }
    }

    // ˜˜˜˜˜˜˜˜˜, ˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜ ˜ currentCode ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜
    if (!currentCode.empty() && currentCode.size() <= 20) { // ˜˜˜˜˜˜˜˜ ˜˜ 20 ˜˜˜˜
        // ˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜, ˜˜˜˜ ˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜
        if (codes.find(currentCode) != codes.end()) {
            decodedString += codes.at(currentCode);
        }
    }

    return decodedString;
}
   




// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜
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

// ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜
void fanoEncoding(const string& inputFile, const string& outputFile, const string& treeFile) {
    ifstream inFile(inputFile);
    if (!inFile) {
        cerr << "˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜." << endl;
        return;
    }

    string text = "", line;
    while (getline(inFile, line)) {
        text += line + '\n'; // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    }
    inFile.close();

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜
    unordered_map<char, double> probabilitiesMap = getProbabilities(text);
    
    // ˜˜˜˜˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜ ˜˜˜
    vector<pair<char, double>> probabilities(probabilitiesMap.begin(), probabilitiesMap.end());

    // ˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜
    Node* root = buildFanoTree(probabilities);
    printFanoTree(root);
    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜
    unordered_map<char, string> codes;
    generateCodes(root, codes);

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜
    vector<uint8_t> encodedBytes = encode(text, codes);

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜ ˜˜˜˜
    ofstream outFile(outputFile, ios::binary);
    outFile.write(reinterpret_cast<char*>(encodedBytes.data()), encodedBytes.size());
    outFile.close();

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜
    writeCodesToFile(codes, treeFile);
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
void fanoDecoding(const string& encodedFile, const string& treeFile, const string& outputFile) {
    ifstream inFile(encodedFile, ios::binary);
    if (!inFile) {
        cerr << "Íå óäàåòñÿ íàéòè ôàéë." << endl;
        return;
    }

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜˜ ˜˜˜˜˜
    vector<uint8_t> encodedBytes((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜
    int bitCount = 8 * encodedBytes.size(); // ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜
    int lastByteBits = bitCount % 8; // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜
    

    if (lastByteBits > 0) {
        // ˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜ ˜˜˜˜, ˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜
        bitCount -= 8 - lastByteBits; // ˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜
    }

    unordered_map<string, char> codes; // ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜
    ifstream treeFileIn(treeFile);
    if (!treeFileIn) {
        cerr << "Ôàéë ñ äåðåâîì íå íàéäåí." << endl;
        return;
    }

    string codeLine;
    int maxCodeLength = 0; // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜

    while (getline(treeFileIn, codeLine)) {
        if (codeLine.empty()) continue; // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜

        // ˜˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ RETURN ˜ ˜˜˜˜˜˜˜˜ ˜˜˜
        if (codeLine.find("RETURN") != string::npos) {
            // ˜˜˜˜ ˜˜˜˜˜˜ "RETURN", ˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ '\n'
            codeLine.replace(codeLine.find("RETURN"), 6, "\n"); // "RETURN" ˜˜˜˜˜ ˜˜˜˜˜ 6 ˜˜˜˜˜˜˜˜
        }
        /*
        else if(codeLine.find("TAB") != string::npos){
            codeLine.replace(codeLine.find("TAB"), 6, "\t");
        }*/

        size_t pos = codeLine.find('\t'); // ˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜
        if (pos != string::npos) {
            char symbol = codeLine.substr(0, pos)[0];
            cout << symbol << endl;
            string code = codeLine.substr(pos + 1);
            codes[code] = symbol; // ˜˜˜˜˜˜˜˜˜ ˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
            // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜
            maxCodeLength = max(maxCodeLength, static_cast<int>(code.length()));
        }
    }

    treeFileIn.close();

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    string decodedString = decode(encodedBytes, codes);

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜ ˜˜˜˜
    ofstream outFile(outputFile);

    if (!outFile) {
        cerr << "Íå óäàëîñü çàïèñàòü ôàéë." << endl;
        return;
    }

    outFile << decodedString;
    outFile.close();
}

// ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
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
    cout << "Âûáåðèòå ðåæèì: 1 - Êîäèðîâêà, 2 - Ðàñêîäèðîâêà, 0 - Âûõîä" << endl;
    cin >> regime;
    if(regime == 1 || regime == 2 || regime == 0){
        break;
    }
    } while(true);
    if(regime == 1){
        do{
        cout << "Ââåäèòå íàçâàíèå ôàéëà:" << endl;
        cin >> inputFile;
        }while(inputFile == "");
        encodedFile = "encoded.bin";
        fanoEncoding(inputFile, encodedFile, treeFile);
    }
    else if(regime == 2){
    do{
        cout << "Ââåäèòå íàçâàíèå çàêîäèðîâàííîãî ôàéëà" << endl;
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