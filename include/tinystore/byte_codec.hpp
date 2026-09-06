#ifndef TINYSTORE_BYTE_CODEC_HPP
#define TINYSTORE_BYTE_CODEC_HPP

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

#include <tinystore/error.hpp>

namespace tinystore {

class ByteWriter {
  public:
    ByteWriter(std::vector<std::byte> &bvec) : _backing(bvec) {}

    void write_u8(const uint8_t);
    void write_u16_le(const uint16_t);
    void write_u32_le(const uint32_t);
    void write_u64_le(const uint64_t);
    void write_string(const std::string &);
    void write_span(const std::span<const std::byte>);

  private:
    std::vector<std::byte> &_backing;
};

class ByteReader {
  public:
    explicit ByteReader(std::span<const std::byte> given, std::size_t starting_offset = 0)
        : _range(given), _pos(starting_offset) {}

    [[nodiscard]] std::expected<uint8_t, DecodeError> read_u8();
    [[nodiscard]] std::expected<uint16_t, DecodeError> read_u16_le();
    [[nodiscard]] std::expected<uint32_t, DecodeError> read_u32_le();
    [[nodiscard]] std::expected<uint64_t, DecodeError> read_u64_le();

    [[nodiscard]]
    std::expected<std::string, DecodeError> read_string(const std::size_t size);

    [[nodiscard]] bool has_ended() { return _pos == _range.size(); };
    [[nodiscard]] std::expected<void, DecodeError> end_consumed();

  private:
    std::span<const std::byte> _range;
    std::size_t _pos;
};

} // namespace tinystore

#endif // TINYSTORE_BYTE_CODEC_HPP
