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
constexpr const std::array<std::byte, 19> encoded_record{
    std::byte{0x01},                  // PUT
    std::byte{0xEF}, std::byte{0xBE}, std::byte{0xAD}, std::byte{0xDE},
    std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // LSN
    std::byte{0x03}, std::byte{0x00}, // key length
    std::byte{0x01}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, // value length
    std::byte{'k'}, std::byte{0x00}, std::byte{'y'}, // key
    std::byte{'v'},                                  // value
};
// clang-format on

const std::vector<std::byte> record_bytes() {
    return {encoded_record.begin(), encoded_record.end()};
}

} // namespace

TEST_CASE("PutMutation encoding matches the specified byte layout",
          "[PutMutation][WALSerializer]") {
    const tinystore::PutMutation mutation{
        .lsn = 0xDEADBEEF,
        .key = std::string{"k\0y", 3},
        .value = "v",
    };

    std::vector<std::byte> bvec;
    tinystore::WriteAheadLogSerializer wal_serializer(bvec);
    const auto result = wal_serializer.serialize_mutation(mutation);

    REQUIRE(result);
    REQUIRE(bvec == record_bytes());
}

TEST_CASE("PutRecord encoding enforces the key length limit", "[PutRecord][WALSerializer]") {
    const tinystore::PutMutation mutation{
        .lsn = 0,
        .key = std::string(static_cast<std::size_t>(tinystore::limits::max_key_length), 'k'),
        .value = "",
    };

    std::vector<std::byte> bvec;
    tinystore::WriteAheadLogSerializer wal_serializer(bvec);
    REQUIRE(wal_serializer.serialize_mutation(mutation));

    auto invalid_key =
        std::string(static_cast<std::size_t>(tinystore::limits::max_key_length), 'k');
    invalid_key.push_back('k');
    const tinystore::PutMutation invalid_mutation{
        .lsn = 0,
        .key = invalid_key,
        .value = "",
    };

    bvec.resize(0);
    const auto result = wal_serializer.serialize_mutation(invalid_mutation);
    REQUIRE_FALSE(result);

    const auto error = result.error();
    REQUIRE(std::holds_alternative<tinystore::EncodeError::FieldTooLong>(error.detail()));

    const auto detail = std::get<tinystore::EncodeError::FieldTooLong>(error.detail());
    REQUIRE(detail.field_name == "key");
}
