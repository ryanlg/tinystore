# Format all files (supported: Nix)
fmt:
    nix fmt flake.nix
    cmake --build build --target format

lint:
    cmake --build build --target tidy

build:
    cmake --build build

test: build
    ctest --test-dir build --output-on-failure
