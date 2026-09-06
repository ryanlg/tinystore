#include <tinystore/byte_codec.hpp>

#include <bit>
#include <concepts>
#include <cstring>

namespace tinystore {

// Encode `to_write` as **little endian**; byte swap as needed depending on
// native endianess.
template <std::integral T> void write_integral_le(std::vector<std::byte> &bvec, const T to_write) {
    const auto old_size = bvec.size();
    const auto new_size = old_size + sizeof(T);

    bvec.resize(new_size);

    auto value = to_write;
    if constexpr (std::endian::native == std::endian::big)
        value = std::byteswap(value);

    std::memcpy(bvec.data() + old_size, &value, sizeof(T));
}

void ByteWriter::write_span(const std::span<const std::byte> span) {
    _backing.insert(_backing.end(), span.begin(), span.end());
}

void ByteWriter::write_string(const std::string &str) {
    write_span(std::as_bytes(std::span<const char>{str.cbegin(), str.cend()}));
}

void ByteWriter::write_u8(uint8_t given) { write_integral_le<uint8_t>(_backing, given); }

void ByteWriter::write_u16_le(uint16_t given) { write_integral_le<uint16_t>(_backing, given); }

void ByteWriter::write_u32_le(uint32_t given) { write_integral_le<uint32_t>(_backing, given); }

void ByteWriter::write_u64_le(uint64_t given) { write_integral_le<uint64_t>(_backing, given); }

[[nodiscard]]
std::expected<void, DecodeError> check_offset_bounds(const std::size_t available,
                                                     const std::size_t offset,
                                                     const std::size_t read_size) {
    if (offset > available || available - offset < read_size)
        return std::unexpected(DecodeError::UnexpectedEnd(available, offset, read_size));

    return {};
}

// Decode an integral value in the backing store at `offset`, **assuming**
// the value is **little endian** in the backing store and byte swap as
// needed depending on native endianess.
template <std::integral T>
[[nodiscard]] std::expected<T, DecodeError> read_le(std::span<const std::byte> bspan,
                                                    const std::size_t offset) {
    auto check = check_offset_bounds(bspan.size(), offset, sizeof(T));
    if (!check)
        return std::unexpected(check.error());

    T result;
    std::memcpy(&result, bspan.data() + offset, sizeof(T));

    if constexpr (std::endian::native == std::endian::big)
        result = std::byteswap(result);

    return result;
}

#define READ_LE(type, range, pos)                                                                  \
    do {                                                                                           \
        auto result = read_le<type>((range), (pos));                                               \
        if (result)                                                                                \
            (pos) += sizeof(type);                                                                 \
        return result;                                                                             \
    } while (0)

[[nodiscard]] std::expected<uint8_t, DecodeError> ByteReader::read_u8() {
    READ_LE(uint8_t, _range, _pos);
}

[[nodiscard]] std::expected<uint16_t, DecodeError> ByteReader::read_u16_le() {
    READ_LE(uint16_t, _range, _pos);
}

[[nodiscard]] std::expected<uint32_t, DecodeError> ByteReader::read_u32_le() {
    READ_LE(uint32_t, _range, _pos);
}

[[nodiscard]] std::expected<uint64_t, DecodeError> ByteReader::read_u64_le() {
    READ_LE(uint64_t, _range, _pos);
}

#undef READ_LE

[[nodiscard]]
std::expected<std::string, DecodeError> ByteReader::read_string(const std::size_t size) {
    auto check = check_offset_bounds(_range.size(), _pos, size);
    if (!check)
        return std::unexpected(check.error());

    auto result = std::string(reinterpret_cast<const char *>(_range.data() + _pos), size);
    _pos += size;
    return result;
}

[[nodiscard]] std::expected<void, DecodeError> ByteReader::end_consumed() {
    if (!has_ended())
        return std::unexpected(DecodeError::TrailingBytes{.available = _range.size(), .end = _pos});

    return {};
}

} // namespace tinystore
