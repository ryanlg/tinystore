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
constexpr std::array<std::byte, 11> encoded_record{
    std::byte{0x01},                  // PUT
    std::byte{0x03}, std::byte{0x00}, // key length
    std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // value length
    std::byte{'k'}, std::byte{0x00}, std::byte{'y'}, // key
    std::byte{'v'},                                  // value
};

std::vector<std::byte> record_bytes() { return {encoded_record.begin(), encoded_record.end()}; }

constexpr std::array<std::byte, 35> encoded_batch{
    // LSN
    std::byte{0xFF}, std::byte{0x88}, std::byte{0x34}, std::byte{0x00},
    std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF},

    // number_of_records
    std::byte{0x02}, std::byte{0x00},

    // Record 1
    std::byte{0x01},                  // PUT
    std::byte{0x03}, std::byte{0x00}, // key length
    std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // value length
    std::byte{'k'}, std::byte{0x00}, std::byte{'y'}, // key
    std::byte{'v'},                                  // value

    // Record 2
    std::byte{0x01},                  // PUT
    std::byte{0x02}, std::byte{0x00}, // key length
    std::byte{0x05}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},  // value length
    std::byte{'1'}, std::byte{'2'},                                                    // key
    std::byte{'v'}, std::byte{'a'}, std::byte{'l'}, std::byte{'u'}, std::byte{'e'},    // value
};

std::vector<std::byte> batch_bytes() { return {encoded_batch.begin(), encoded_batch.end()}; }
// clang-format on

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

        const auto error = result.error();
        REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));
    }
}

TEST_CASE("Record decoding rejects malformed input", "[Record]") {
    SECTION("trailing byte") {
        auto bytes = record_bytes();
        bytes.push_back(std::byte{0xFF});

        const auto result = tinystore::decode_record(bytes);
        REQUIRE_FALSE(result);

        const auto error = result.error();
        REQUIRE(std::holds_alternative<tinystore::DecodeError::TrailingBytes>(error.detail()));

        const auto detail = std::get<tinystore::DecodeError::TrailingBytes>(error.detail());
        REQUIRE(detail.available == bytes.size());
        REQUIRE(detail.end == encoded_record.size());
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

        const auto error = result.error();
        REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));

        const auto detail = std::get<tinystore::DecodeError::UnexpectedEnd>(error.detail());
        REQUIRE(detail.available == bytes.size());
        REQUIRE(detail.offset == bytes.size());
        REQUIRE(detail.read_size == tinystore::limits::max_value_length);
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

    const auto error = result.error();
    REQUIRE(std::holds_alternative<tinystore::EncodeError::FieldTooLong>(error.detail()));

    const auto detail = std::get<tinystore::EncodeError::FieldTooLong>(error.detail());
    REQUIRE(detail.field_name == "key");
}

TEST_CASE("LogBatch encoding matches the specified byte layout", "[LogBatch]") {
    tinystore::Record record1{
        .op = tinystore::Operation::PUT,
        .key = std::string{"k\0y", 3},
        .value = "v",
    };
    tinystore::Record record2{
        .op = tinystore::Operation::PUT,
        .key = "12",
        .value = "value",
    };
    const uint64_t lsn = 0xEFBEADDE003488FF;

    tinystore::LogBatch lb(lsn);
    lb.append(std::move(record1));
    lb.append(std::move(record2));

    const auto result = tinystore::encode_batch(lb);

    REQUIRE(result);
    REQUIRE(*result == batch_bytes());
}
