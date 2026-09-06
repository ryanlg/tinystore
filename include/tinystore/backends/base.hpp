/// Store backend abstraction.
///
/// This is the base interface for store backends, which executes commands
/// when received from `Store`.

#ifndef TINYSTORE_BACKENDS_BASE_HPP
#define TINYSTORE_BACKENDS_BASE_HPP

#include <optional>
#include <string>

#include <tinystore/mutation.hpp>

namespace tinystore {

class StoreBackend {
  public:
    virtual ~StoreBackend() = default;

    virtual void put(PutMutation) = 0;

    [[nodiscard]]
    virtual std::optional<std::string> get(const std::string &key) = 0;
};

} // namespace tinystore

#endif // TINYSTORE_BACKENDS_BASE_HPP
