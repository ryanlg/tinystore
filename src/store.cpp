#include <tinystore/store.hpp>

#include <stdexcept>
#include <string>
#include <utility>

#include <tinystore/backends/inmemory.hpp>
#include <tinystore/options.hpp>

namespace tinystore {

std::unique_ptr<StoreBackend> Store::make_backend(const StoreOptions &options) {
    switch (options.backend_type) {
    case StoreBackendType::InMemory:
        return std::make_unique<InMemoryBackend>();
    case StoreBackendType::Unspecified:
        // Fall through
    }

    throw std::invalid_argument("invalid or unsupported backend type");
}

Store::Store(const StoreOptions &options) : _backend(make_backend(options)) {}

Store::Store(std::unique_ptr<StoreBackend> backend) {
    if (!backend)
        throw std::invalid_argument("*backend cannot be nullptr");
    _backend = std::move(backend);
}

void Store::put(std::string key, std::string value) {
    _backend->put(std::move(key), std::move(value));
}

std::optional<std::string> Store::get(const std::string &key) { return _backend->get(key); }

} // namespace tinystore
