#include "InvertedIndex.hpp"
#include "DocumentBuilder.hpp"
#include <algorithm>

void InvertedIndex::addDocument(Document doc)
{
    Document::Id id = doc.id();
    auto tokens = DocumentBuilder::tokenize(doc.text());

    documents_.emplace(id, std::move(doc));

    for (auto& token : tokens)
    {
        index_[token][id]++;
        docWords_[id].insert(std::move(token));
    }
}

bool InvertedIndex::removeDocument(Document::Id id)
{
    auto docIt = documents_.find(id);
    if (docIt == documents_.end())
    {
        return false;
    }

    auto wordsIt = docWords_.find(id);
    if (wordsIt != docWords_.end())
    {
        for (const auto& word : wordsIt->second)
        {
            auto& docMap = index_[word];
            docMap.erase(id);
            if (docMap.empty())
            {
                index_.erase(word);
            }
        }
        docWords_.erase(wordsIt);
    }

    documents_.erase(docIt);
    return true;
}

std::vector<SearchResult> InvertedIndex::search(const std::string& word) const
{
    const std::string lowerWord = DocumentBuilder::toLower(word);

    auto it = index_.find(lowerWord);
    if (it == index_.end())
    {
        return {};
    }

    std::vector<SearchResult> results;
    results.reserve(it->second.size());

    for (const auto& [docId, count] : it->second)
    {
        results.push_back({docId, documents_.at(docId).name(), count});
    }

    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) { return a.count > b.count; });

    return results;
}

std::size_t InvertedIndex::wordCount(Document::Id id, const std::string& word) const
{
    const std::string lowerWord = DocumentBuilder::toLower(word);

    auto wordIt = index_.find(lowerWord);
    if (wordIt == index_.end())
        return 0;

    auto docIt = wordIt->second.find(id);
    if (docIt == wordIt->second.end())
        return 0;

    return docIt->second;
}

const Document* InvertedIndex::getDocument(Document::Id id) const
{
    auto it = documents_.find(id);
    return (it != documents_.end()) ? &it->second : nullptr;
}