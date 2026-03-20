## Section 1: Current System Analysis (Software Architect View)

### 1.1 Summary of Current Code
The provided code implements a chess board representation using **bitboards** (64-bit integers) for each piece type and color. It includes:
- Board initialization (`ResetBoard`) to the standard starting position.
- Basic bit manipulation helpers (`GetBit`, `SetBit`, `ResetBit`).
- Utility functions to retrieve combined piece sets (`GetWhitePieces`, `GetBlackPieces`, `GetAllPieces`).
- A function to find the position of a single piece (intended for the king) using a linear scan.
- A function to get the bitboard pointer for a given square (`getPieceBitboardAtSquare`).
- An attack detection function (`isSquareAttacked`) that checks if a square is attacked by a given color, covering pawns, knights, sliding pieces, and the king.
- An obstruction check for sliding moves (`isMoveObstructed`).
- A move legality checker (`moveLegalityCheck`) that validates a move according to piece-specific rules (excluding castling, en passant, and promotion).
- A move execution function (`makeMove`) that attempts to move a piece, checks if the move leaves the moving side’s king in check, and updates the board and turn if legal.
- A placeholder for move generation (`listOfValidMoves` is empty).
- A simple ASCII board printer.

The code compiles and runs a minimal demonstration (prints the starting board).

### 1.2 Implemented Components
| Component                | Status | Description |
|--------------------------|--------|-------------|
| Board representation     | ✅     | Bitboards for each piece type/color. |
| Board reset              | ✅     | Sets up initial position. |
| Bit manipulation         | ✅     | Set, reset, test bits. |
| Attack detection         | ✅     | Checks if a square is attacked by a color (supports pawns, knights, sliding, king). |
| Move obstruction check   | ✅     | Checks if intermediate squares are occupied for sliding moves. |
| Piece‑specific legality  | ✅     | Validates moves for each piece (pawn, knight, bishop, rook, queen, king) – no special moves yet. |
| Move execution with king‑safety | ✅ | `makeMove` applies a move, verifies king is not left in check, and toggles turn. |
| Board printing            | ✅     | ASCII representation. |

### 1.3 Missing Components
| Component                | Required For                 |
|--------------------------|------------------------------|
| Full move generation     | Generating all legal moves from a position. |
| Castling, en passant, promotion | Completeness of chess rules. |
| Move list storage        | Representing moves (from, to, promotion piece, etc.). |
| Evaluation function      | Scoring a position for the AI. |
| Minimax search           | Decision‑making for the AI. |
| Alpha‑beta pruning       | Improving search efficiency. |
| Move ordering            | Enhancing alpha‑beta performance. |
| Game loop / UI           | Interacting with a human or UCI. |
| Transposition tables     | Avoiding re‑search of identical positions. |
| Iterative deepening      | Time management and better move ordering. |
| Opening book / endgame   | Advanced performance. |

### 1.4 Architecture Diagram (Textual)

```
+---------------------+        +-----------------------+
|      Game Loop      |        |       User Input      |
| (turn management,   |<------>| (console or UCI)      |
|  display)           |        +-----------------------+
+---------------------+
         |
         | calls AI move
         v
+---------------------+
|  Minimax with       |
|  Alpha‑Beta         |
+---------------------+
         |
         | uses
         v
+---------------------+        +-----------------------+
|  Move Generation    |------->|   Move List (struct)  |
| (pseudo‑legal then  |        +-----------------------+
|  legality with king |
|  safety)            |
+---------------------+
         |
         | evaluates positions
         v
+---------------------+
|  Evaluation         |
| (material, PST,     |
|  mobility, etc.)    |
+---------------------+
         ^
         | consults
+---------------------+
|  Move Ordering      |
| (captures, killers, |
|  history)           |
+---------------------+

Data Flow:
1. Game loop receives a command (human move or "AI move").
2. For AI move: current board state → Move Generation → list of legal moves.
3. Move list is passed to Minimax (with move ordering) which recursively explores moves.
4. At leaf nodes, Evaluation returns a score.
5. Minimax returns the best move → Game loop updates board (via makeMove) and displays.
```

---

## Section 2: Roadmap & Design (Architect View)

### 2.1 Development Phases

