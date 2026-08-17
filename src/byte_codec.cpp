#include <tinystore/byte_codec.hpp>

namespace tinystore {

ByteCodec::ByteCodec(std::size_t capacity) { _backing.reserve(capacity); };
ByteCodec::ByteCodec(const std::vector<std::byte> &&given) : _backing(std::move(given)) {};

void ByteCodec::write(const std::span<const std::byte> span) {
    _backing.insert(_backing.end(), span.begin(), span.end());
}

void ByteCodec::write(const std::string &str) {
    write(std::as_bytes(std::span<const char>{str.cbegin(), str.cend()}));
}

std::vector<std::byte> ByteCodec::finalize() && noexcept { return std::move(_backing); }

[[nodiscard]]
std::expected<std::string, DecodeError> ByteCodec::read_string(const std::size_t offset,
                                                               const std::size_t size) {
    if (offset > _backing.size() || _backing.size() - offset < size)
        return std::unexpected(DecodeError::offset_out_of_bounds(offset));

    return std::string(reinterpret_cast<const char *>(_backing.data() + offset), size);
}

} // namespace tinystore
