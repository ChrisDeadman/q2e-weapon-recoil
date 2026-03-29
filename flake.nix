{
  description = "q2e-weapon-recoil - sharper, configurable full-arsenal recoil for Quake II Enhanced";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [
          pkgs.cmake
          pkgs.git
          pkgs.ninja
          pkgs.zig
        ];

        shellHook = ''
          echo "q2e-weapon-recoil cross-compilation shell"
          echo "Build: cmake --fresh -B build -G Ninja --toolchain cmake/zig-windows.cmake && cmake --build build"
          echo "Staged mod: build/dist/q2e-weapon-recoil"
        '';
      };
    };
}
