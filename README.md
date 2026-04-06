# q2e-weapon-recoil

Weapons recoil for Quake II Remastered. Adds recoil to all weapons to make them feel punchier and more fun to shoot. Recoil is configurable per weapon.

## What It Does

- Applies configurable recoil to the full weapon set instead of only the machinegun family.
- Makes hitscan and projectile weapons feel sharper and more consistent.
- Preserves reversed recoil on chaingun-style weapons such as the chainfist.
- Adds a short smooth return to neutral when firing stops.

## How to Install

1. Download the latest release zip from [the latest release page](https://github.com/ChrisDeadman/q2e-weapon-recoil/releases), or build the mod yourself and use the files from `build/dist/q2e-weapon-recoil/`.
2. Extract or copy the `q2e-weapon-recoil` folder so it contains `game_x64.dll` and `description.txt`.
3. Place that folder in `%USERPROFILE%\\Saved Games\\Nightdive Studios\\Quake II\\`.
4. Launch the game with `+set game q2e-weapon-recoil`.

## Recoil Parameters

The mod exposes archived per-weapon recoil cvars through the console. Each weapon uses a name-derived cvar such as `g_recoil_machinegun`, `g_recoil_chaingun`, `g_recoil_rocketlauncher`, or `g_recoil_bfg`.

Unknown weapons added by other mods also pick up a derived cvar name from their weapon name and fall back to sensible defaults until overridden.

One shared timing cvar, `g_recoil_time`, controls both how long recoil stays active and how long it takes to smooth back down. The default is `0.1` seconds.

Examples:

```sh
set g_recoil_machinegun 1.0
set g_recoil_chaingun 0.5
set g_recoil_bfg 8.0
set g_recoil_time 0.1
```

Higher values increase recoil for that weapon. Setting a recoil cvar to `0` disables recoil for that weapon.

## How to Build

1. Initialize submodules:

```sh
git submodule update --init --recursive
```

2. Configure and build the Windows DLL:

```sh
cmake --fresh -B build -G Ninja --toolchain cmake/zig-windows.cmake
cmake --build build
```

3. Or run the same steps using the Nix flake:

```sh
nix develop -c cmake --fresh -B build -G Ninja --toolchain cmake/zig-windows.cmake
nix develop -c cmake --build build
```

The staged mod output is written to `build/dist/q2e-weapon-recoil/`.