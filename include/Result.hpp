#pragma once
#include <expected>
#include <string>

enum class IndexError
{
    DocumentNotFound,
    DuplicateDocument,
    TransactionFailed,
};

inline std::string toString(IndexError err)
{
    switch (err)
    {
    case IndexError::DocumentNotFound:
        return "Document not found";
    case IndexError::DuplicateDocument:
        return "Document with this ID already exists";
    case IndexError::TransactionFailed:
        return "Transaction failed";
    }
    return "Unknown error";
}

template <typename T> using Result = std::expected<T, IndexError>;