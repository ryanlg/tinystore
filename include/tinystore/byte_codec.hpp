#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <expected>
#include <span>
#include <string>
#include <vector>

#include <tinystore/error.hpp>

namespace tinystore {

class ByteCodec {
    std::vector<std::byte> _backing;

  public:
    ByteCodec() {};
    ByteCodec(std::size_t);
    // @todo: this is terrible. only realized the problem after
    // I decided to put reading in this class too. need to split this out again.
    ByteCodec(const std::vector<std::byte> &&);

    ~ByteCodec() = default;

    // No copying the codec
    ByteCodec(const ByteCodec &) = delete;
    ByteCodec &operator=(const ByteCodec &) = delete;

    // But you can move it
    ByteCodec(ByteCodec &&) noexcept = default;
    ByteCodec &operator=(ByteCodec &&) noexcept = default;

    // Encode `to_write` as **little endian**; byte swap as needed depending on
    // native endianess.
    template <std::integral T> void write_le(const T &to_write) {
        const auto old_size = _backing.size();
        const auto new_size = old_size + sizeof(T);

        _backing.resize(new_size);

        auto value = to_write;
        if constexpr (std::endian::native == std::endian::big)
            value = std::byteswap(value);

        std::memcpy(_backing.data() + old_size, &value, sizeof(T));
    }

    void write(const std::span<const std::byte>);
    void write(const std::string &);
    [[nodiscard]] std::vector<std::byte> finalize() && noexcept;

    // Decode an integral value in the backing store at `offset`, **assuming**
    // the value is **little endian** in the backing store and byte swap as
    // needed depending on native endianess.
    template <std::integral T>
    [[nodiscard]] std::expected<T, DecodeError> read_le(const std::size_t offset) {
        if (offset > _backing.size() || _backing.size() - offset < sizeof(T))
            return std::unexpected(DecodeError::offset_out_of_bounds(offset));

        T result;
        std::memcpy(&result, _backing.data() + offset, sizeof(T));

        if constexpr (std::endian::native == std::endian::big)
            result = std::byteswap(result);

        return result;
    }

    [[nodiscard]]
    std::expected<std::string, DecodeError> read_string(const std::size_t offset,
                                                        const std::size_t size);
};

} // namespace tinystore
