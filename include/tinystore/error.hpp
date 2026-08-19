#ifndef TINYSTORE_ERROR_HPP
#define TINYSTORE_ERROR_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace tinystore {

enum class DecodeErrorCode : uint8_t {
    offset_out_of_bounds,
};

struct DecodeError {
    DecodeErrorCode code;
    std::size_t offset;

    constexpr std::string_view reason() noexcept {
        switch (code) {
        case DecodeErrorCode::offset_out_of_bounds:
            return "Given offset is out of bounds";
        }

        return "unknown error";
    }

    static constexpr DecodeError offset_out_of_bounds(std::size_t offset) {
        return DecodeError{
            .code = DecodeErrorCode::offset_out_of_bounds,
            .offset = offset,
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
