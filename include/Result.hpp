#pragma once

#include <expected>
#include <string>

enum class IndexError
{
    DocumentNotFound,
    DuplicateDocument,
    TransactionAlreadyFinished,
    TransactionCopyFailed,
    TransactionCommitFailed,
    ActiveTransactionExists,
};

inline std::string toString(IndexError err)
{
    switch (err)
    {
    case IndexError::DocumentNotFound:           return "Document not found";
    case IndexError::DuplicateDocument:          return "Document with this ID already exists";
    case IndexError::TransactionAlreadyFinished: return "Transaction already finished";
    case IndexError::TransactionCopyFailed:      return "Failed to copy index for transaction";
    case IndexError::TransactionCommitFailed:    return "Failed to apply transaction changes";
    case IndexError::ActiveTransactionExists:    return "Another transaction is already active";
    }
    return "Unknown error";
}

template <typename T>
using Result = std::expected<T, IndexError>;
