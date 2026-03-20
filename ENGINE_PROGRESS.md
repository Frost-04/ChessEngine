# Chess Engine Progress Overview

## Current Status (What’s Done)

- Board representation uses 64-bit bitboards for each piece type and color.
- Initial position setup (standard chess starting position) is implemented in `Board::ResetBoard()`.
- Basic bitboard utilities exist: get, set, and reset a bit by index.
- A simple text-based board display (`Board::printBoard`) shows piece placement on the console.
- Turn tracking is started via `whiteToMove` (whose turn it is).

## Next Steps (High-Level Roadmap)

1. **Improve Board Representation & Helpers**
   - Add a way to identify which side a piece belongs to when printing (e.g., uppercase for White, lowercase for Black, or a prefix).
   - Add helper functions to move pieces from one square to another (update the correct bitboards, handle captures).
   - Add a simple way to input or apply a move (e.g., from-square/to-square notation like "e2e4").

2. **Move Generation**
   - Implement legal move generation for each piece type (start with pawns, knights, and kings).
   - Add basic rules: captures, promotions, castling, en passant, and checks (these can come gradually; start simple).
   - Create a function that, given a board and side to move, returns all pseudo-legal moves, then filter out moves that leave your own king in check.

3. **Position Evaluation**
   - Implement a basic evaluation function (e.g., material balance: piece values for pawns, knights, bishops, rooks, queens).
   - Gradually add simple positional factors later (piece activity, king safety, pawn structure), but keep it basic at first.

4. **Minimax (Search) Framework**
   - Implement a basic minimax search that explores moves to a fixed depth using your move generator and evaluation.
   - Add alpha-beta pruning to make the search faster.
   - Add a simple search driver that picks the best move for the current side.

5. **Game Loop & Interface**
   - Create a loop where a human can input moves and the engine responds with its chosen move.
   - Reuse `printBoard` (or improve it) to display the board after each move.
   - Add basic move legality checking for human input.

6. **Testing & Debugging Tools**
   - Add unit tests or simple test positions to verify move generation and evaluation.
   - Add debug printing options for bitboards, moves, and evaluation scores.

## Suggested Order to Implement

1. Strengthen board helpers (moving pieces, applying moves).
2. Implement move generation for a subset of rules (no castling/en passant at first).
3. Add a very simple evaluation (material only).
4. Implement minimax with alpha-beta and hook it into a basic game loop.
5. Gradually extend move generation and evaluation for more realistic play.

This overview is meant to guide you step-by-step from the current basic board setup toward a working minimax-based chess engine.