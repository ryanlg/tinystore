#ifndef TINYSTORE_RECORD_HPP
#define TINYSTORE_RECORD_HPP

#include <tinystore/error.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace tinystore {

enum class Operation : uint8_t {
    PUT = 1,
    DELETE = 2,
};

struct Record {
    Operation op;
    std::string key;
    std::string value;
};

[[nodiscard]]
std::expected<std::vector<std::byte>, EncodeError> encode_record(const Record &);

[[nodiscard]]
std::expected<Record, DecodeError> decode_record(const std::vector<std::byte> &);

} // namespace tinystore

#endif // TINYSTORE_RECORD_HPP
