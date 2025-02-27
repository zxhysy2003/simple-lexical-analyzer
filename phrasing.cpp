#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <fstream>
#include <sstream>

enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    OPERATOR,
    DELIMITER,
    ERROR
};

struct Token {
    TokenType type;
    std::string value;
    Token(TokenType t, std::string v) : type(t), value(v) {}
};

class Lexer {
private:
    std::string input;
    size_t pos = 0;
    std::unordered_map<std::string, int> keywords_map = {{"if", 11}, {"then", 12}, {"else", 13}, {"int", 14}, {"char", 15}, {"for", 16}};
    std::unordered_map<std::string, int> operators_map = {{"=", 21}, {">=", 22}, {"==", 23}, {"+", 24}, {"/", 25}, {"%", 26}, {"++", 27}};
    std::unordered_map<std::string, int> delimiters_map = {{"\"", 31}, {";", 32}};
    std::unordered_map<std::string, int> variables_map;
    std::unordered_map<std::string, int> constants_map;
    int variableCounter = 41;
    int constantCounter = 51;

    char peek() { return (pos < input.length()) ? input[pos] : '\0'; }
    void consume() { if (pos < input.length()) pos++; }

    void skipWhitespace() {
        while (isspace(peek())) consume();
    }

    friend int getWordIndex(const TokenType& type, Lexer& lexer, const std::string& word) {
        switch (type) {
            case TokenType::KEYWORD:
                return lexer.keywords_map[word];
            case TokenType::OPERATOR:
                return lexer.operators_map[word];
            case TokenType::DELIMITER:
                return lexer.delimiters_map[word];
            case TokenType::IDENTIFIER:
                return lexer.variables_map[word];
            case TokenType::CONSTANT:
                return lexer.constants_map[word];
            default:
                return -1;
        }
    }

public:
    Lexer(const std::string& str) : input(str) {}

    void insert_map(TokenType type, std::string value) {
        switch (type) {
            case TokenType::IDENTIFIER:
                if (variables_map.count(value) == 0) {
                    variables_map[value] = variableCounter++;
                }
                break;
            case TokenType::CONSTANT:
                if (constants_map.count(value) == 0) {
                    constants_map[value] = constantCounter++;
                }
                break;
            default:
                break;
        }
    }

    Token nextToken() {
        skipWhitespace();
        if (pos >= input.length()) return {TokenType::ERROR, ""};

        char curr = peek();
        
        // 处理标识符和关键字
        if (isalpha(curr)) {
            std::string ident;
            do {
                ident += curr;
                consume();
            } while (isalpha(curr = peek()));

            // 例如：if1 是非法标识符
            if (isdigit(curr = peek())) {
                do {
                    ident += curr;
                    consume();
                } while (isdigit(curr = peek()));
                return {TokenType::ERROR, ident};
            }

            if (ident.length() >= 10)
                return {TokenType::ERROR, ident};
                
            if (keywords_map.count(ident)) {
                return {TokenType::KEYWORD, ident};
            }
            
            insert_map(TokenType::IDENTIFIER, ident);
            
            return {TokenType::IDENTIFIER, ident};
        }
        
        // 处理常数
        if (isdigit(curr)) {
            std::string num;
            do {
                num += curr;
                consume();
            } while (isdigit(curr = peek()));
            
            // 例如：123a 是非法常数
            if (isalpha(curr = peek())) {
                do {
                    num += curr;
                    consume();
                } while (isalpha(curr = peek()) || isdigit(curr = peek()));
                return {TokenType::ERROR, num};
            }

            insert_map(TokenType::CONSTANT, num);
            
            return {TokenType::CONSTANT, num};
        }
        
        // 处理操作符
        if (std::string("=+/><%").find(curr) != std::string::npos) {
            std::string op(1, curr);
            consume();
            
            if (curr == '=' && peek() == '=') {
                consume();
                return {TokenType::OPERATOR, "=="};
            }
            if (curr == '>' && peek() == '=') {
                consume();
                return {TokenType::OPERATOR, ">="};
            }
            if (curr == '+' && peek() == '+') {
                consume();
                return {TokenType::OPERATOR, "++"};
            }
            if (curr == '<' && peek() == '=') {
                consume();
                return {TokenType::ERROR, "<="};
            }
            if (op == ">" || op == "<") return {TokenType::ERROR, op}; // 单独><非法

            return {TokenType::OPERATOR, op};
        }
        
        // 处理分隔符
        if (curr == '\"' || curr == ';') {
            consume();
            return {TokenType::DELIMITER, std::string(1, curr)};
        }
        
        // 处理非法字符
        std::string illegal(1, curr);
        consume();
        return {TokenType::ERROR, illegal};
    }

    void printTables(const std::string &filename) {
        std::ofstream output_file(filename, std::ios_base::app);
        if (!output_file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return;
        }
        output_file << "\n";
        output_file << "=== 变量表 ===" << std::endl;
        for (auto& var : variables_map) output_file << var.first << std::endl;
        
        output_file << "\n=== 常量表 ===" << std::endl;
        for (auto& num : constants_map) output_file << num.first << std::endl;
        
    }
};

std::string readFile(const std::string &filename) {
    // 创建输入文件流
    std::ifstream file(filename);

    // 检查文件是否成功打开
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return "";
    }

    // 使用 stringstream 读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();  // 将文件内容读取到 buffer 中

    // 返回文件内容作为字符串
    std::string result = buffer.str();
    // 将字符串中的字母都改为小写字母
    for (char &c : result) {
        if (isalpha(c)) {
            c = tolower(c);
        }
    }
    return result;
}

int main(int argc, char *argv[]) {

    if (argc != 3)
    {
        std::cerr << "Usage: cpp_program <input_file> <output_file>" << std::endl;
        return 1;
    }
    
    std::string code = readFile(argv[1]);

    Lexer lexer(code);
    std::ofstream output_file(argv[2], std::ios_base::app);
    if (!output_file.is_open()) {
        std::cerr << "无法打开文件: " << argv[2] << std::endl;
        return 1;
    }
    
    while (true) {
        Token token = lexer.nextToken();
        if (token.type == TokenType::ERROR && token.value.empty()) break;
        
        int index = getWordIndex(token.type, lexer, token.value);
        std::string result = (index == -1) ? "err" : std::to_string(index);
        output_file << token.value << "\t" << "(" << token.value << " , " << result << ")" << std::endl;
        
    }
    
    lexer.printTables(argv[2]);
    return 0;
}