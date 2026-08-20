#ifndef TINYSTORE_ERROR_HPP
#define TINYSTORE_ERROR_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <variant>

namespace tinystore {

class DecodeError {
  public:
    struct InvalidValue {
        std::string_view field_name;

        [[nodiscard]]
        constexpr std::string_view describe() const noexcept {
            return "Field decodes to invalid value";
        }
    };

    struct UnexpectedEnd {
        std::size_t available;
        std::size_t offset;
        std::size_t read_size;

        [[nodiscard]]
        constexpr std::string_view describe() const noexcept {
            return "Available input is shorted than expected";
        }
    };

    struct TrailingBytes {
        std::size_t available;
        std::size_t end;

        [[nodiscard]]
        constexpr std::string_view describe() const noexcept {
            return "Unexpected additional data at the end of input";
        }
    };

    using Detail = std::variant<InvalidValue, UnexpectedEnd, TrailingBytes>;

    // Explicitly allow implicit conversion
    template <class T> DecodeError(const T &detail) : _detail(detail) {}

    [[nodiscard]]
    constexpr std::string_view describe() const {
        // @todo: this method can't be marked as noexcept because the variant might
        // be valueless_by_exception and visit() would throw.
        return std::visit([](const auto &detail) -> std::string_view { return detail.describe(); },
                          _detail);
    }

    [[nodiscard]]
    const Detail &detail() const noexcept {
        return _detail;
    }

  private:
    Detail _detail;
};

enum class EncodeErrorCode : uint8_t {
    field_too_long,
    overflow,
};

class EncodeError {
  public:
    struct FieldTooLong {
        std::string_view field_name;
        std::size_t input_length;
        std::size_t max_length;

        [[nodiscard]]
        constexpr std::string_view describe() const noexcept {
            return "Field length exceeds specification";
        }
    };

    struct Overflow {
        [[nodiscard]]
        constexpr std::string_view describe() const noexcept {
            return "Encodee length exceeds machine capacity";
        }
    };

    using Detail = std::variant<FieldTooLong, Overflow>;

    template <class T> EncodeError(const T &detail) : _detail(detail) {}

    [[nodiscard]]
    constexpr std::string_view describe() const {
        return std::visit([](const auto &detail) -> std::string_view { return detail.describe(); },
                          _detail);
    }

    [[nodiscard]]
    const Detail &detail() const noexcept {
        return _detail;
    }

  private:
    Detail _detail;
};

} // namespace tinystore

#endif // TINYSTORE_ERROR_HPP
