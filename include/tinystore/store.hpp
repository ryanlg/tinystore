/// Store abstraction.
///
/// This class is the interface for interacting with Tinystore. It holds
/// a `StoreBackend`, which is responsible for actually executing the read/write
/// commands.

#pragma once

#include <memory>
#include <optional>
#include <string>

namespace tinystore {

class StoreBackend;

class Store {
    std::unique_ptr<StoreBackend> _backend;

  public:
    Store(std::unique_ptr<StoreBackend> backend);

    void put(std::string key, std::string value);

    [[nodiscard]]
    std::optional<std::string> get(const std::string &key);
};

} // namespace tinystore
