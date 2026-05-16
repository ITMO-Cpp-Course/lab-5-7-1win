#include "UpdateTransaction.hpp"
#include "IndexStore.hpp"

UpdateTransaction::UpdateTransaction(IndexStore& store, InvertedIndex workingCopy)
    : store_(store), workingCopy_(std::move(workingCopy))
{
}

UpdateTransaction::UpdateTransaction(UpdateTransaction&& other) noexcept
    : store_(other.store_), workingCopy_(std::move(other.workingCopy_)), finished_(other.finished_)
{
    other.finished_ = true;
}

UpdateTransaction::~UpdateTransaction()
{
    if (!finished_)
        rollback();
}

Result<void> UpdateTransaction::addDocument(Document doc)
{
    if (finished_)
        return std::unexpected(IndexError::TransactionAlreadyFinished);

    if (workingCopy_.contains(doc.id()))
        return std::unexpected(IndexError::DuplicateDocument);

    try
    {
        InvertedIndex temp = workingCopy_;
        temp.addDocument(std::move(doc));
        workingCopy_ = std::move(temp);
    }
    catch (...)
    {
        return std::unexpected(IndexError::TransactionCopyFailed);
    }

    return {};
}

Result<void> UpdateTransaction::removeDocument(Document::Id id)
{
    if (finished_)
        return std::unexpected(IndexError::TransactionAlreadyFinished);

    if (!workingCopy_.removeDocument(id))
        return std::unexpected(IndexError::DocumentNotFound);

    return {};
}

Result<void> UpdateTransaction::commit()
{
    if (finished_)
        return std::unexpected(IndexError::TransactionAlreadyFinished);

    try
    {
        store_.index_ = std::move(workingCopy_);
    }
    catch (...)
    {
        return std::unexpected(IndexError::TransactionCommitFailed);
    }

    finished_ = true;
    store_.releaseTransaction();
    return {};
}

void UpdateTransaction::rollback() noexcept
{
    finished_ = true;
    store_.releaseTransaction();
}
