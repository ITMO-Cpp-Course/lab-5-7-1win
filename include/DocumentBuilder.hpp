#pragma once

#include "Document.hpp"
#include <string>
#include <vector>

class DocumentBuilder {
public:
    DocumentBuilder() = default;

    DocumentBuilder &setName(std::string name) {
        name_ = std::move(name);
        return *this;
    }

    DocumentBuilder &setText(std::string text) {
        text_ = std::move(text);
        return *this;
    }

    [[nodiscard]] Document build() {
        return Document{nextId_++, std::move(name_), std::move(text_)};
    }


    [[nodiscard]] static std::vector<std::string> tokenize(const std::string &text);


    [[nodiscard]] static std::string toLower(std::string word);

private:
    std::string name_;
    std::string text_;


    Document::Id nextId_ = 1;
};

