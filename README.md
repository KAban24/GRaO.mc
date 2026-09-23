# GRaO.mc — Million-Cell Minesweeper

**G**reat **R**ender **a**nd **O**ptimization · **m**illion **c**ells

A minesweeper that doesn't choke on 1000×1000 fields (1,000,000 cells). DirectX 11, SIMD/AVX2, immediate-mode rendering via ImGui. No allocations in the hot path, no GC, no whiny slow frameworks.

---

## Features

### Rendering
- **DirectX 11** + DXGI Flip Discard — minimal latency, no tearing
- **Frustum culling** — only visible cells are rendered, everything else is discarded
- Everything drawn directly through `ImDrawList` — no textures for cells, zero state-change draw calls
- Zoom, panning (RMB/MMB drag, scroll wheel), WASD keys
- **Minimap** — full field overview, click to teleport camera, radar overlay

### Game Logic
- **1 byte per cell** — `fieldState` packed into bitfields (`#pragma pack(push, 1)`) with `static_assert` guarantee
- **AVX2 SIMD** in `CheckWinConditionSIMD` — win condition checked 32 cells per iteration via `_mm256_*` intrinsics and `__popcnt`
- **BFS flood fill** (`IterativeUnlock`) — iterative stack instead of recursion, no stack overflow on large fields
- Safe 3×3 zone around the first click — mines are placed after the first click
- Accurate timer using `std::chrono::steady_clock`

### UI/UX
- 5 visual themes: **Cyber Emerald**, **Cyberpunk Neon**, **Classic Win95** (retro), **Sunset Vaporwave**, **Deep Space**
- Animated backgrounds per theme — matrix rain, synthwave sunset, stars, comets
- Spring-overshoot menu animations (easing with overshoot)
- Particle engine: confetti on win, explosion on defeat, screen shake
- Procedural audio — synthesized directly into `waveOut` PCM on the fly, no sound files whatsoever
- Fullscreen via F11 using `SetWindowPlacement` + `DXGI_MWA_NO_ALT_ENTER`
- RU/EN localization from a single `LocStrings` table
- Keyboard navigation W/S/Enter/Escape through menus

### Presets
| Mode | Size | Mines |
|---|---|---|
| 🌱 Beginner | 9 × 9 | 10 |
| 🌿 Intermediate | 16 × 16 | 40 |
| 🔥 Expert | 30 × 16 | 99 |
| ⚡ Master | 100 × 100 | 1,500 |
| 👑 Legend | 1000 × 1000 | 150,000 |
| 🛠️ Custom | 5–1000 × 5–1000 | up to 35% |

---

## Architecture

```
GRaO.mc/
├── main.cpp          — RenderApp, ParticleEngine, menus, game UI, WndProc
├── GameLogic.h       — MinesweeperBackend (BFS, SIMD win-check, fieldState)
├── SoundEngine.h     — PCM synthesis in a detached thread, waveOut
├── Theme.h           — VisualTheme, 5 themes
├── UIComponents.h    — DrawPillButton, DrawCyberPanel, easing, sliders
└── ImGui/            — imgui + dx11/win32 backends
```

### Field Data Structure

```cpp
#pragma pack(push, 1)
struct fieldState {
    uint8_t visible      : 1;
    uint8_t mine         : 1;
    uint8_t flaged       : 1;
    uint8_t nearbyNumber : 1;
    uint8_t bombNearby   : 4;
};
#pragma pack(pop)
// sizeof(fieldState) == 1  <-- guaranteed by static_assert
```

1,000,000 cells = **1 MB RAM**. The entire array fits in L3 cache.

### SIMD Win-Check (AVX2)

```cpp
__m256i maskVisible = _mm256_set1_epi8(0x01);
__m256i maskMine    = _mm256_set1_epi8(0x02);
// 32 bytes per iteration -> popcount -> sum of revealed non-mines
```

---

## Building

**Requirements:** MSVC 19.x+, CMake 3.20+, Windows 10/11, D3D11-capable GPU

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Or open `CMakeLists.txt` in CLion / Visual Studio directly.

**Compiler flags:** `/O2 /Oi /Ot /arch:AVX2 /MP`

> AVX2 is required — without it `CheckWinConditionSIMD` won't compile. Needs Intel Haswell (2013) / AMD Ryzen (2017) or newer.

**System font dependencies:** `segoeuib.ttf`, `segoeui.ttf`, `seguiemj.ttf`, `seguisym.ttf` — all ship with standard Windows.

---

## Controls

### Menus
| Key | Action |
|---|---|
| W / S | Select item |
| Enter | Confirm |
| Escape | Back |
| F11 | Toggle fullscreen |

### In-Game
| Action | Control |
|---|---|
| Reveal cell | LMB |
| Place flag | RMB (no drag) |
| Pan camera | RMB / MMB + drag |
| Zoom | Scroll wheel |
| Pan with keys | WASD / Arrow keys |
| Reset | R |
| New game / reset | 🙂 button in top bar |
| Toggle radar | Button in top bar |
| Teleport camera | LMB on minimap |

---

## Technical Highlights

- **No heap allocations per frame** — all containers (particles, BFS stack) pre-reserve memory upfront
- `SoundEngine` — pure PCM synthesis: sine waves with envelope, exponential decay, FM for different sound types. Zero wav/mp3 files
- `SpawnExplosion` / `SpawnConfetti` — two particle modes with distinct physics: explosion (radial fade) vs confetti (gravity + horizontal drag)
- **Win95 retro theme** switches the entire render pipeline: beveled borders, titlebar gradient, classic buttons
- Custom board setup lets you tune mine density from 1% to 35%, with auto-calculated difficulty label
- Aviasales easter egg: click the MINESWEEPER title 5 times in a row
