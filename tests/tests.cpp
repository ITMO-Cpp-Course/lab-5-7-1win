#include <catch2/catch_all.hpp>
#include "IndexStore.hpp"
#include "UpdateTransaction.hpp"
#include "DocumentBuilder.hpp"

TEST_CASE("IndexStore: add and contains", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("a").setText("hello world").build();
    auto id = doc.id();

    REQUIRE(store.addDocument(std::move(doc)).has_value());
    REQUIRE(store.contains(id).value() == true);
    REQUIRE(store.size().value() == 1);
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
    REQUIRE(store.size().value() == 1);
}

TEST_CASE("IndexStore: remove existing document", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("a").setText("hello").build();
    auto id = doc.id();

    store.addDocument(std::move(doc));
    REQUIRE(store.removeDocument(id).has_value());
    REQUIRE(store.contains(id).value() == false);
    REQUIRE(store.size().value() == 0);
}

TEST_CASE("IndexStore: remove non-existent returns error", "[store]")
{
    IndexStore store;
    auto result = store.removeDocument(9999u);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == IndexError::DocumentNotFound);
}

TEST_CASE("IndexStore: search", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    store.addDocument(b.setName("doc1").setText("cat dog").build());
    store.addDocument(b.setName("doc2").setText("cat fish").build());

    auto results = store.search("cat");
    REQUIRE(results.has_value());
    REQUIRE(results.value().size() == 2);
}

TEST_CASE("IndexStore: wordCount", "[store]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("d").setText("the cat and the cat").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    REQUIRE(store.wordCount(id, "cat").value() == 2);
    REQUIRE(store.wordCount(id, "the").value() == 2);
    REQUIRE(store.wordCount(id, "dog").value() == 0);
}

TEST_CASE("Transaction: commit applies changes", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    REQUIRE(tx.addDocument(b.setName("doc1").setText("hello").build()).has_value());
    REQUIRE(tx.addDocument(b.setName("doc2").setText("world").build()).has_value());
    REQUIRE(tx.commit().has_value());

    REQUIRE(store.size().value() == 2);
}

TEST_CASE("Transaction: commit remove applies to store", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    REQUIRE(tx.removeDocument(id).has_value());
    REQUIRE(tx.commit().has_value());

    REQUIRE(store.contains(id).value() == false);
    REQUIRE(store.size().value() == 0);
}


TEST_CASE("Transaction: no commit means rollback", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    {
        auto txResult = store.beginTransaction();
        REQUIRE(txResult.has_value());
        txResult.value().addDocument(b.setName("doc").setText("hello").build());
    }

    REQUIRE(store.size().value() == 0);
}

TEST_CASE("Transaction: explicit rollback does not affect store", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    tx.removeDocument(id);
    tx.rollback();

    REQUIRE(store.contains(id).value() == true);
    REQUIRE(store.size().value() == 1);
}

TEST_CASE("Transaction: store unchanged if tx destroyed after partial changes", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc = b.setName("original").setText("data").build();
    auto id = doc.id();
    store.addDocument(std::move(doc));

    {
        auto txResult = store.beginTransaction();
        REQUIRE(txResult.has_value());
        auto& tx = txResult.value();

        tx.removeDocument(id);
        auto res = tx.removeDocument(9999u);
        REQUIRE_FALSE(res.has_value());
        REQUIRE(res.error() == IndexError::DocumentNotFound);
    }

    REQUIRE(store.contains(id).value() == true);
    REQUIRE(store.size().value() == 1);
}

TEST_CASE("Transaction: double commit returns TransactionAlreadyFinished", "[transaction]")
{
    IndexStore store;
    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    REQUIRE(tx.commit().has_value());

    auto second = tx.commit();
    REQUIRE_FALSE(second.has_value());
    REQUIRE(second.error() == IndexError::TransactionAlreadyFinished);
}

TEST_CASE("Transaction: commit after rollback returns TransactionAlreadyFinished", "[transaction]")
{
    IndexStore store;
    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    tx.rollback();

    auto result = tx.commit();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == IndexError::TransactionAlreadyFinished);
}

TEST_CASE("Transaction: add after commit returns TransactionAlreadyFinished", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    tx.commit();

    auto res = tx.addDocument(b.setName("doc").setText("late").build());
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error() == IndexError::TransactionAlreadyFinished);
}


TEST_CASE("Transaction: two parallel transactions are forbidden", "[transaction]")
{
    IndexStore store;

    auto tx1Result = store.beginTransaction();
    REQUIRE(tx1Result.has_value());

    auto tx2Result = store.beginTransaction();
    REQUIRE_FALSE(tx2Result.has_value());
    REQUIRE(tx2Result.error() == IndexError::ActiveTransactionExists);
}

TEST_CASE("Transaction: new transaction allowed after commit", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    {
        auto txResult = store.beginTransaction();
        REQUIRE(txResult.has_value());
        txResult.value().addDocument(b.setName("doc1").setText("hello").build());
        txResult.value().commit();
    }

    auto tx2Result = store.beginTransaction();
    REQUIRE(tx2Result.has_value());
    tx2Result.value().addDocument(b.setName("doc2").setText("world").build());
    REQUIRE(tx2Result.value().commit().has_value());

    REQUIRE(store.size().value() == 2);
}

TEST_CASE("Transaction: new transaction allowed after rollback", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    {
        auto txResult = store.beginTransaction();
        REQUIRE(txResult.has_value());
        txResult.value().rollback();
    }

    auto tx2Result = store.beginTransaction();
    REQUIRE(tx2Result.has_value());
    tx2Result.value().addDocument(b.setName("doc").setText("text").build());
    REQUIRE(tx2Result.value().commit().has_value());

    REQUIRE(store.size().value() == 1);
}

TEST_CASE("Transaction: new transaction allowed after destructor rollback", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    {
        auto txResult = store.beginTransaction();
        REQUIRE(txResult.has_value());
    }

    auto tx2Result = store.beginTransaction();
    REQUIRE(tx2Result.has_value());
}


TEST_CASE("Transaction: changes not visible before commit", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;

    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    auto doc = b.setName("doc").setText("hello").build();
    auto id = doc.id();
    tx.addDocument(std::move(doc));

    REQUIRE(store.contains(id).value() == false);
    REQUIRE(store.size().value() == 0);

    tx.commit();

    REQUIRE(store.contains(id).value() == true);
}

TEST_CASE("Transaction: duplicate in working copy returns error", "[transaction]")
{
    IndexStore store;
    DocumentBuilder b;
    auto doc1 = b.setName("doc").setText("hello").build();
    Document doc2{doc1.id(), "doc_copy", "world"};

    auto txResult = store.beginTransaction();
    REQUIRE(txResult.has_value());
    auto& tx = txResult.value();

    REQUIRE(tx.addDocument(std::move(doc1)).has_value());

    auto res = tx.addDocument(std::move(doc2));
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error() == IndexError::DuplicateDocument);
}
