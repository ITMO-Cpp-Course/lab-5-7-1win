#pragma once

#include "InvertedIndex.hpp"
#include "Result.hpp"

class IndexStore;

class UpdateTransaction
{
  public:
    explicit UpdateTransaction(IndexStore& store, InvertedIndex workingCopy);

    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;

    UpdateTransaction(UpdateTransaction&& other) noexcept;
    UpdateTransaction& operator=(UpdateTransaction&&) = delete;

    ~UpdateTransaction();

    Result<void> addDocument(Document doc);
    Result<void> removeDocument(Document::Id id);

    Result<void> commit();
    void rollback() noexcept;

    [[nodiscard]] bool isCommitted() const noexcept
    {
        return finished_;
    }

  private:
    IndexStore& store_;
    InvertedIndex workingCopy_;

    bool finished_ = false;
};
