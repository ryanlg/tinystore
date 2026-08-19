#ifndef TINYSTORE_ERROR_HPP
#define TINYSTORE_ERROR_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace tinystore {

enum class DecodeErrorCode : uint8_t {
    invalid_value,
    unexpected_end,
    trailing_bytes,
};

struct DecodeError {
    DecodeErrorCode code;
    std::size_t available;
    std::size_t offset;
    std::size_t read_size;

    constexpr std::string_view reason() noexcept {
        switch (code) {
        case DecodeErrorCode::unexpected_end:
            return "Available range is shorted than expected";
        case DecodeErrorCode::trailing_bytes:
            return "Unexpected additional data";
        case DecodeErrorCode::invalid_value:
            return "Invalid value";
        }

        return "unknown error";
    }

    static constexpr DecodeError unexpected_end(std::size_t available, std::size_t offset,
                                                std::size_t read_size) {
        return DecodeError{
            .code = DecodeErrorCode::unexpected_end,
            .available = available,
            .offset = offset,
            .read_size = read_size,
        };
    }

    static constexpr DecodeError trailing_bytes(std::size_t available, std::size_t offset) {
        return DecodeError{
            .code = DecodeErrorCode::trailing_bytes,
            .available = available,
            .offset = offset,
            .read_size = 0,
        };
    }

    static constexpr DecodeError invalid_value() {
        return DecodeError{
            .code = DecodeErrorCode::invalid_value,
            .available = 0,
            .offset = 0,
            .read_size = 0,
        };
    }
};

enum class EncodeErrorCode : uint8_t {
    field_too_long,
    overflow,
};

struct EncodeError {
    EncodeErrorCode code;
    std::string field_name;

    constexpr std::string_view reason() noexcept {
        switch (code) {
        case EncodeErrorCode::field_too_long:
            return "Field length exceeds specification";
        case EncodeErrorCode::overflow:
            return "Encodee length exceeds machine capacity";
        }

        return "unknown error";
    }

    static constexpr EncodeError field_too_long(std::string &&field_name) {
        return EncodeError{
            .code = EncodeErrorCode::field_too_long,
            .field_name = std::move(field_name),
        };
    }

    static constexpr EncodeError overflow() {
        return EncodeError{
            .code = EncodeErrorCode::overflow,
            .field_name = "",
        };
    }
};

} // namespace tinystore

#endif // TINYSTORE_ERROR_HPP
