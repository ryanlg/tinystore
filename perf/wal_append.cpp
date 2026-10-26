#include <tinystore/backends/inmemory.hpp>
#include <tinystore/store.hpp>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <print>
#include <string_view>
#include <vector>

constexpr static const uint32_t DEFAULT_ITERATION = 1'000;

using Clock = std::chrono::steady_clock;

/// Nearest-rank percentile.
/// input is assumed to be sorted. tile is the percentage: 90, 99, etc.
static Clock::duration percentile(const std::vector<Clock::duration> &input, const double tile) {

    const auto rank =
        static_cast<size_t>(std::ceil(tile / 100.0 * static_cast<double>(input.size())));
    return input[std::clamp(rank, size_t{1}, input.size()) - 1];
}

static std::chrono::microseconds to_us(Clock::duration input) {
    return std::chrono::duration_cast<std::chrono::microseconds>(input);
}

int main(int argc, char *argv[]) {
    uint32_t iterations = DEFAULT_ITERATION;

    if (argc > 1) {
        // Only one option: number of iteration
        std::string_view const arg = argv[1];
        auto [ptr, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), iterations);

        if (ec != std::errc{} || ptr != arg.data() + arg.size() || iterations == 0) {
            std::println(stderr, "usage: {} [iterations] (default: {})", argv[0],
                         DEFAULT_ITERATION);
            return 1;
        }
    }

    std::vector<Clock::duration> latencies(iterations);
    tinystore::Store store(tinystore::StoreOptions{tinystore::StoreBackendType::InMemory});

    auto total_start = Clock::now();
    for (uint32_t i = 0; i < iterations; i++) {
        auto cur_start = Clock::now();

        store.put("language", "C++RustJavaGoJavaScript");

        auto cur_end = Clock::now();
        latencies[i] = cur_end - cur_start;
    }
    auto total_end = Clock::now();
    const auto total_duration = std::chrono::duration<double>(total_end - total_start).count();

    std::sort(latencies.begin(), latencies.end());

    std::println("iterations : {}", iterations);
    std::println("total      : {:.3f} s", total_duration);
    std::println("throughput : {:.2f} ops/s", static_cast<double>(iterations) / total_duration);
    std::println("put latency (us)");
    std::println("  min      : {}", to_us(latencies.front()));
    std::println("  p50      : {}", to_us(percentile(latencies, 50)));
    std::println("  p95      : {}", to_us(percentile(latencies, 95)));
    std::println("  p99      : {}", to_us(percentile(latencies, 99)));
    std::println("  max      : {}", to_us(latencies.back()));
}
