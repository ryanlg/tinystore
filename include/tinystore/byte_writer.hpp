#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace tinystore {

class ByteWriter {
    std::vector<std::byte> _backing;
    off_t offset;

  public:
    ByteWriter() : offset(0) {};
    ByteWriter(unsigned int capacity) : _backing(std::vector<std::byte>(capacity)), offset(0) {};

    template <typename T>
        requires std::is_fundamental_v<T>
    void write(const T &to_write) {
        const auto new_size = offset + sizeof(T);

        _backing.resize(new_size);
        std::memcpy(_backing.data() + offset, &to_write, sizeof(T));
        offset = new_size;
    }

    void write(const std::span<const std::byte>);
    void write(const std::string &);
    [[nodiscard]]
    std::vector<std::byte> finalize();
};

} // namespace tinystore
