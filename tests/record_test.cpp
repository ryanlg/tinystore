#include <tinystore/record.hpp>

#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("Encode and decode a record") {
    tinystore::Record rec = {
        .op = tinystore::Operation::PUT,
        .key = "key",
        .value = "value",
    };

    auto encoded = tinystore::encode_record(rec);
    auto decoded = tinystore::decode_record(encoded);
    REQUIRE(rec.op == decoded.op);
    REQUIRE(rec.key == decoded.key);
    REQUIRE(rec.value == decoded.value);
}
