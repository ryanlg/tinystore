#include <tinystore/backends/inmemory.hpp>
#include <tinystore/record.hpp>

#include <unordered_map>

namespace tinystore {

void InMemoryBackend::put(std::string key, std::string value) {
    const auto wal_result = _wal.log_put(PutRecord(key, value));

    // @todo: what do we actually do with serialization fails?
    (void)wal_result;

    _backing.insert_or_assign(std::move(key), std::move(value));
}

std::optional<std::string> InMemoryBackend::get(const std::string &key) {
    const auto found = _backing.find(key);
    if (found == _backing.end())
        return std::nullopt;

    return found->second;
}

} // namespace tinystore
