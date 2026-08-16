#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace tinystore {

enum Operation {
    PUT = 1,
    DELETE = 2,
};

struct Record {
    Operation op;
    std::string key;
    std::string value;
};

std::vector<std::byte> encode_record(const Record &);
Record decode_record(const std::vector<std::byte> &);

} // namespace tinystore
