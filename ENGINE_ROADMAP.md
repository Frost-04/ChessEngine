# Chess Engine Roadmap (Step-by-Step)

This roadmap builds on your current code and gives a bit more detail on *what* to do next and *roughly how* to do it, without going too deep into theory.

---

## 1. Strengthen the Board Class

**Goal:** Make it easy to update the board when a move is played.

**What to add:**
- A way to move a single piece from one square to another.
- A simple move structure to describe a move.

**How (high level):**
- Represent squares by an index 0–63 (you already implicitly do this with bits).
- Create a small `struct Move { int from; int to; /* later: flags for promotion, castling, etc. */ };`
- Write a `makeMove(const Move& m)` method:
  - Figure out which piece (and color) is on `from`.
  - Clear the bit for `from` in that piece’s bitboard.
  - If there is an opponent piece on `to`, clear that bit from the opponent’s bitboard (capture).
  - Set the bit for `to` in the moving piece’s bitboard.
  - Flip `whiteToMove`.

At this stage, ignore special moves (no promotions, no castling, no en passant). Just handle normal moves.

---

## 2. Basic Move Generation

**Goal:** Be able to list all possible moves for the side to move (even if not yet fully “legal”).

**What to add:**
- Separate functions for each piece type, for example:
  - `generatePawnMoves(...)`
  - `generateKnightMoves(...)`
  - `generateBishopMoves(...)`, etc.
- A container (like `std::vector<Move>`) to store moves.

**How (high level):**
- For each piece type of the side to move:
  - Loop through all bits set in that piece’s bitboard (you can scan from 0 to 63 and check `getBit(...)`).
  - For each piece square, compute its target squares based on how that piece moves.
- Start very simple:
  - **Pawns:** only single-step pushes and captures (no double push, no en passant, no promotion yet).
  - **Knights:** precompute the 8 possible jumps from a square and keep only those on board that are not blocked by your own pieces.
  - **Kings:** one square in any direction, again making sure not to land on your own piece.
- For each target square, if it is:
  - Empty: add a normal move.
  - Occupied by opponent: add a capture move.

Don’t worry about checks yet—these are “pseudo-legal” moves.

---

## 3. Filtering Out Illegal Moves (King in Check)

**Goal:** Only keep moves that do not leave your own king in check.

**What to add:**
- A function like `bool isSquareAttacked(int square, bool byWhite)` to test if a square is attacked.
- A wrapper that turns pseudo-legal moves into legal moves.

**How (high level):**
- To get legal moves:
  - Generate all pseudo-legal moves for the current side.
  - For each move:
    - Make a *temporary copy* of the board.
    - Apply the move on the temp board using `makeMove`.
    - Find your king’s square in the temp position.
    - Use `isSquareAttacked` to check if that king square is attacked by the opponent.
    - If the king is safe, keep the move; otherwise discard it.

This is slower but very simple and clear, which is perfect for a first version.

---

## 4. Simple Evaluation Function

**Goal:** Give a numeric score to a position (positive = good for White, negative = good for Black).

**What to add:**
- A function like `int evaluate()` inside `Board`.

**How (high level):**
- Assign basic values, for example:
  - Pawn = 100
  - Knight = 300
  - Bishop = 300
  - Rook = 500
  - Queen = 900
- For each piece type:
  - Count how many bits are set in its bitboard (you can write a simple popcount loop or use builtin functions if available).
  - Multiply count by its value, add for White, subtract for Black.
- Return the final total score.

At this stage, no positional terms are needed. Material only is fine.

---

## 5. Minimax + Alpha-Beta

**Goal:** Let the engine look ahead a few moves and pick the best one.

**What to add:**
- A recursive search function, for example:
  - `int search(int depth, int alpha, int beta)`
- A helper function: `Move findBestMove(int depth)`.

**How (high level):**
- Base case (when `depth == 0`):
  - Return `evaluate()`.
- Recursive case:
  - Generate all legal moves for the side to move.
  - If no legal moves:
    - If the king is in check: return a large negative or positive value (checkmate).
    - Otherwise: return 0 (stalemate).
  - For each move:
    - Make a copy of the board.
    - Apply the move on the copy.
    - Call `search(depth - 1, -beta, -alpha)` on the child position and *negate* the result (negamax style).
    - Update `alpha` with the maximum of current `alpha` and this value.
    - If `alpha >= beta`, break out early (alpha-beta pruning).
  - Return `alpha`.
- `findBestMove`:
  - Loop over all legal moves at the root.
  - For each move, call `search(depth - 1, -INF, INF)` on the resulting board.
  - Keep the move that gives the best score for the side to move.

Keep the first implementation very small depth (e.g., depth 2 or 3) to keep it fast.

---

## 6. Simple Game Loop

**Goal:** Play against the engine from the console.

**What to add:**
- In `main`, replace the one-time print with a loop.

**How (high level):**
- Pseudo-code:
  - Initialize board.
  - While the game is not over:
    - Print the board.
    - If it’s the human’s turn:
      - Ask for a move like `e2e4`.
      - Convert that to a `Move` (file/rank to index 0–63).
      - Check that this move is in the list of legal moves; if not, ask again.
      - Apply the move.
    - Else (engine’s turn):
      - Call `findBestMove(depth)`.
      - Apply the chosen move.

You can start with both sides being the engine (engine vs. itself) to test easily.

---

## 7. Gradual Improvements

Once everything above works in a basic way, you can improve:

- **Move generation:**
  - Add pawn double pushes, promotions, castling, and en passant.
- **Evaluation:**
  - Add small bonuses/penalties for development, king safety, pawn structure, etc.
- **Search:**
  - Add iterative deepening (search depth 1, then 2, then 3, etc.).
  - Add a simple transposition table (cache evaluations for positions you’ve already seen).

You don’t need all of this to get a working engine. The key is to follow the steps in order: first make moves work, then generate them, then evaluate, then search.
