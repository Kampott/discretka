void generateCodes(Node* root, const string& code, unordered_map<char, string>& codes) {
    if (!root) return;
    
    if (root->symbol != '\0') { // Если это листовой узел
        codes[root->symbol] = code;
    }
    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}