#ifndef TINYSTORE_OPTIONS_HPP
#define TINYSTORE_OPTIONS_HPP

#include <cstdint>

namespace tinystore {

enum class StoreBackendType : uint8_t {
    Unspecified = 0,
    InMemory,
};

struct StoreOptions {
  public:
    StoreBackendType backend_type{};

    explicit StoreOptions(StoreBackendType sbt) : backend_type(sbt) {}
};

} // namespace tinystore

#endif // TINYSTORE_OPTIONS_HPP
