#pragma once

#include <cstdint>
#include <string>

class Document {
public:
    using Id = uint64_t;

    Document(Id id, std::string name, std::string text)
        : id_(id), name_(std::move(name)), text_(std::move(text)) {}

    Document(const Document&) = default;
    Document(Document&&) = default;
    Document& operator=(const Document&) = default;
    Document& operator=(Document&&) = default;
    ~Document() = default;

    [[nodiscard]] Id id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::string& text() const noexcept { return text_; }

    [[nodiscard]] friend bool operator==(const Document& lhs, const Document& rhs) noexcept {
        return lhs.id_ == rhs.id_ && lhs.name_ == rhs.name_ && lhs.text_ == rhs.text_;
    }

private:
    Id id_;
    std::string name_;
    std::string text_;
};