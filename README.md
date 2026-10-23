# teleport-engine

Software renderer for portal-linked sectors, where portals can teleport
you to the other side of the map.

## Build

Needs SFML 2 and `pkg-config`. Nix pins both:

```sh
nix run        # build and play
nix develop    # shell with SFML 2, make and pkg-config
make run       # inside that shell
```

Without Nix, install SFML 2 and `pkg-config` yourself, then `make run`.
Distros now ship SFML 3 (e.g. Ubuntu 26.04's `libsfml-dev`), which this
code doesn't support yet. On macOS:

```sh
brew install sfml@2 pkg-config
export PKG_CONFIG_PATH="$(brew --prefix sfml@2)/lib/pkgconfig"
```

On Linux outside NixOS, Nix-built OpenGL apps don't see the system GPU
drivers: run the game through [nixGL](https://github.com/nix-community/nixGL).

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` | Move |
| `Q` `E`, `←` `→` | Turn |
| `Z` `X` | Raise, lower eye height |
| `[` `]` | Slow down, speed up time |
