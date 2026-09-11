{
  description = "Tinystore";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "aarch64-darwin"
        "aarch64-linux"
        "x86_64-darwin"
        "x86_64-linux"
      ];

      perSystem = { pkgs, ... }: {
        formatter = pkgs.nixfmt;

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            catch2_3
            # clang package by default includes the extra tools (e.g. clang-tidy),
            # but they are not wrapped like `llvmPackages.clang-tools`, which means
            # they don't have access to the library path that the Nix wrapper injects.
            # Disable them here so that we can rely on the other package. This however
            # make clang a build cache miss.
            (clang.override {
              cc = llvmPackages.clang-unwrapped.override {
                enableClangToolsExtra = false;
              };
            })
            cmake
            hyperfine
            just
            lldb
            llvmPackages.clang-tools
            ninja
            pkg-config
            python313 # Needed by run-clang-tidy
          ];

          # Required for LLDB on macOS (stinky)
          shellHook = pkgs.lib.optionalString pkgs.stdenv.isDarwin ''
            # Tested both paths on my machine and they both work (adds some flexibility)
            if [[ -z "$LLDB_DEBUGSERVER_PATH" ]]; then
              XCODE_PATH="/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Versions/A/Resources/debugserver"
              CLT_PATH="/Library/Developer/CommandLineTools/Library/PrivateFrameworks/LLDB.framework/Versions/A/Resources/debugserver"

              if [[ -f "$XCODE_PATH" ]]; then
                export LLDB_DEBUGSERVER_PATH="$XCODE_PATH"
              elif [[ -f "$CLT_PATH" ]]; then
                export LLDB_DEBUGSERVER_PATH="$CLT_PATH"
              fi
            fi
          '';
        };
      };
    };
}
