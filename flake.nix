{
  description = "Q2E-Restore-Recoil – restore machinegun recoil in Quake II Enhanced";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      mingw = pkgs.pkgsCross.mingwW64;
    in {
      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [
          pkgs.cmake
          pkgs.ninja
          mingw.stdenv.cc
        ];

        shellHook = ''
          echo "Q2E-Restore-Recoil cross-compilation shell"
          echo "Build: cmake -B build -G Ninja --toolchain cmake/mingw-w64.cmake && cmake --build build"
        '';
      };
    };
}
