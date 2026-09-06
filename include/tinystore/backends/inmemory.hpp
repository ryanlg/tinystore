#ifndef TINYSTORE_BACKENDS_INMEMORY_HPP
#define TINYSTORE_BACKENDS_INMEMORY_HPP

#include <optional>
#include <string>
#include <unordered_map>

#include <tinystore/backends/base.hpp>
#include <tinystore/mutation.hpp>
#include <tinystore/store.hpp>

namespace tinystore {

class InMemoryBackend : public StoreBackend {
  public:
    void put(PutMutation) override;

    [[nodiscard]]
    std::optional<std::string> get(const std::string &key) override;

  private:
    std::unordered_map<std::string, std::string> _backing;
};

} // namespace tinystore

#endif // TINYSTORE_BACKENDS_INMEMORY_HPP
