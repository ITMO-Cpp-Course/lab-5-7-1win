#include "IndexStore.hpp"
#include "UpdateTransaction.hpp"

Result<void> IndexStore::addDocument(Document doc)
{
    if (hasActiveTransaction_) {
        return std::unexpected(IndexError::ActiveTransactionExists);
    }

    if (index_.contains(doc.id()))
        return std::unexpected(IndexError::DuplicateDocument);

    index_.addDocument(std::move(doc));
    return {};
}

Result<void> IndexStore::removeDocument(Document::Id id)
{
    if (hasActiveTransaction_) {
        return std::unexpected(IndexError::ActiveTransactionExists);
    }
    if (!index_.removeDocument(id))
        return std::unexpected(IndexError::DocumentNotFound);

    return {};
}

Result<std::vector<SearchResult>> IndexStore::search(const std::string& word) const
{
    return index_.search(word);
}

Result<std::size_t> IndexStore::wordCount(Document::Id id, const std::string& word) const
{
    return index_.wordCount(id, word);
}

Result<std::size_t> IndexStore::size() const noexcept
{
    return index_.size();
}

Result<bool> IndexStore::contains(Document::Id id) const noexcept
{
    return index_.contains(id);
}

Result<UpdateTransaction> IndexStore::beginTransaction()
{
    if (hasActiveTransaction_)
        return std::unexpected(IndexError::ActiveTransactionExists);

    try
    {
        InvertedIndex copy = index_;
        hasActiveTransaction_ = true;
        return UpdateTransaction{*this, std::move(copy)};
    }
    catch (...)
    {
        return std::unexpected(IndexError::TransactionCopyFailed);
    }
}

void IndexStore::releaseTransaction() noexcept
{
    hasActiveTransaction_ = false;
}