| Phase | Focus | Deliverables |
|-------|-------|--------------|
| 1 | **Complete Move Generation** | – Implement generation of pseudo‑legal moves for all pieces.<br>– Add castling, en passant, pawn promotion.<br>– Create a move struct (from, to, flags).<br>– Implement `listOfValidMoves()` that returns a vector of legal moves (including king‑safety check).<br>– Test with known positions. |
| 2 | **Evaluation Function** | – Basic material count (piece values).<br>– Piece‑square tables for positional bonus.<br>– Simple mobility or king safety (optional).<br>– Return score from the perspective of the side to move. |
| 3 | **Minimax with Alpha‑Beta** | – Recursive search function.<br>– Integrate evaluation as leaf node score.<br>– Add alpha‑beta pruning.<br>– Return best move and its score. |
| 4 | **Move Ordering & Game Loop** | – Order moves: captures (MVV‑LVA), then killer moves, then history.<br>– Create a simple console game loop (human vs. AI).<br>– Allow entering moves in algebraic notation (optional). |
| 5 | **Performance Enhancements** | – Transposition table (caching evaluated positions).<br>– Iterative deepening.<br>– Quiescence search to avoid horizon effect. |
| 6 | **Advanced Features (Optional)** | – Opening book.<br>– UCI protocol support.<br>– Endgame tablebases.<br>– Multi‑threading. |

### 2.2 Module Definitions

#### 2.2.1 Move Generation Module
- **Responsibility**: Produce a list of all legal moves from a given board position.
- **Input**: `Board` object.
- **Output**: `std::vector<Move>` where each `Move` contains `from`, `to`, `promotion` (for pawns), and flags (castling, en passant).
- **Implementation approach**: Iterate over all pieces of the side to move, generate pseudo‑legal moves, then filter out those that leave the king in check (using a lightweight “make move and test” function).

#### 2.2.2 Evaluation Module
- **Responsibility**: Assign a numerical score to a board position (positive if side to move is better).
- **Input**: `Board` object.
- **Output**: `int` score.
- **Components**: Material sum, piece‑square tables, mobility, pawn structure, king safety.
- **Initial version**: Material + piece‑square tables only.

#### 2.2.3 Minimax with Alpha‑Beta
- **Responsibility**: Search the game tree to find the best move.
- **Input**: Current board, depth, alpha, beta, side to move.
- **Output**: Best move and its score.
- **Algorithm**:
  ```
  minimax(board, depth, alpha, beta, maximizingPlayer):
      if depth == 0 or game over:
          return evaluate(board)
      moves = generateMoves(board)
      orderMoves(moves)
      if maximizingPlayer:
          best = -INF
          for each move in moves:
              makeMove(move)
              score = minimax(board, depth-1, alpha, beta, false)
              undoMove(move)
              best = max(best, score)
              alpha = max(alpha, best)
              if beta <= alpha: break
          return best
      else:
          similarly for minimizing
  ```

#### 2.2.4 Move Ordering Module
- **Responsibility**: Sort moves to improve alpha‑beta pruning.
- **Heuristics**:
  - **Captures**: Most Valuable Victim – Least Valuable Attacker (MVV‑LVA).
  - **Killer moves**: Moves that caused a beta cutoff at same depth.
  - **History heuristic**: How often a move (from→to) caused a cutoff.
- **Implementation**: Assign each move a score and sort descending before search.

#### 2.2.5 Game Loop
- **Responsibility**: Manage turns, accept user input, invoke AI, display board.
- **Simple console version**:
  - While game not over:
    - Print board.
    - If human turn: read move (e.g., in algebraic), parse, validate, apply.
    - If AI turn: call minimax, apply best move.
    - Toggle turn.

### 2.3 Module Interactions and Data Flow

```
[Human]  -->  Game Loop  -->  [Board]  -->  Move Generation  -->  Move List
                  |                                                    |
                  v                                                    v
              [AI (Minimax)]  <----------------------------------  Move Ordering
                  |
                  v
            [Evaluation]
```

- **Game Loop** holds the current `Board` and side to move.
- When it’s AI’s turn, it calls **Minimax** with the board and desired depth.
- **Minimax** asks **Move Generation** for all legal moves, then **Move Ordering** sorts them.
- For each move, it makes the move on a copy (or uses undo) and recurses.
- At leaf nodes, **Evaluation** returns a score.
- The best move is returned to the Game Loop, which applies it to the real board.

