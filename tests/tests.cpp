#include "DocumentBuilder.hpp"
#include "IndexStore.hpp"
#include "UpdateTransaction.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("IndexStore: add and contains", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("a").setText("hello world").build();
    auto id = doc.id();
    auto result = store.addDocument(std::move(doc));

    REQUIRE(result.has_value());
    REQUIRE(store.contains(id));
    REQUIRE(store.size() == 1);
}

TEST_CASE("IndexStore: add duplicate returns error", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc1 = b.setName("a").setText("hello").build();
    Document doc2{doc1.id(), "b", "world"};

    REQUIRE(store.addDocument(std::move(doc1)).has_value());

    auto result = store.addDocument(std::move(doc2));

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == IndexError::DuplicateDocument);
    REQUIRE(store.size() == 1);
}

TEST_CASE("IndexStore: remove existing document", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("a").setText("hello").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));
    auto result = store.removeDocument(id);

    REQUIRE(result.has_value());
    REQUIRE_FALSE(store.contains(id));
    REQUIRE(store.size() == 0);
}

TEST_CASE("IndexStore: remove non-existent returns error", "[store]")
{
    IndexStore store;
    auto result = store.removeDocument(9999u);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == IndexError::DocumentNotFound);
}

TEST_CASE("IndexStore: search delegates to index", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    store.addDocument(b.setName("doc1").setText("cat dog").build());
    store.addDocument(b.setName("doc2").setText("cat fish").build());

    auto results = store.search("cat");
    REQUIRE(results.size() == 2);
}

TEST_CASE("IndexStore: wordCount delegates to index", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("d").setText("the cat and the cat").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    REQUIRE(store.wordCount(id, "cat") == 2);
    REQUIRE(store.wordCount(id, "the") == 2);
    REQUIRE(store.wordCount(id, "dog") == 0);
}

TEST_CASE("Transaction: commit applies changes", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto tx = store.beginTransaction();

    REQUIRE(tx.addDocument(b.setName("doc1").setText("hello").build()).has_value());
    REQUIRE(tx.addDocument(b.setName("doc2").setText("world").build()).has_value());
    REQUIRE(tx.commit().has_value());
    REQUIRE(store.size() == 2);
}

TEST_CASE("Transaction: commit remove applies to store", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));
    auto tx = store.beginTransaction();

    REQUIRE(tx.removeDocument(id).has_value());
    REQUIRE(tx.commit().has_value());

    REQUIRE_FALSE(store.contains(id));
    REQUIRE(store.size() == 0);
}

TEST_CASE("Transaction: no commit means rollback", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    {
        auto tx = store.beginTransaction();
        tx.addDocument(b.setName("doc").setText("hello").build());
    }

    REQUIRE(store.size() == 0);
}

TEST_CASE("Transaction: rollback does not affect store", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    auto tx = store.beginTransaction();
    tx.removeDocument(id);
    tx.rollback();

    REQUIRE(store.contains(id));
    REQUIRE(store.size() == 1);
}

TEST_CASE("Transaction: store unchanged if tx goes out of scope after error", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("original").setText("data").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    {
        auto tx = store.beginTransaction();
        tx.removeDocument(id);
        auto res = tx.removeDocument(9999u);
        REQUIRE_FALSE(res.has_value());
        REQUIRE(res.error() == IndexError::DocumentNotFound);
    }

    REQUIRE(store.contains(id));
    REQUIRE(store.size() == 1);
}

TEST_CASE("Transaction: double commit returns error", "[transaction]")
{
    IndexStore store;
    auto tx = store.beginTransaction();

    REQUIRE(tx.commit().has_value());

    auto second = tx.commit();
    REQUIRE_FALSE(second.has_value());
    REQUIRE(second.error() == IndexError::TransactionFailed);
}

TEST_CASE("Transaction: add after commit returns error", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto tx = store.beginTransaction();
    tx.commit();

    auto res = tx.addDocument(b.setName("doc").setText("late").build());
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error() == IndexError::TransactionFailed);
}

TEST_CASE("Transaction: changes in tx not visible in store before commit", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    auto tx = store.beginTransaction();
    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    tx.addDocument(std::move(doc));

    REQUIRE_FALSE(store.contains(id));
    REQUIRE(store.size() == 0);

    tx.commit();

    REQUIRE(store.contains(id));
}

TEST_CASE("Transaction: duplicate in working copy returns error", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc1 = b.setName("doc").setText("hello").build();
    Document doc2{doc1.id(), "doc_copy", "world"};

    auto tx = store.beginTransaction();
    REQUIRE(tx.addDocument(std::move(doc1)).has_value());

    auto res = tx.addDocument(std::move(doc2));
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error() == IndexError::DuplicateDocument);
}
