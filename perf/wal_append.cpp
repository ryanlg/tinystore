#include <tinystore/backends/inmemory.hpp>
#include <tinystore/store.hpp>

#include <cstdint>

constexpr static const uint32_t ITERATION = 1'000'000;

int main() {
    tinystore::Store store(tinystore::StoreOptions{tinystore::StoreBackendType::InMemory});

    for (uint32_t i = 0; i < ITERATION; i++)
        store.put("language", "C++RustJavaGoJavaScript");
}
