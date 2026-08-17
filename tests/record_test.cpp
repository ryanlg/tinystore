#include <tinystore/record.hpp>

#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("Encode and decode a record") {
    tinystore::Record rec = {
        .op = tinystore::Operation::PUT,
        .key = "key",
        .value = "value",
    };

    const auto encode_result = tinystore::encode_record(rec);
    REQUIRE(encode_result);

    const auto encoded = *encode_result;
    const auto decode_result = tinystore::decode_record(encoded);

    REQUIRE(decode_result);
    const auto decoded = *decode_result;

    REQUIRE(rec.op == decoded.op);
    REQUIRE(rec.key == decoded.key);
    REQUIRE(rec.value == decoded.value);
}
