# Format all files (supported: Nix)
fmt:
    nix fmt flake.nix
    cmake --build build --target format
