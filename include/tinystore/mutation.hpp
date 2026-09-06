#ifndef TINYSTORE_MUTATION_HPP
#define TINYSTORE_MUTATION_HPP

#include <string>

#include <tinystore/types.hpp>

namespace tinystore {

struct PutMutation {
    lsn_t lsn;
    std::string key;
    std::string value;
};

} // namespace tinystore

#endif // TINYSTORE_MUTATION_HPP
