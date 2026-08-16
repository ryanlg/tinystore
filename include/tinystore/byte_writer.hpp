#pragma once

#include <cstddef>
#include <cstring>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace tinystore {

class ByteWriter {
    std::vector<std::byte> _backing;

  public:
    ByteWriter() {};
    ByteWriter(unsigned int capacity) { _backing.reserve(capacity); };

    template <typename T>
        requires std::is_fundamental_v<T>
    void write(const T &to_write) {
        const auto old_size = _backing.size();
        const auto new_size = old_size + sizeof(T);

        _backing.resize(new_size);
        std::memcpy(_backing.data() + old_size, &to_write, sizeof(T));
    }

    void write(const std::span<const std::byte>);
    void write(const std::string &);
    [[nodiscard]]
    std::vector<std::byte> finalize();
};

} // namespace tinystore
