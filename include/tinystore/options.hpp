#pragma once

namespace tinystore {

enum StoreBackendType {
    Unspecified = 0,
    InMemory,
};

struct StoreOptions {
  public:
    StoreBackendType backend_type{};

    explicit StoreOptions(StoreBackendType sbt) : backend_type(sbt) {}
};

} // namespace tinystore
