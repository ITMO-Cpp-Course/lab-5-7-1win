#include "UpdateTransaction.hpp"
#include "IndexStore.hpp"

UpdateTransaction::UpdateTransaction(IndexStore& store) : store_(store), workingCopy_(store.index_) {}

UpdateTransaction::UpdateTransaction(UpdateTransaction&& other) noexcept
    : store_(other.store_), workingCopy_(std::move(other.workingCopy_)), committed_(other.committed_)
{
    other.committed_ = true;
}

Result<void> UpdateTransaction::addDocument(Document doc)
{
    if (committed_)
        return std::unexpected(IndexError::TransactionFailed);

    if (workingCopy_.contains(doc.id()))
        return std::unexpected(IndexError::DuplicateDocument);

    workingCopy_.addDocument(std::move(doc));
    return {};
}

Result<void> UpdateTransaction::removeDocument(Document::Id id)
{
    if (committed_)
        return std::unexpected(IndexError::TransactionFailed);

    if (!workingCopy_.removeDocument(id))
        return std::unexpected(IndexError::DocumentNotFound);

    return {};
}

Result<void> UpdateTransaction::commit()
{
    if (committed_)
        return std::unexpected(IndexError::TransactionFailed);

    store_.index_ = std::move(workingCopy_);
    committed_ = true;
    return {};
}

void UpdateTransaction::rollback() noexcept
{
    committed_ = true;
}