---

## Section 3: Implementation Guide (Senior Developer View)

### 3.1 Move Generation (Phase 1)

#### Step‑by‑step

1. **Define a Move struct**:
   ```cpp
   struct Move {
       int from;          // 0..63
       int to;            // 0..63
       int promotion;     // 0=none, 1=knight, 2=bishop, 3=rook, 4=queen (or piece type)
       bool isCastling;   // optional, can be deduced from from/to
       bool isEnPassant;  // optional
   };
   ```

2. **Implement pseudo‑legal move generation for each piece type**:
   - **Pawns**: 
     - Single push (if square ahead empty).
     - Double push (if on starting rank and path clear).
     - Captures (diagonally, including en passant).
     - Promotion: when pawn reaches last rank, generate moves for each promotion piece.
   - **Knights**: all 8 jumps, if target square not occupied by own piece.
   - **King**: one step in all 8 directions, not onto own piece. Also castling (if not in check, squares between empty, not under attack).
   - **Sliding pieces (bishop, rook, queen)**: loop in each direction until board edge or piece; if opponent piece, add capture and stop; if own piece, stop; if empty, add move and continue.

3. **En passant**:
   - Need to store the en passant target square (the square behind the pawn that just moved two steps) in the board. Currently not tracked. Add a member `int enPassantSq` (set to -1 if none).
   - When a pawn makes a double push, set `enPassantSq` to the square passed over.
   - En passant capture: pawn moves to the square behind the opponent pawn, removing that pawn.

4. **Castling**:
   - Need to know if kings/rooks have moved. Add booleans `whiteCastleKingside`, `whiteCastleQueenside`, `blackCastleKingside`, `blackCastleQueenside` to `Board`.
   - In `makeMove`, if king or rook moves, update these flags.
   - Castling legality: king not in check, squares between empty and not attacked, king does not move through attacked square, and rook not moved.

5. **Generate all moves**:
   ```cpp
   vector<Move> Board::generateMoves() {
       vector<Move> moves;
       // For each piece of the side to move, call piece-specific generator
       // and add pseudo-legal moves to a temporary list.
       // Then filter: for each pseudo-legal move, use a "makeMove" that doesn't
       // change the actual board but tests if king is left in check.
   }
   ```

6. **Filter out illegal moves**:
   - Create a function `bool moveLeavesKingInCheck(Move m)` that:
     - Makes the move on a copy of the board (or uses undo on a temporary board).
     - Checks if the king of the moving side is attacked.
   - Alternatively, generate only pseudo‑legal moves and rely on `makeMove` (which already checks king safety) – but `makeMove` modifies the board. Better to have a separate `testMove` that works on a copy.

7. **Complexity**: O(pieces × max moves per piece). With bitboard magic, can be very fast; for simplicity, loops are acceptable for now.

#### Pseudocode for pawn moves (white)
```cpp
void Board::addPawnMoves(int from, vector<Move>& moves) {
    int rank = from / 8;
    int file = from % 8;
    // single push
    int to = from + 8;
    if (to < 64 && !getBit(allPieces, to)) {
        if (rank == 6) { // promotion
            for (int prom = 1; prom <= 4; ++prom)
                moves.push_back({from, to, prom});
        } else {
            moves.push_back({from, to, 0});
            // double push from rank 1
            if (rank == 1 && !getBit(allPieces, from + 16))
                moves.push_back({from, from + 16, 0});
        }
    }
    // captures left/right
    for (int df : {-1, 1}) {
        int toFile = file + df;
        if (toFile >= 0 && toFile < 8) {
            int toIdx = from + 8 + df;
            if (toIdx < 64 && getBit(blackPieces, toIdx)) {
                if (rank == 6) { // promotion capture
                    for (int prom = 1; prom <= 4; ++prom)
                        moves.push_back({from, toIdx, prom});
                } else {
                    moves.push_back({from, toIdx, 0});
                }
            }
            // en passant capture
            if (rank == 4 && toIdx == enPassantSq) {
                moves.push_back({from, toIdx, 0, false, true});
            }
        }
    }
}
```

### 3.2 Evaluation Function (Phase 2)

