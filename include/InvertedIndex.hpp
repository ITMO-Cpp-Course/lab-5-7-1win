#pragma once

#include "Document.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct SearchResult
{
    Document::Id docId;
    std::string docName;
    std::size_t count;
};

class InvertedIndex
{
  public:
    InvertedIndex() = default;

    void addDocument(Document doc);

    [[nodiscard]] bool removeDocument(Document::Id id);

    [[nodiscard]] std::vector<SearchResult> search(const std::string& word) const;

    [[nodiscard]] std::size_t wordCount(Document::Id id, const std::string& word) const;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return documents_.size();
    }

    [[nodiscard]] bool contains(Document::Id id) const noexcept
    {
        return documents_.count(id) > 0;
    }

    [[nodiscard]] const Document* getDocument(Document::Id id) const;

  private:
    std::unordered_map<Document::Id, Document> documents_;
    std::unordered_map<std::string, std::unordered_map<Document::Id, std::size_t>> index_;
    std::unordered_map<Document::Id, std::unordered_set<std::string>> docWords_;
};