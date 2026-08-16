#include <tinystore/backends/inmemory.hpp>
#include <tinystore/store.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("a new store does not contain a key") {
    tinystore::Store store(tinystore::StoreOptions{tinystore::StoreBackendType::InMemory});

    const auto result = store.get("missing");

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("put stores a value") {
    tinystore::Store store(std::make_unique<tinystore::InMemoryBackend>());

    store.put("language", "C++");

    const auto result = store.get("language");

    REQUIRE(result.has_value());
    REQUIRE(*result == "C++");
}

TEST_CASE("put replaces an existing value") {
    tinystore::Store store(std::make_unique<tinystore::InMemoryBackend>());

    store.put("status", "old");
    store.put("status", "new");

    const auto result = store.get("status");

    REQUIRE(result.has_value());
    REQUIRE(*result == "new");
}
