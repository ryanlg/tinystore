#include <tinystore/backends/base.hpp>
#include <tinystore/store.hpp>

namespace tinystore {

Store::Store(std::unique_ptr<StoreBackend> backend) : _backend(std::move(backend)) {}

void Store::put(std::string key, std::string value) {
    _backend->put(std::move(key), std::move(value));
}

std::optional<std::string> Store::get(const std::string &key) { return _backend->get(key); }

} // namespace tinystore
