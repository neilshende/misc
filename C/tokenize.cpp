#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> tokenizeString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start)); // Add the last token
    return tokens;
}

int main() {
    std::string myString = "hello,,,,world,how,are,you";
    char delimiter = ',';

    std::vector<std::string> tokens = tokenizeString(myString, delimiter);

    for (const auto& token : tokens) {
        if (token.length()) std::cout << token << std::endl;
    }

    return 0;
}
