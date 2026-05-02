# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

A collection of terminal-based arcade games written in C++. Each game lives in its own subdirectory with a single `.cpp` source file and a compiled binary alongside it.

Current games:
- `hangman/` — complete (two-player: one types the word, one guesses)
- `tetris/` — in progress (see `tetris/docs/design.md`)

## Building and Running

No build system (no Makefile, no CMake). Compile each game directly with `g++`:

```bash
# Hangman
g++ -o hangman/hangman hangman/hangman.cpp

# Tetris
g++ -std=c++17 -O2 -o tetris/tetris tetris/main.cpp tetris/board.cpp tetris/tetromino.cpp tetris/renderer.cpp tetris/input.cpp
```

## Architecture Patterns

Each game is a **single directory** under `arcade/`. Hangman is one self-contained `.cpp` file; Tetris is split across multiple files due to complexity.

**Conventions established in `hangman.cpp`:**
- ANSI escape codes for terminal colors (`\033[1;31m` red, `\033[1;32m` green, `\033[0m` reset) and cursor control (`\033[2J\033[H` to clear screen)
- Input validation loops that use `std::cin.clear()` + `std::cin.ignore(...)` to recover from bad input
- All state in `main()` or passed by reference; no heap allocation beyond STL containers

**Tetris-specific:**
- Raw terminal mode via `termios` (`input.cpp`) for non-blocking, single-keypress input
- Cursor-positioned rendering (`\033[row;colH`) to overwrite individual cells without full-screen clears
- See `tetris/docs/design.md` for full system design, data model, and development phases
