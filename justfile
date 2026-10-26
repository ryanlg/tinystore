# User-defined functions (`build_dir`) are unstable at the moment.
set unstable

default: build test

build_dir(build_type) := if build_type == "Debug" { "build" } else { "build-" + lowercase(build_type) }

# Configure the build directory (only if it isn't configured yet).
[arg("build_type", long="release", value="Release")]
configure build_type="Debug":
    [ -f {{ build_dir(build_type) }}/CMakeCache.txt ] || cmake -B {{ build_dir(build_type) }} -G Ninja -DCMAKE_BUILD_TYPE={{ build_type }}

# Format all files (supported: Nix, C++)
fmt: configure
    nix fmt flake.nix
    cmake --build build --target format

# Lint C++ files
lint: configure
    cmake --build build --target tidy

# Build. Default to Debug. Use `--release` to build release.
[arg("build_type", long="release", value="Release")]
build build_type="Debug": (configure build_type)
    cmake --build {{ build_dir(build_type) }}

# Test. Default to Debug. Use `--release` to build release.
[arg("build_type", long="release", value="Release")]
test build_type="Debug": (build build_type)
    ctest --test-dir {{ build_dir(build_type) }} --output-on-failure

# Run the perf/wal_append, always Release build.
wal-perf *args: (build "Release")
    ./{{ build_dir("Release") }}/tsperf_wal_append {{ args }}
