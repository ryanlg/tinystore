#include <tinystore/store.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

#include <tinystore/backends/inmemory.hpp>
#include <tinystore/mutation.hpp>
#include <tinystore/options.hpp>
#include <tinystore/record.hpp>

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

Store::Store(const StoreOptions &options)
    : _backend(make_backend(options)), _next_lsn(0), _wal(std::filesystem::path("./wal")) {}

Store::Store(std::unique_ptr<StoreBackend> backend) : _wal(std::filesystem::path("./wal")) {
    if (!backend)
        throw std::invalid_argument("*backend cannot be nullptr");
    _backend = std::move(backend);
    _next_lsn = 0;
}

void Store::put(std::string key, std::string value) {
    const auto this_lsn = _next_lsn++;

    // Mutation will own all data until acknowledgement
    PutMutation pm = {
        .lsn = this_lsn,
        .key = std::move(key),
        .value = std::move(value),
    };

    // @todo: pass by mutation
    const auto wal_result = _wal.log_put(pm);
    // @todo: what do we actually do with serialization fails?
    (void)wal_result;

    _backend->put(std::move(pm));
}

std::optional<std::string> Store::get(const std::string &key) { return _backend->get(key); }

} // namespace tinystore
