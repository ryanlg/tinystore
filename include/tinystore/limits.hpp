#ifndef TINYSTORE_LIMITS_HPP
#define TINYSTORE_LIMITS_HPP

#include <cstdint>

namespace tinystore::limits {

// Records
constexpr uint16_t max_key_length = UINT16_MAX;
constexpr uint32_t max_value_length = UINT32_MAX;

} // namespace tinystore::limits

#endif // TINYSTORE_LIMITS_HPP
