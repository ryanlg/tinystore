#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace tinystore {

class Store {
public:
    void put(std::string key, std::string value);

    [[nodiscard]]
    std::optional<std::string> get(const std::string& key);

private:
    // @temp: memory backed for now
    std::unordered_map<std::string, std::string> _backing;
};

} // namespace tinystore
