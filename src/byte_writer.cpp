#include <tinystore/byte_writer.hpp>

namespace tinystore {

void ByteWriter::write(const std::span<const std::byte> span) {
    _backing.insert(_backing.end(), span.begin(), span.end());
    offset += span.size();
}

void ByteWriter::write(const std::string &str) {
    write(std::as_bytes(std::span<const char>{str.cbegin(), str.cend()}));
}

std::vector<std::byte> ByteWriter::finalize() { return std::move(_backing); }

} // namespace tinystore
