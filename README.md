# teleport-engine

Software-rendered portal engine, where portals can teleport you to the
other side of the map, and sight, bullets and fireballs follow. Runs
natively and in the browser, multithreaded on both.

The demo is a small Doom-like shooter: clear the test map of demons.
The secret room up the stairs has a shotgun.

## Build

Needs GLFW 3, libepoxy, stb, nlohmann_json and `pkg-config`, plus
OpenGL 3.3. Nix pins all of them:

```sh
nix run        # build and play
nix develop    # shell with the libraries, make and pkg-config
make run       # inside that shell
make test      # headless checks: maps, movement, portals, traces
```

Without Nix, install them yourself, then `make run`. On Debian/Ubuntu:

```sh
sudo apt install libglfw3-dev libepoxy-dev libstb-dev nlohmann-json3-dev pkg-config
```

On Linux outside NixOS, Nix-built OpenGL apps don't see the system GPU
drivers: run the game through [nixGL](https://github.com/nix-community/nixGL).

### Web

The same code builds to WebAssembly with Emscripten and runs in any
browser with WebGL 2:

```sh
nix build .#web          # result/: index.html, teleport*.{js,wasm,data}
nix develop .#web        # shell with Emscripten, make and pkg-config
make PLATFORM=web run    # inside that shell: http://localhost:8080
                         # (PORT=8081 to pick another)
```

Serve the files over HTTP, browsers don't load `.wasm` from `file://`.
`index.html` picks one of two builds:

- `teleport-mt`: threaded, needs a cross-origin isolated page. The
  server must send `Cross-Origin-Opener-Policy: same-origin` and
  `Cross-Origin-Embedder-Policy: require-corp` (`emrun` does)
- `teleport`: single-threaded, runs anywhere

## Controls

| Key | Action |
|---|---|
| Click | Capture the mouse and play; Esc pauses |
| Mouse, `Q` `E`, `←` `→` | Look, turn |
| `W` `A` `S` `D`, `↑` `↓` | Move |
| Space | Jump |
| Left click, Ctrl | Shoot |
| `1` `2` | Pistol, shotgun |
| `R` | Restart after dying or clearing the map |
| F1 | Stats: FPS, render time, threads |
| F2 | Render scale: 1, 2, 3, 4 screen pixels per pixel |
| F11 | Fullscreen |

## Options

Native flags, or the page's query string on the web
(`index.html?bench&scale=1`):

| Flag | |
|---|---|
| `--bench [frames]` | Turn in place, print render times, quit |
| `--size WxH` | Window size |
| `--scale N` | Render scale |
| `--threads N` | Render threads, default: all, up to 16 |

## Engine

Everything under `Engine/` is game-agnostic, `Game/` is the demo built
on it. A game implements `te::App`: `update()`, `getCamera()`,
`drawHud()`, and hands it to `te::Engine::run()`.

| Part | |
|---|---|
| `Render/Renderer` | Portal renderer: walls, floors, ceilings, sprites with depth; screen strips in parallel |
| `World/World` | Entities, sliding collision, gravity, stairs, portal crossing, `trace()` for hitscan and sight |
| `World/MapLoader` | JSON maps; the format is documented in `MapLoader.hpp` |
| `Core/Engine` | Window, input, main loop, benchmark |
| `Graphics/Canvas` | 2D drawing into the frame, for HUDs |

Maps are convex sectors joined by portals. A portal pairs two walls of
equal length anywhere on the map, at any angle: neighbors are just
portals whose walls coincide. See `Resources/Maps/teleport.json`.

Sprites are original pixel art drawn by `Tools/make_sprites.py`
(needs Pillow). Wall textures are from Wolfenstein 3D.
