# teleport-engine

Software renderer for portal-linked sectors, where portals can teleport
you to the other side of the map.

## Build

Needs GLFW 3, libepoxy, stb and `pkg-config`, plus OpenGL 3.3. Nix pins
all of them:

```sh
nix run        # build and play
nix develop    # shell with the libraries, make and pkg-config
make run       # inside that shell
```

Without Nix, install them yourself, then `make run`. On Debian/Ubuntu:

```sh
sudo apt install libglfw3-dev libepoxy-dev libstb-dev pkg-config
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
