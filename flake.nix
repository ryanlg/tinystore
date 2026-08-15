{
  description = "Tinystore";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      systems = [
        "aarch64-darwin"
        "aarch64-linux"
        "x86_64-darwin"
        "x86_64-linux"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      devShells = forAllSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              catch2_3
              clang
              cmake
              lldb
              llvmPackages.clang-tools
              ninja
              pkg-config
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
        }
      );
    };
}

