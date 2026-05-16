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

    [[nodiscard]] Result<std::vector<SearchResult>> search(const std::string& word) const;
    [[nodiscard]] Result<std::size_t> wordCount(Document::Id id, const std::string& word) const;
    [[nodiscard]] Result<std::size_t> size() const noexcept;
    [[nodiscard]] Result<bool> contains(Document::Id id) const noexcept;

    [[nodiscard]] Result<UpdateTransaction> beginTransaction();

    void releaseTransaction() noexcept;

private:
    InvertedIndex index_;

    bool hasActiveTransaction_ = false;

    friend class UpdateTransaction;
};
