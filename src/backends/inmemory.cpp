#include <tinystore/backends/inmemory.hpp>

#include <unordered_map>

namespace tinystore {

void InMemoryBackend::put(PutMutation mutation) {
    _backing.insert_or_assign(std::move(mutation.key), std::move(mutation.value));
}

std::optional<std::string> InMemoryBackend::get(const std::string &key) {
    const auto found = _backing.find(key);
    if (found == _backing.end())
        return std::nullopt;

    return found->second;
}

} // namespace tinystore
