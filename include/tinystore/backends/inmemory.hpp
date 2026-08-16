#pragma once

#include <unordered_map>

#include <tinystore/store.hpp>

#include "../backends/base.hpp"

namespace tinystore {

class InMemoryBackend : public StoreBackend {
    std::unordered_map<std::string, std::string> _backing;

  public:
    void put(std::string key, std::string value) override;

    [[nodiscard]]
    std::optional<std::string> get(const std::string &key) override;
};

} // namespace tinystore