#### Step‑by‑step

1. **Material values** (standard):
   ```cpp
   const int PawnValue = 100;
   const int KnightValue = 320;
   const int BishopValue = 330;
   const int RookValue = 500;
   const int QueenValue = 900;
   ```

2. **Piece‑square tables** (PST):
   - Arrays of size 64 for each piece type, giving a bonus/penalty for being on a square.
   - Use mirroring for black (e.g., index = 63 - sq for black’s PST).
   - Example for white pawns: encourage centre and advancement.

3. **Evaluation function**:
   ```cpp
   int Board::evaluate() {
       int score = 0;
       // Material + PST for white
       score += popcount(WP) * PawnValue + sumPST(WP, whitePawnTable);
       score += popcount(WN) * KnightValue + sumPST(WN, whiteKnightTable);
       // ... other pieces
       // Subtract for black
       score -= popcount(BP) * PawnValue + sumPST(BP, blackPawnTable);
       // ...
       // Return from perspective of side to move? Usually we want positive if white is better.
       // For minimax, we can evaluate from the perspective of the side that just moved,
       // but easier: return score relative to white, and let minimax handle sign.
       return score;
   }
   ```

4. **Complexity**: O(1) (popcount and table lookups). Very fast.

### 3.3 Minimax with Alpha‑Beta (Phase 3)

#### Pseudocode
```cpp
int Board::alphabeta(int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) return evaluate();

    vector<Move> moves = generateMoves();
    if (moves.empty()) {
        // checkmate or stalemate
        if (isInCheck()) return maximizing ? -MATE_SCORE : MATE_SCORE;
        else return 0; // stalemate
    }

    orderMoves(moves); // use simple ordering for now

    if (maximizing) {
        int best = -INF;
        for (Move m : moves) {
            makeMove(m); // modifies board
            int score = alphabeta(depth - 1, alpha, beta, false);
            undoMove(m);
            best = max(best, score);
            alpha = max(alpha, best);
            if (beta <= alpha) break; // prune
        }
        return best;
    } else {
        // similarly for minimizing
    }
}
```

- **Note**: `makeMove` and `undoMove` need to be efficient. `undoMove` can restore bitboards and state flags (castling rights, en passant). Use a stack or copy board.
- **Depth‑limited search**: start with depth 4, increase as engine improves.

### 3.4 Move Ordering (Phase 4)

#### Heuristics
1. **Captures**: Score by MVV‑LVA: `victimValue - attackerValue` (larger victim, smaller attacker). Predefined values: queen=9, rook=5, etc.
2. **Killer moves**: Two slots per depth (or one). If a move causes a beta cutoff and is not a capture, store it.
3. **History heuristic**: Table `history[from][to]` incremented when move causes cutoff.

#### Implementation
```cpp
void Board::scoreMoves(vector<Move>& moves, int depth) {
    for (Move& m : moves) {
        if (isCapture(m)) {
            int victim = pieceTypeAt(m.to);
            int attacker = pieceTypeAt(m.from);
            m.score = victimValue[victim] * 10 - attackerValue[attacker]; // MVV-LVA
        } else if (m == killerMoves[depth][0] || m == killerMoves[depth][1]) {
            m.score = KILLER_BONUS;
        } else {
            m.score = history[m.from][m.to];
        }
    }
    sort(moves.begin(), moves.end(), [](Move a, Move b) { return a.score > b.score; });
}
```

### 3.5 Game Loop (Phase 4)

#### Simple console loop
```cpp
int main() {
    Board board;
    board.resetBoard();
    bool humanWhite = true; // configurable

    while (!gameOver(board)) {
        board.printBoard();
        if ((humanWhite && board.whiteToMove) || (!humanWhite && !board.whiteToMove)) {
            // human move: read from stdin (e.g., "e2e4")
            string input;
            cin >> input;
            Move m = parseMove(input);
            if (board.makeMove(m)) {
                // success
            } else {
                cout << "Illegal move\n";
            }
        } else {
            // AI move
            Move best = board.search(MAX_DEPTH);
            board.makeMove(best);
            cout << "AI plays: " << moveToString(best) << endl;
        }
    }
    return 0;
}
```

---

## Section 4: Code Review & Bug Analysis (QA Engineer View)

