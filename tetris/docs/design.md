# Tetris — Design Document

Terminal-based Tetris clone written in C++. Fully playable in the terminal using ANSI escape codes and raw mode input.

---

## Table of Contents

1. [System Architecture](#1-system-architecture)
2. [Project Structure](#2-project-structure)
3. [Data Model](#3-data-model)
4. [Key Workflows](#4-key-workflows)
5. [Development Phases](#5-development-phases)

---

## 1. System Architecture

Five systems with clear boundaries:

| System | Responsibility |
|---|---|
| **Terminal** | Raw mode setup, non-blocking stdin, ANSI cursor control |
| **Tetromino** | 7-piece definitions, 4 rotation states each |
| **Board** | 10×20 grid state, collision detection, line clearing |
| **Renderer** | Cursor-positioned draw calls, double-buffer to prevent flicker |
| **Game Loop** | Gravity timing, input dispatch, game state machine |

---

## 2. Project Structure

```
tetris/
├── docs/
│   └── design.md
├── main.cpp          # game loop, state machine, entry point
├── board.h/.cpp      # grid, collision detection, line clearing
├── tetromino.h/.cpp  # piece data, rotation logic
├── renderer.h/.cpp   # all ANSI/terminal rendering
├── input.h/.cpp      # raw mode, non-blocking key reads
└── tetris            # compiled binary
```

### Build

```bash
g++ -std=c++17 -O2 -o tetris/tetris tetris/main.cpp tetris/board.cpp tetris/tetromino.cpp tetris/renderer.cpp tetris/input.cpp
```

---

## 3. Data Model

### Board

```cpp
int grid[22][10];
```

Values: `0` = empty, `1–7` = piece color index. Rows 0–1 are hidden spawn rows above the visible 20-row playfield.

### Tetromino

Each piece holds 4 rotation states as coordinate offsets from a pivot point:

```cpp
struct Tetromino {
    std::array<std::array<std::pair<int,int>, 4>, 4> rotations; // [rotation][cell]
    int colorId;
};
```

The 7 pieces (I, O, T, S, Z, J, L) map to color IDs 1–7.

### Active Piece

```cpp
struct ActivePiece {
    int type;       // 0–6 (I, O, T, S, Z, J, L)
    int rotation;   // 0–3
    int row, col;   // pivot position on board
};
```

### Game State

```cpp
struct GameState {
    Board board;
    ActivePiece current, next;
    int score, level, linesCleared;

    enum class Phase { Playing, Paused, GameOver };
    Phase phase;
};
```

---

## 4. Key Workflows

### 4.1 Terminal Setup / Teardown

`input.cpp` uses `termios` to enable raw mode on startup:

- `ICANON` and `ECHO` disabled — input is available immediately, one byte at a time, with no terminal echo
- `VMIN=0`, `VTIME=0` — reads are non-blocking; returns 0 bytes if nothing is available
- Original terminal settings restored via RAII destructor or `atexit`

Arrow keys arrive as 3-byte escape sequences (`\033[A/B/C/D`) and must be consumed atomically. All key codes are mapped to an enum before entering the game loop.

### 4.2 Game Loop

```
last_gravity = now()

loop:
    key = read_input()             // non-blocking
    dispatch key:
        ← →     → try move current left/right
        ↓        → accelerated drop (soft drop)
        ↑ / Z    → try rotate
        Space    → hard drop (teleport to lowest valid position)
        P        → toggle Paused
        Q        → quit

    if now() - last_gravity >= gravity_interval(level):
        if can_move(current, DOWN):
            current.row++
        else:
            lock_piece()           // write current cells into board.grid
            lines = clear_lines()  // scan, compact rows, update score
            spawn_next()
            if collision(current): // spawn position overlaps existing blocks
                phase = GameOver
        last_gravity = now()

    render(gameState)
```

**Gravity interval** follows the NES speed curve:

```
interval_ms = (48 - (level - 1) * 5) * (1000 / 60)
```

Clamped to a minimum of ~50ms at high levels.

### 4.3 Rotation (SRS-lite)

1. Compute the 4 new cell positions for `(rotation + 1) % 4`
2. Test for collision in place
3. If collision, try wall kicks in order: shift left 1, right 1, left 2, right 2 (I-piece uses wider ±2 offsets)
4. Accept the first non-colliding offset; if all fail, cancel the rotation

### 4.4 Collision Detection

A piece collides if any of its 4 cells would land on:

- A column outside `[0, 9]`
- A row ≥ 20 (below the floor)
- A non-zero cell in `board.grid`

### 4.5 Line Clearing

1. Scan rows 2–21 bottom-up
2. For each full row: remove it, shift all rows above down by 1, prepend an empty row at the top
3. Count simultaneously cleared lines, apply scoring, update level

**Scoring** (NES formula):

| Lines Cleared | Points |
|---|---|
| 1 | 40 × (level + 1) |
| 2 | 100 × (level + 1) |
| 3 | 300 × (level + 1) |
| 4 (Tetris) | 1200 × (level + 1) |

Level increments every 10 lines cleared.

### 4.6 Rendering

- On startup: draw the static border frame, score panel, and next-piece preview area once
- Each frame: use `\033[row;colH` to overwrite only changed cells — avoid full-screen clears, which cause flicker
- Pieces rendered as colored block characters (`█` or `[]`) using ANSI 256-color or basic 8-color background codes, one color per piece type
- Next-piece preview drawn in a 4×4 area in the sidebar
- Score, level, and line count updated in fixed positions in the sidebar each frame

---

## 5. Development Phases

| Phase | Deliverable |
|---|---|
| 1 | Terminal layer — raw mode, non-blocking read, key enum |
| 2 | Static render — empty board + border via cursor positioning |
| 3 | Tetromino data — all 7 pieces and their 4 rotation states |
| 4 | Spawn + render — active piece appears and displays on board |
| 5 | Collision + movement — left/right/down input with boundary checks |
| 6 | Rotation — with wall kicks |
| 7 | Gravity loop — timed auto-drop using `std::chrono` |
| 8 | Lock + line clear — piece locking, row removal, scoring |
| 9 | Level progression — speed curve, next-piece preview |
| 10 | Game over + restart — detection, end screen, replay prompt |

Phases 1 and 3 are independent and can be implemented in parallel.
