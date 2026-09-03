#ifndef TINYSTORE_BACKENDS_INMEMORY_HPP
#define TINYSTORE_BACKENDS_INMEMORY_HPP

#include <optional>
#include <string>
#include <unordered_map>

#include <tinystore/record.hpp>
#include <tinystore/store.hpp>

#include "../backends/base.hpp"

namespace tinystore {

class InMemoryBackend : public StoreBackend {
  public:
    void put(std::string key, std::string value) override;

    [[nodiscard]]
    std::optional<std::string> get(const std::string &key) override;

  private:
    std::unordered_map<std::string, std::string> _backing;
    WriteAheadLogWriter _wal_writer;
};

} // namespace tinystore

#endif // TINYSTORE_BACKENDS_INMEMORY_HPP