### 4.1 Logical Bugs and Risks

1. **Pointer comparison in `moveLegalityCheck`**:
   - `if (piece == &WP)` etc. This works only if the pointer passed is exactly the address of the member variable. If the board is ever copied (e.g., passed by value), the pointers become invalid. **Risk**: copying `Board` would break move validation. **Recommendation**: Use an enum `PieceType` stored in the board or pass the type explicitly.

2. **`getPiecePosition`**:
   - Uses a linear scan (O(64)). Called only for king position, but still inefficient. Could use `std::countr_zero` as commented, but it's not used. Should implement that.

3. **`isMoveObstructed`**:
   - Assumes `from` and `to` are valid for sliding pieces; if called with non‑sliding, it may loop incorrectly (e.g., pawn double push, but it's not used there). However, the function returns false if the step direction is zero? Actually, it computes `moveRank` and `moveFile` from sign of diff, so for knight moves the diff might be 2,1, but the loop steps only in that direction until it reaches `to`, which would be wrong for knights (but it’s only called for sliders). Still, better to assert or not call for non‑sliders.

4. **Pawn double push obstruction**:
   - In `moveLegalityCheck`, for white pawn double push: `!GetBit(whitePieces|blackPieces,to-8)` checks the intermediate square only if from rank==1 and to rank==3. This is correct. But note: `to-8` is the intermediate square. However, the code uses `to-8` assuming the board is little‑endian? It's correct because rank increases with index.

5. **En passant and castling not implemented**:
   - The code explicitly notes they are missing. This is a major gap.

6. **`makeMove` king safety check**:
   - It uses `isSquareAttacked(kingSq, !isWhite)`. This is correct for checking if the king is in check after the move. However, `isSquareAttacked` loops over all pieces each time – may be slow but acceptable for now.

7. **No check for game over**:
   - No detection of checkmate or stalemate. `listOfValidMoves` is empty, so cannot detect.

8. **`getPieceBitboardAtSquare` returns pointer to bitboard**:
   - If the square is empty, returns `nullptr`. In `makeMove`, if `pieceToCapture` is `nullptr`, it correctly skips resetting. However, if the square contains a piece, we store the bitboard pointer. But after making the move, we restore the bitboard in case of illegal move. This is fragile because the pointer could become dangling if the board is later copied.

9. **`makeMove` updates `whiteToMove` only after successful move**:
   - Good.

10. **`abs` function**:
    - Custom `abs` using bit manipulation works for integers but may overflow on `INT_MIN`. Safer to use `std::abs`.

### 4.2 Edge Cases Not Handled

- **Castling**: no generation or validation.
- **En passant**: no generation.
- **Pawn promotion**: moves are not generated with promotion choices; `moveLegalityCheck` doesn't handle promotion at all.
- **Threefold repetition / fifty‑move rule**: not implemented.
- **Stalemate**: not detected.
- **Checkmate**: not detected.
- **Insufficient material**: not considered.

### 4.3 Performance Bottlenecks

1. **`isSquareAttacked`**:
   - Loops over 8 directions, up to 7 steps each, and for each step checks piece types. This is O(56) per call. Called in `makeMove` for king safety and will be called many times during search. Should be optimized.
2. **`moveLegalityCheck` for sliders**:
   - Calls `isMoveObstructed`, which again loops over squares between from and to. This duplicates work done in attack detection.
3. **`getPiecePosition`**:
   - Linear scan for king (worst case 64 steps). Use `std::countr_zero`.
4. **Move generation** (once implemented):
   - Will need to be efficient. Current approach (looping per piece) is acceptable for moderate depth but will become a bottleneck as search depth increases.

### 4.4 Suggested Optimizations

- **Precompute attack tables**:
  - For knights, kings: pre‑computed bitboards of attacks from each square.
  - For pawns: pre‑computed attack masks.
  - For sliding pieces: use magic bitboards or at least pre‑computed ray masks and occupancy lookups.
- **Use `__builtin_popcountll`** or `std::popcount` for counting pieces.
- **Store king position** as a separate variable updated during `makeMove`/`undoMove` to avoid scanning.
- **Use move generation that returns a list of moves, not repeated legality checks**. In search, we will generate moves once per node, then try them. That’s the right pattern.
- **Avoid pointer comparisons** – replace with piece type enum.

---

## Section 5: Performance & Scalability

### 5.1 Optimizing Minimax

| Technique | Description | Benefit |
|-----------|-------------|---------|
| **Alpha‑Beta Pruning** | Already planned; essential. |
| **Move Ordering** | Captures, killers, history – improves pruning. |
| **Transposition Table** | Cache evaluated positions (using Zobrist hashing). Stores depth, score, flag (exact/alpha/beta), best move. Reduces search tree. |
| **Iterative Deepening** | Search depth 1,2,... until time runs out. Provides best move for any time control and fills transposition table for move ordering. |
| **Quiescence Search** | After depth limit, search only captures to avoid horizon effect. |
| **Null Move Pruning** | Let opponent move twice; if score still above beta, prune (risky, requires careful implementation). |
| **Principal Variation Search** | Narrow window search after first move. |

### 5.2 Memory Considerations

- **Board representation**: 12 bitboards + flags (few bytes). Very compact.
- **Transposition table**: Size depends on available memory (e.g., 16–64 MB). Each entry ~16 bytes: hash, depth, score, flag, best move. Table size = number of entries.
- **Move list**: Per node, a vector of moves (max ~218). Acceptable.
- **History table**: 64×64 ints = 16KB.
- **Killer table**: depth × 2 moves (e.g., 100×2 = small).

### 5.3 Future Improvements

- **Opening book**: Pre‑computed lines to avoid searching early game.
- **Endgame tablebases**: For perfect play in endgames (requires large storage).
- **Multi‑threading**: Parallel search (e.g., Lazy SMP).
- **Evaluation tuning**: Use Texel tuning or machine learning to adjust PST and material values.

---

## Section 6: Final Action Plan

### Prioritized Checklist (Start with Phase 1)

- [x] **Phase 0 – Understand current code** (done).
- [ ] **Phase 1 – Complete Move Generation**
  - [ ] Add `enPassantSq`, castling rights to `Board`.
  - [ ] Define `Move` struct.
  - [ ] Implement pseudo‑legal move generation for all pieces (including castling, en passant, promotion).
  - [ ] Implement `generateMoves()` returning `vector<Move>`.
  - [ ] Implement a fast `makeMove`/`undoMove` that updates bitboards and flags.
  - [ ] Implement `moveLeavesKingInCheck` (or reuse `makeMove` with copy).
  - [ ] Test with perft (move count) against known positions.

- [ ] **Phase 2 – Basic Evaluation**
  - [ ] Add material values and piece‑square tables.
  - [ ] Implement `evaluate()` returning relative score.
  - [ ] Test on simple positions.

- [ ] **Phase 3 – Minimax with Alpha‑Beta**
  - [ ] Implement recursive `alphabeta` with depth limit.
  - [ ] Add game‑over detection (checkmate, stalemate).
  - [ ] Test with fixed depth and known tactics.

- [ ] **Phase 4 – Move Ordering & Game Loop**
  - [ ] Implement capture scoring (MVV‑LVA).
  - [ ] Add killer move slots (2 per depth).
  - [ ] Add history table and update on beta cutoffs.
  - [ ] Create simple console game loop (human vs AI).
  - [ ] Add move parsing (e.g., algebraic notation).

- [ ] **Phase 5 – Performance Enhancements**
  - [ ] Implement Zobrist hashing and transposition table.
  - [ ] Add iterative deepening.
  - [ ] Add quiescence search.
  - [ ] Profile and optimize hotspots (attack tables, etc.).

- [ ] **Phase 6 – Polish (Optional)**
  - [ ] Implement UCI protocol.
  - [ ] Tune evaluation using self‑play.
  - [ ] Add opening book.

### Recommendations

- **Start simple**: Do not implement all advanced features at once. Get a working minimax with move generation first.
- **Test early**: Use perft to validate move generation. Debug with known positions.
- **Keep code modular**: Separate board, move generation, search, evaluation into distinct classes/files.
- **Use version control** and commit after each milestone.
- **Avoid premature optimization**: Focus on correctness first, then profile and optimize.

This document provides a complete roadmap and detailed guidance to transform your current board representation into a fully functional Minimax‑based chess engine. Follow the phases sequentially, and you will have a solid engine suitable for a personal project.