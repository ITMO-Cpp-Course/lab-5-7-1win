#pragma once

#include "InvertedIndex.hpp"
#include "Result.hpp"

class UpdateTransaction;

class IndexStore
{
  public:
    IndexStore() = default;

    IndexStore(const IndexStore&) = delete;
    IndexStore& operator=(const IndexStore&) = delete;
    IndexStore(IndexStore&&) = default;
    IndexStore& operator=(IndexStore&&) = default;

    Result<void> addDocument(Document doc);

    Result<void> removeDocument(Document::Id id);

    [[nodiscard]] std::vector<SearchResult> search(const std::string& word) const;

    [[nodiscard]] std::size_t wordCount(Document::Id id, const std::string& word) const;

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool contains(Document::Id id) const noexcept;

    [[nodiscard]] UpdateTransaction beginTransaction();

  private:
    InvertedIndex index_;
    friend class UpdateTransaction;
};
