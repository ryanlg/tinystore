#include <tinystore/record.hpp>

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <tinystore/limits.hpp>

namespace {

// clang-format off
constexpr const std::array<std::byte, 11> encoded_record{
    std::byte{0x01},                  // PUT
    std::byte{0x03}, std::byte{0x00}, // key length
    std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // value length
    std::byte{'k'}, std::byte{0x00}, std::byte{'y'}, // key
    std::byte{'v'},                                  // value
};
// clang-format on

const std::vector<std::byte> record_bytes() {
    return {encoded_record.begin(), encoded_record.end()};
}

const std::expected<tinystore::PutRecord, tinystore::DecodeError>
deserialize_record(const std::vector<std::byte> &bvec) {
    tinystore::ByteReader br(std::span{bvec});
    return tinystore::PutRecord::deserialize(br);
}

} // namespace

TEST_CASE("PutRecord encoding matches the specified byte layout", "[PutRecord]") {
    const tinystore::PutRecord record(std::string{"k\0y", 3}, "v");

    tinystore::ByteWriter bw;
    const auto result = record.serialize(bw);

    REQUIRE(result);
    REQUIRE(std::move(bw).finalize() == record_bytes());
}

TEST_CASE("PutRecord decoding handles embedded NUL and empty fields", "[PutRecord]") {
    const auto decoded = deserialize_record(record_bytes());

    REQUIRE(decoded);
    REQUIRE(decoded->op == tinystore::Operation::PUT);
    REQUIRE(decoded->key() == std::string{"k\0y", 3});
    REQUIRE(decoded->value() == "v");

    tinystore::ByteWriter bw;
    const tinystore::PutRecord empty("", "");
    const auto encoded_empty = empty.serialize(bw);
    REQUIRE(encoded_empty);

    const auto decoded_vector = std::move(bw).finalize();
    const auto decoded_empty = deserialize_record(decoded_vector);
    REQUIRE(decoded_empty);
    REQUIRE(decoded_empty->op == empty.op);
    REQUIRE(decoded_empty->key().empty());
    REQUIRE(decoded_empty->value().empty());
}

TEST_CASE("PutRecord decoding rejects every truncated prefix", "[PutRecord]") {
    for (std::size_t length = 0; length < encoded_record.size(); ++length) {
        INFO("prefix length: " << length);
        const auto prefix_span = std::span{encoded_record}.first(length);
        const std::vector<std::byte> prefix{prefix_span.begin(), prefix_span.end()};

        const auto result = deserialize_record(prefix);
        ;
        REQUIRE_FALSE(result);

        const auto error = result.error();
        REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));
    }
}

TEST_CASE("PutRecord decoding rejects malformed input", "[PutRecord]") {
    SECTION("unknown operation") {
        auto bytes = record_bytes();
        bytes.front() = std::byte{0xFF};

        REQUIRE_FALSE(deserialize_record(bytes));
    }

    SECTION("declared value length exceeds available input") {
        const std::vector<std::byte> bytes{
            std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF}, std::byte{0xFF},
        };

        const auto result = deserialize_record(bytes);
        REQUIRE_FALSE(result);

        const auto error = result.error();
        REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));

        const auto detail = std::get<tinystore::DecodeError::UnexpectedEnd>(error.detail());
        REQUIRE(detail.available == bytes.size());
        REQUIRE(detail.offset == bytes.size());
        REQUIRE(detail.read_size == tinystore::limits::max_value_length);
    }
}

TEST_CASE("Record encoding enforces the key length limit", "[Record]") {
    tinystore::PutRecord record_valid{
        std::string(static_cast<std::size_t>(tinystore::limits::max_key_length), 'k'),
        "",
    };

    tinystore::ByteWriter bw;
    REQUIRE(record_valid.serialize(bw));

    auto invalid_key =
        std::string(static_cast<std::size_t>(tinystore::limits::max_key_length), 'k');
    invalid_key.push_back('k');
    tinystore::PutRecord record_invalid{invalid_key, ""};

    const auto result = record_invalid.serialize(bw);
    ;
    REQUIRE_FALSE(result);

    const auto error = result.error();
    REQUIRE(std::holds_alternative<tinystore::EncodeError::FieldTooLong>(error.detail()));

    const auto detail = std::get<tinystore::EncodeError::FieldTooLong>(error.detail());
    REQUIRE(detail.field_name == "key");
}
