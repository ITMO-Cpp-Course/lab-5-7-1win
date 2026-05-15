#pragma once

#include "InvertedIndex.hpp"
#include "Result.hpp"

class IndexStore;

class UpdateTransaction
{
  public:
    explicit UpdateTransaction(IndexStore& store);

    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;

    UpdateTransaction(UpdateTransaction&& other) noexcept;
    UpdateTransaction& operator=(UpdateTransaction&&) = delete;

    ~UpdateTransaction() = default;

    Result<void> addDocument(Document doc);
    Result<void> removeDocument(Document::Id id);

    Result<void> commit();

    void rollback() noexcept;

    [[nodiscard]] bool isCommitted() const noexcept
    {
        return committed_;
    }

  private:
    IndexStore& store_;
    InvertedIndex workingCopy_;

    bool committed_ = false;
};
