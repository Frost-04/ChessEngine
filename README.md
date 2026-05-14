# Porcelain
### *A C++ based Chess Engine*

A from-scratch C++20 chess engine implementing the core systems behind modern chess software: bitboard-based board representation, pseudo-legal move generation with full legality filtering, FEN position loading, SAN/algebraic input parsing, Zobrist hashing, perft validation, static evaluation, and depth-limited negamax search with alpha-beta pruning.

This project is designed as more than a playable command-line chess program. It is a systems-level implementation of a chess engine architecture, built to demonstrate performance-aware data structures, reversible game-state management, rule-complete move execution, deterministic position hashing, and search/evaluation techniques commonly used in engine development.

The engine supports all standard chess move rules, including castling, en passant, promotion, check detection, make/undo move semantics, and benchmark-driven move-generation validation through built-in perft suites. It also includes an interactive CLI for human-vs-engine play and a dedicated test runner for validating evaluation, search, and move-generation correctness.

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue)
![Engine](https://img.shields.io/badge/type-chess%20engine-purple)
![Status](https://img.shields.io/badge/status-active-brightgreen)

## Features

- **Bitboard board representation** with one 64-bit board per piece type.
- **Legal move handling** for:
  - normal piece moves
  - captures
  - castling
  - en passant
  - pawn promotion
  - double pawn pushes
- **FEN loader** for importing arbitrary chess positions.
- **SAN/algebraic parser** for user-friendly move input such as `e4`, `Nf3`, `exd5`, `O-O`, and `a8=Q`.
- **Perft test runner** with standard benchmark positions for validating move generation.
- **Static evaluator** using material values and piece-square tables.
- **Negamax alpha-beta search** with:
  - iterative deepening at the root
  - quiescence search
  - basic MVV-LVA capture ordering
  - repetition detection support via Zobrist hashes
- **Interactive CLI game mode** where a human can play against the engine.

## Project Structure

```text
.
├── algebraic_parser.cpp/.h   # SAN/algebraic move parser
├── board.cpp/.h              # Board state, make/undo move, attack detection
├── constants.h               # Bitboard square constants, flags, movement offsets
├── engine_tests.cpp          # Interactive test/perft/evaluation/search runner
├── evaluator.cpp/.h          # Static evaluation
├── fenLoader.cpp/.h          # FEN parsing and board loading
├── main.cpp                  # CLI game against the engine
├── movegenerator.cpp/.h      # Pseudo-legal move generation
├── perft.cpp/.h              # Perft and divide utilities
├── search.cpp/.h             # Negamax search and quiescence
├── structs.h                 # Core structs/enums: Piece, Move, Undo, MoveList
└── zorbist.h                 # Zobrist hashing table
```

## Requirements

- A C++20-compatible compiler.
  - Tested-style command examples use `g++`.
- Standard C++ library only; no external runtime dependencies are required.

The code uses C++20 facilities such as `<bit>` (`std::countr_zero`, `std::popcount`), so compile with `-std=c++20` or newer.

## Getting Started

### 1. Clone the repository

```bash
git clone <your-repository-url>
cd <your-repository-directory>
```

### 2. Build the playable engine

```bash
g++ -std=c++20 -O2 main.cpp board.cpp movegenerator.cpp evaluator.cpp search.cpp algebraic_parser.cpp fenLoader.cpp perft.cpp -o chess_engine.exe

```

### 3. Run the engine

```bash
./chess_engine
```

You will be prompted for:

1. search depth
2. whether you want to play as White or Black

Example interaction:

```text
Enter search depth: 3
Play as (w)hite or (b)lack? w
Your move (e4, Nf3, O-O). Type 'quit' to exit: e4
```

Supported user move examples:

```text
e4
Nf3
Bb5
exd5
O-O
O-O-O
a8=Q
Qxf7+
```

## Running Tests and Perft

Build the test runner:

```bash
g++ -std=c++20 -O2 engine_tests.cpp board.cpp movegenerator.cpp evaluator.cpp search.cpp algebraic_parser.cpp fenLoader.cpp perft.cpp -o engine_tests.exe

```

Run it:

```bash
./engine_tests
```

The test menu includes:

```text
==== Engine Tests ====
1) Run all tests
2) Perft suite (built-in positions)
3) Perft custom FEN
4) Evaluation sanity tests
5) Search sanity tests
0) Exit
```

### Built-in perft positions

The perft runner includes common validation positions such as:

- Initial position
- Kiwipete
- Several additional tactical/special-rule positions

Example custom perft workflow:

```text
Select option: 3
Enter FEN (or 'q' to cancel): rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Enter depth: 4
Perft nodes: 197281
```

## Using FEN Positions

FEN strings are loaded through `FenLoader`:

```cpp
#include "board.h"
#include "fenLoader.h"

Board board;
FenLoader loader;
std::string error;

if (!loader.load(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &error)) {
    // handle error
}
```

The loader validates:

- six FEN fields
- piece placement shape
- valid active color
- castling rights field
- en passant square format
- halfmove/fullmove counters
- exactly one white king and one black king

## Parsing Algebraic Moves

Use `AlgebraicParser::parse` to convert SAN-like user input into a `Move`:

```cpp
#include "algebraic_parser.h"

Move move{};
std::string error;

if (AlgebraicParser::parse(board, "Nf3", move, &error)) {
    board.makeMove(move);
} else {
    // error contains a human-readable reason
}
```

The parser matches the input against generated legal moves, so ambiguous or illegal SAN input is rejected.

## Engine Overview

### Board and move representation

- Squares use `A1 = 0` through `H8 = 63`.
- Bitboards use LSB as `A1`.
- `Board::makeMove()` applies a move and rejects it if the moving side's king remains or becomes attacked.
- `Board::undoMove()` restores the previous state using an undo stack.

### Move generation

`MoveGenerator::nextValidMoves()` generates pseudo-legal moves. Legal filtering is performed by attempting `Board::makeMove()` and undoing successful moves.

### Search

`Search::bestMove()` performs iterative deepening from depth `1` to the requested maximum depth using negamax alpha-beta search. At leaf nodes, quiescence search extends capture/check situations to reduce horizon effects.

### Evaluation

`Evaluator::evaluate()` scores a position from the side-to-move perspective using:

- material values
- piece-square tables
- a middlegame king table

## Development Notes

- Keep move-generation changes validated with perft before changing search/evaluation.
- For debugging move generation, use `Perft::divide(board, depth)` to print per-move node counts.
- The engine currently focuses on correctness and clarity over advanced performance features.

Potential future improvements:

- transposition table
- killer/history move ordering
- principal variation reporting
- UCI protocol support
- opening book
- endgame-specific evaluation
- stronger checkmate/game-result reporting in the CLI

## Contributing

Contributions are welcome. A good workflow is:

1. Create a focused branch.
2. Add or update tests/perft coverage for move-generation changes.
3. Build with warnings enabled.
4. Run the relevant test menu options.
5. Open a pull request with a concise description of the change.

