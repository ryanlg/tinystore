#include <tinystore/record.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <tinystore/limits.hpp>

namespace {

constexpr std::array<std::byte, 11> encoded_record{
    std::byte{0x01},                  // PUT
    std::byte{0x03}, std::byte{0x00}, // key length
    std::byte{0x01}, std::byte{0x00}, // value length
    std::byte{0x00}, std::byte{0x00}, std::byte{'k'}, std::byte{0x00}, std::byte{'y'}, // key
    std::byte{'v'},                                                                    // value
};

std::vector<std::byte> record_bytes() { return {encoded_record.begin(), encoded_record.end()}; }

} // namespace

TEST_CASE("Record encoding matches the specified byte layout", "[Record]") {
    const tinystore::Record record{
        .op = tinystore::Operation::PUT,
        .key = std::string{"k\0y", 3},
        .value = "v",
    };

    const auto result = tinystore::encode_record(record);

    REQUIRE(result);
    REQUIRE(*result == record_bytes());
}

TEST_CASE("Record decoding handles embedded NUL and empty fields", "[Record]") {
    const auto decoded = tinystore::decode_record(record_bytes());

    REQUIRE(decoded);
    REQUIRE(decoded->op == tinystore::Operation::PUT);
    REQUIRE(decoded->key == std::string{"k\0y", 3});
    REQUIRE(decoded->value == "v");

    const tinystore::Record empty{
        .op = tinystore::Operation::DELETE,
        .key = "",
        .value = "",
    };
    const auto encoded_empty = tinystore::encode_record(empty);
    REQUIRE(encoded_empty);

    const auto decoded_empty = tinystore::decode_record(*encoded_empty);
    REQUIRE(decoded_empty);
    REQUIRE(decoded_empty->op == empty.op);
    REQUIRE(decoded_empty->key.empty());
    REQUIRE(decoded_empty->value.empty());
}

TEST_CASE("Record decoding rejects every truncated prefix", "[Record]") {
    for (std::size_t length = 0; length < encoded_record.size(); ++length) {
        INFO("prefix length: " << length);
        const auto prefix_span = std::span{encoded_record}.first(length);
        const std::vector<std::byte> prefix{prefix_span.begin(), prefix_span.end()};

        const auto result = tinystore::decode_record(prefix);

        REQUIRE_FALSE(result);
        REQUIRE(result.error().code == tinystore::DecodeErrorCode::unexpected_end);
    }
}

TEST_CASE("Record decoding rejects malformed input", "[Record]") {
    SECTION("trailing byte") {
        auto bytes = record_bytes();
        bytes.push_back(std::byte{0xFF});

        const auto result = tinystore::decode_record(bytes);

        REQUIRE_FALSE(result);
        REQUIRE(result.error().code == tinystore::DecodeErrorCode::trailing_bytes);
        REQUIRE(result.error().available == bytes.size());
        REQUIRE(result.error().offset == encoded_record.size());
    }

    SECTION("unknown operation") {
        auto bytes = record_bytes();
        bytes.front() = std::byte{0xFF};

        REQUIRE_FALSE(tinystore::decode_record(bytes));
    }

    SECTION("declared value length exceeds available input") {
        const std::vector<std::byte> bytes{
            std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
        };

        const auto result = tinystore::decode_record(bytes);

        REQUIRE_FALSE(result);
        REQUIRE(result.error().code == tinystore::DecodeErrorCode::unexpected_end);
        REQUIRE(result.error().available == bytes.size());
        REQUIRE(result.error().offset == bytes.size());
        REQUIRE(result.error().read_size == tinystore::limits::max_value_length);
    }
}

TEST_CASE("Record encoding enforces the key length limit", "[Record]") {
    tinystore::Record record{
        .op = tinystore::Operation::PUT,
        .key = std::string(static_cast<std::size_t>(tinystore::limits::max_key_length), 'k'),
        .value = "",
    };

    REQUIRE(tinystore::encode_record(record));

    record.key.push_back('k');
    const auto result = tinystore::encode_record(record);

    REQUIRE_FALSE(result);
    REQUIRE(result.error().code == tinystore::EncodeErrorCode::field_too_long);
    REQUIRE(result.error().field_name == "key");
}
