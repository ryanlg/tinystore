/// Store backend abstraction.
///
/// This is the base interface for store backends, which executes commands
/// when received from `Store`.

#pragma once

#include <string>
#include <optional>

namespace tinystore {

class StoreBackend {
  public:
    virtual ~StoreBackend() = default;

    virtual void put(std::string key, std::string value) = 0;

    [[nodiscard]]
    virtual std::optional<std::string> get(const std::string &key) = 0;
};

} // namespace tinystore
