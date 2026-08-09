#pragma once

namespace tinystore {

enum StoreBackendType {
    InMemory,
};

struct StoreOptions {
  public:
    StoreBackendType backend_type;
};

} // namespace tinystore
