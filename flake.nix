{
  description = "Mimiris";

  nixConfig = {
    extra-trusted-public-keys = "devenv.cachix.org-1:w1cLUi8dv3hnoSPGAuibQv+f9TZLr6cv/Hm9XgU50cw=";
    extra-substituters = "https://devenv.cachix.org";
  };

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    devenv.url = "github:cachix/devenv";
    git-hooks.url = "github:cachix/git-hooks.nix";
  };

  outputs =
    inputs@{
      flake-parts,
      nixpkgs,
      devenv,
      ...
    }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = nixpkgs.lib.systems.flakeExposed;

      perSystem =
        { pkgs, ... }:
        let
          llvm = pkgs.llvmPackages_22;

          buildTools = with pkgs; [
            cmake
            ninja
            pkg-config
          ];
        in
        {
          devShells.default = devenv.lib.mkShell {
            inherit inputs pkgs;

            modules = [
              {
                stdenv = llvm.libcxxStdenv;

                packages =
                  buildTools
                  ++ [
                    llvm.lld
                    llvm.lldb
                    llvm.clang-tools

                    pkgs.gtest
                    pkgs.cppcheck
                    pkgs.codespell
                    pkgs.doxygen
                  ]
                  ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
                    pkgs.valgrind-light
                  ];

                git-hooks.hooks = {
                  nixfmt.enable = true;
                  clang-format.enable = true;
                  cmake-format = {
                    enable = true;
                    entry = "cmake-format -i";
                    files = "(CMakeLists\\.txt|\\.cmake)$";
                  };
                };
              }
            ];
          };
        };
    };
}
