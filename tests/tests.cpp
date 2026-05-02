#include "Document.hpp"
#include "DocumentBuilder.hpp"
#include "InvertedIndex.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("DocumentBuilder: unique IDs and move semantics", "[builder]")
{
    DocumentBuilder builder;
    auto doc1 = builder.setName("Doc1").setText("Hello world").build();
    auto doc2 = builder.setName("Doc2").setText("Hello C++").build();

    REQUIRE(doc1.id() != doc2.id());
    REQUIRE(doc1.name() == "Doc1");
    REQUIRE(doc2.name() == "Doc2");
}

TEST_CASE("DocumentBuilder: tokenize ASCII text", "[builder]")
{
    auto tokens = DocumentBuilder::tokenize("Hello, World! 123 test.");
    REQUIRE(tokens == std::vector<std::string>{"hello", "world", "123", "test"});

    auto empty = DocumentBuilder::tokenize("... !!! ---");
    REQUIRE(empty.empty());
}

TEST_CASE("DocumentBuilder: toLower ASCII", "[builder]")
{
    REQUIRE(DocumentBuilder::toLower("ABC123xyz") == "abc123xyz");
    REQUIRE(DocumentBuilder::toLower("Already Lower") == "already lower");
}

TEST_CASE("InvertedIndex: add and search", "[index]")
{
    InvertedIndex index;
    DocumentBuilder builder;

    auto doc = builder.setName("Test").setText("Hello hello world").build();
    index.addDocument(std::move(doc));

    auto results = index.search("hello");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].count == 2);
}

TEST_CASE("InvertedIndex: remove document", "[index]")
{
    InvertedIndex index;
    DocumentBuilder builder;

    auto doc = builder.setName("Test").setText("Hello world").build();
    auto id = doc.id();
    index.addDocument(std::move(doc));

    REQUIRE(index.contains(id));
    REQUIRE(index.removeDocument(id));
    REQUIRE_FALSE(index.contains(id));
    REQUIRE(index.search("hello").empty());
}

TEST_CASE("InvertedIndex: wordCount & case-insensitivity", "[index]")
{
    InvertedIndex index;
    DocumentBuilder builder;

    auto doc = builder.setName("Cpp").setText("C++ is great. C++ is fast.").build();
    auto id = doc.id();
    index.addDocument(std::move(doc));

    REQUIRE(index.wordCount(id, "c") == 2);
    REQUIRE(index.wordCount(id, "is") == 2);
    REQUIRE(index.wordCount(id, "cpp") == 0);
    REQUIRE(index.wordCount(999, "hello") == 0);
}

TEST_CASE("InvertedIndex: sorted search results", "[index]")
{
    InvertedIndex index;
    DocumentBuilder builder;

    auto d1 = builder.setName("A").setText("hello hello hello").build();
    auto d2 = builder.setName("B").setText("hello world").build();
    auto id1 = d1.id();

    index.addDocument(std::move(d1));
    index.addDocument(std::move(d2));

    auto results = index.search("hello");
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].docId == id1);
    REQUIRE(results[0].count == 3);
    REQUIRE(results[1].count == 1);
}