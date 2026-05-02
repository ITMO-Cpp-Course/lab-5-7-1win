#include "DocumentBuilder.hpp"
#include <algorithm>
#include <cctype>


std::vector<std::string> DocumentBuilder::tokenize(const std::string &text) {
    std::vector<std::string> tokens;
    tokens.reserve(text.size() / 5);

    std::string token;
    token.reserve(32);

    for (char ch: text) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            token += ch;
        } else {
            if (!token.empty()) {
                tokens.push_back(toLower(std::move(token)));
                token = {};
            }
        }
    }

    if (!token.empty()) {
        tokens.push_back(toLower(std::move(token)));
    }

    return tokens;
}


std::string DocumentBuilder::toLower(std::string word) {
    std::transform(word.begin(), word.end(), word.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return word;
}

