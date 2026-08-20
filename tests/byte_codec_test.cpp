#include <tinystore/byte_codec.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace {

constexpr std::array<std::byte, 12> encoded_values{
    std::byte{0xAB},                  // u8
    std::byte{0x34}, std::byte{0x12}, // u16, little-endian
    std::byte{0xEF}, std::byte{0xCD}, // u32, little-endian
    std::byte{0xAB}, std::byte{0x89}, std::byte{'A'},
    std::byte{0x00}, std::byte{'B'},  // string with embedded NUL
    std::byte{0x00}, std::byte{0xFF}, // raw span
};

} // namespace

TEST_CASE("ByteWriter emits the specified byte representation", "[ByteWriter]") {
    tinystore::ByteWriter writer{encoded_values.size()};
    const std::string string_with_nul{"A\0B", 3};
    const std::array trailing_bytes{std::byte{0x00}, std::byte{0xFF}};

    writer.write_u8(std::uint8_t{0xAB});
    writer.write_u16_le(std::uint16_t{0x1234});
    writer.write_u32_le(std::uint32_t{0x89ABCDEF});
    writer.write_string(string_with_nul);
    writer.write_span(trailing_bytes);

    const std::vector<std::byte> expected{encoded_values.begin(), encoded_values.end()};
    REQUIRE(std::move(writer).finalize() == expected);
}

TEST_CASE("ByteWriter can finalize an empty buffer", "[ByteWriter]") {
    auto bytes = tinystore::ByteWriter{}.finalize();

    REQUIRE(bytes.empty());
}

TEST_CASE("ByteReader decodes sequential values and detects the end", "[ByteReader]") {
    tinystore::ByteReader reader{encoded_values};

    REQUIRE(reader.read_u8() == std::uint8_t{0xAB});
    REQUIRE(reader.read_u16_le() == std::uint16_t{0x1234});
    REQUIRE(reader.read_u32_le() == std::uint32_t{0x89ABCDEF});
    REQUIRE(reader.read_string(3) == std::string{"A\0B", 3});
    REQUIRE(reader.read_u8() == std::uint8_t{0x00});
    REQUIRE(reader.read_u8() == std::uint8_t{0xFF});

    const auto past_end = reader.read_u8();
    REQUIRE_FALSE(past_end);

    const auto error = past_end.error();
    REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));

    const auto detail = std::get<tinystore::DecodeError::UnexpectedEnd>(error.detail());
    REQUIRE(detail.offset == encoded_values.size());
}

TEST_CASE("ByteReader does not consume input when a read fails", "[ByteReader]") {
    constexpr std::array bytes{std::byte{0x2A}};
    tinystore::ByteReader reader{bytes};

    const auto too_wide = reader.read_u16_le();
    REQUIRE_FALSE(too_wide);

    const auto error = too_wide.error();
    REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));

    const auto detail = std::get<tinystore::DecodeError::UnexpectedEnd>(error.detail());
    REQUIRE(detail.offset == 0);

    REQUIRE(reader.read_u8() == std::uint8_t{0x2A});
}

TEST_CASE("ByteReader checks string bounds and accepts an empty string", "[ByteReader]") {
    constexpr std::array bytes{std::byte{'o'}, std::byte{'k'}};
    tinystore::ByteReader reader{bytes};

    const auto too_long = reader.read_string(3);
    REQUIRE_FALSE(too_long);

    const auto error = too_long.error();
    REQUIRE(std::holds_alternative<tinystore::DecodeError::UnexpectedEnd>(error.detail()));

    const auto detail = std::get<tinystore::DecodeError::UnexpectedEnd>(error.detail());
    REQUIRE(detail.offset == 0);

    REQUIRE(reader.read_string(2) == "ok");
    REQUIRE(reader.read_string(0) == "");
}

TEST_CASE("ByteReader checks for trailing bytes", "[ByteReader]") {
    constexpr std::array bytes{std::byte{'1'}, std::byte{'2'}};
    tinystore::ByteReader reader{bytes};

    const auto s1 = reader.read_string(1);
    REQUIRE(s1);

    const auto trailing = reader.end_consumed();
    REQUIRE_FALSE(trailing);

    const auto error = trailing.error();
    REQUIRE(std::holds_alternative<tinystore::DecodeError::TrailingBytes>(error.detail()));

    const auto detail = std::get<tinystore::DecodeError::TrailingBytes>(error.detail());
    REQUIRE(detail.available == 2);
    REQUIRE(detail.end == 1);
}
