#include "IndexStore.hpp"
#include "UpdateTransaction.hpp"

Result<void> IndexStore::addDocument(Document doc)
{
    if (index_.contains(doc.id()))
        return std::unexpected(IndexError::DuplicateDocument);

    index_.addDocument(std::move(doc));
    return {};
}

Result<void> IndexStore::removeDocument(Document::Id id)
{
    if (!index_.removeDocument(id))
        return std::unexpected(IndexError::DocumentNotFound);

    return {};
}

std::vector<SearchResult> IndexStore::search(const std::string& word) const
{
    return index_.search(word);
}

std::size_t IndexStore::wordCount(Document::Id id, const std::string& word) const
{
    return index_.wordCount(id, word);
}

std::size_t IndexStore::size() const noexcept
{
    return index_.size();
}

bool IndexStore::contains(Document::Id id) const noexcept
{
    return index_.contains(id);
}

UpdateTransaction IndexStore::beginTransaction()
{
    return UpdateTransaction{*this};
}
