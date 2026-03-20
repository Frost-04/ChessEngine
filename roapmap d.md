# Technical Design Document: Minimax Chess Engine

## 1. CURRENT SYSTEM ANALYSIS

### 1.1 Overview
The current codebase implements a chess board representation using 12 separate bitboards (one per piece type and color). It provides basic board manipulation, attack detection, single-move validation, and move execution with rollback on king safety violation. The code is written in C++ and uses 64-bit integers to represent squares.

### 1.2 Implemented Components

| Module | Responsibility | Status |
|--------|----------------|--------|
| **Board Representation** | Stores piece positions via bitboards (`WP`, `WN`, ..., `BK`) and whose turn it is (`whiteToMove`). | Complete |
| **Bit Manipulation** | `GetBit`, `SetBit`, `ResetBit` – low-level bitboard operations. | Complete |
| **Piece Aggregation** | `GetWhitePieces`, `GetBlackPieces`, `GetAllPieces` – combined bitboards. | Complete |
| **Square Inquiry** | `getPieceBitboardAtSquare` – returns pointer to the bitboard containing a given square. | Complete |
| **Attack Detection** | `isSquareAttacked` – checks if a square is attacked by a given color (pawn, knight, sliding pieces, king). | Complete |
| **Move Obstruction** | `isMoveObstructed` – checks if any piece lies between two squares (used for sliding pieces). | Complete |
| **Move Legality (Pseudo‑legal)** | `moveLegalityCheck` – validates piece movement rules (pawn, knight, bishop, rook, queen, king) without considering king safety. | Partial (missing en passant, castling, promotion) |
| **Move Execution** | `makeMove` – performs a move, checks for leaving own king in check, reverts if unsafe, and updates turn. | Complete (except special moves) |
| **Board Display** | `printBoard` – outputs ASCII representation of the board. | Complete |
| **Move Listing** | `listOfValidMoves` – placeholder (empty). | Missing |

### 1.3 Missing Components for a Minimax Engine
- **Full move generation** (including en passant, castling, pawn promotion)
- **Checkmate / stalemate detection**
- **Position evaluation** (material, piece‑square tables, mobility, etc.)
- **Search algorithm** (Minimax with alpha‑beta pruning)
- **Move ordering** and other search enhancements
- **Game state tracking** (halfmove clock, fullmove number, castling rights, en passant target square)

### 1.4 Architecture Diagram (Textual)

```
+-------------------+       +---------------------+
|    Board          |       |    MoveGenerator    |  (to be implemented)
|-------------------|       |---------------------|
| - bitboards       |<------| + generateMoves()   |
| - whiteToMove     |       | - generatePawnMoves |
| + makeMove()      |       | - generateKnight... |
| + isSquareAttacked|       +---------------------+
| + ...             |                 |
+-------------------+                 | uses
        ^                             |
        | calls                       v
        |                     +-------------------+
        |                     |    Move           |  (struct)
        |                     |-------------------|
        |                     | - from, to        |
        |                     | - piece           |
        |                     | - special flags   |
        |                     +-------------------+
        |
+-------------------+       +---------------------+
|    Evaluation     |<------|      Search         |
|-------------------|       |---------------------|
| + evaluate()      |       | + minimax()         |
| - material        |       | + alphaBeta()       |
| - piece-square    |       | - quiescence()      |
| - ...             |       | - iterativeDeepening|
+-------------------+       +---------------------+
```

## 2. ROADMAP & DESIGN

### 2.1 Development Phases

#### Phase 1: Complete Move Generation
- Implement pseudo‑legal move generation for all pieces, including:
  - Pawn pushes, double pushes, captures, en passant, promotion.
  - Knight moves.
  - Sliding moves (bishop, rook, queen) using bitboard techniques.
  - King moves (including castling).
- Store castling rights and en passant target square in the board class.
- Generate moves into a container (e.g., `std::vector<Move>`).

#### Phase 2: Game State & Legal Move Filtering
- Add checkmate/stalemate detection (if no legal moves, determine outcome).
- Ensure that `makeMove` updates castling rights and en passant state.
- Provide a method to generate only strictly legal moves (pseudo‑legal moves filtered by king safety).

#### Phase 3: Evaluation Function
- Implement a simple evaluation based on material count and piece‑square tables.
- Add basic positional terms (pawn structure, king safety, mobility) as needed.

#### Phase 4: Search Algorithm
- Implement Minimax with alpha‑beta pruning.
- Add move ordering (captures first, then killer moves, history heuristic).
- Incorporate iterative deepening and a simple transposition table (optional but recommended).

### 2.2 Module Interactions

- **Search** calls `MoveGenerator` to obtain a list of legal moves for the current board.
- For each move, it calls `Board::makeMove` to apply the move and obtain a new board.
- It then calls `Evaluation::evaluate` on the resulting board (or on leaf nodes).
- The search returns the best move and its score back to the caller (e.g., the UCI interface or main loop).

### 2.3 Data Flow

```
[Start Position] 
       |
       v
[Board] --> [MoveGenerator] --> list of Move objects
       |
       v
[Search] (for each move)
       |-- apply move via Board::makeMove
       |-- if terminal node: evaluate via Evaluation::evaluate
       |-- else recurse
       |-- undo move (or use copy of board)
       v
[Best Move] --> output
```

## 3. IMPLEMENTATION GUIDE

### 3.1 Move Generation

#### 3.1.1 Representing Moves
Define a struct `Move` (or use a 32‑bit integer) that encodes:
- From square (6 bits)
- To square (6 bits)
- Moving piece (4 bits – type + color)
- Captured piece (4 bits, or special flag for en passant)
- Promotion piece (2‑3 bits, if any)
- Special flags (castling, en passant, promotion)

**Pseudocode:**
```cpp
struct Move {
    int from;
    int to;
    int piece;      // e.g., 0=WP,1=WN,... 6=BP,... (or use enum)
    int captured;   // piece type or 0 if none
    int promote;    // 0 if no promotion, else piece type
    bool isCastle;
    bool isEnPassant;
    // possibly store score for ordering
};
```

#### 3.1.2 Generating Pseudo‑legal Moves

##### Pawn Moves
- For each pawn bitboard of the side to move:
  - Single push: target square = current + 8 (white) or -8 (black) if empty.
  - Double push: only if on starting rank and both intermediate and target squares empty.
  - Captures: diagonally forward if opponent piece present.
  - En passant: if an en passant target square exists, check if pawn is on the correct rank and adjacent file.
  - Promotion: if pawn reaches last rank, generate moves for each promotion piece (Q, R, B, N).

Use bitboard operations to find set bits (e.g., `while (pawns) { int from = pop_lsb(pawns); ... }`).

##### Knight Moves
Precompute knight attack tables for each square (64 × 8 bits). For each knight, iterate through target squares from the precomputed list and check if target is empty or contains opponent piece.

##### Sliding Moves (Bishop, Rook, Queen)
Simplest approach: loop over the four/eight ray directions and accumulate squares until a blocker is encountered. For each direction, step one square at a time, add move if square is empty or opponent, and stop if occupied.

**Optimisation:** Use magic bitboards for faster generation, but start with simple loops.

##### King Moves
Similar to knight: precompute king attacks for each square. For castling:
- Check if king and rook haven’t moved, squares between are empty, and king does not pass through check.
- Add king moves to the two castling destinations.

**Pseudocode for pawn move generation (white):**
```cpp
void generatePawnMoves(Board& board, int from, std::vector<Move>& moves) {
    int to = from + 8; // single push
    if (to < 64 && !board.getPieceAt(to)) {
        if (to >= 56) { // promotion rank
            for (int prom = WQ; prom <= WN; ++prom) // or loop over queen, rook, bishop, knight
                moves.push_back(Move(from, to, WP, 0, prom));
        } else {
            moves.push_back(Move(from, to, WP));
            // double push
            if (from >= 8 && from < 16) { // second rank
                int to2 = from + 16;
                if (!board.getPieceAt(from+8) && !board.getPieceAt(to2))
                    moves.push_back(Move(from, to2, WP));
            }
        }
    }
    // captures
    for (int dx : {-1, 1}) {
        int to = from + 8 + dx;
        if (to >= 0 && to < 64 && (to%8) == (from%8)+dx) {
            if (board.isPieceAt(to, Black)) // opponent piece
                moves.push_back(Move(from, to, WP, board.getPieceType(to)));
        }
    }
    // en passant (if enPassantSq is set)
    if (board.enPassantSq != -1) {
        // similar capture logic but target is enPassantSq and captured pawn is behind
    }
}
```

### 3.2 Special Moves Handling

#### En Passant
- Store `enPassantSq` in `Board`, updated after a double pawn push.
- In `makeMove`, when an en passant capture occurs, remove the captured pawn from the appropriate bitboard (the one behind the destination).

#### Castling
- Store castling rights as four booleans (WK, WQ, BK, BQ) or a bitmask.
- In `makeMove`, update rights when king or rook moves.
- In move generation, verify that the squares between are empty and not attacked.

#### Promotion
- During move generation, for pawn moves to the last rank, generate four separate moves with promotion piece encoded.
- In `makeMove`, replace the pawn with the chosen piece on the destination square.

### 3.3 Checkmate / Stalemate Detection
After generating all legal moves (or pseudo‑legal filtered by king safety):
- If list is empty:
  - If the side to move is in check → checkmate.
  - Else → stalemate.

### 3.4 Evaluation Function
A simple starting evaluation:
```cpp
int evaluate(const Board& board) {
    int score = 0;
    // Material
    score += popcount(WP) * 100 - popcount(BP) * 100;
    score += popcount(WN) * 320 - popcount(BN) * 320;
    score += popcount(WB) * 330 - popcount(BB) * 330;
    score += popcount(WR) * 500 - popcount(BR) * 500;
    score += popcount(WQ) * 900 - popcount(BQ) * 900;
    // Piece-square tables (precomputed arrays)
    for each piece, add value from table based on square.
    // Return relative to side to move? Usually positive means white advantage.
    return (board.whiteToMove ? score : -score);
}
```
Piece‑square tables can be taken from known sources (e.g., PST for midgame).

### 3.5 Minimax with Alpha‑Beta

**Pseudocode:**
```cpp
int alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) return quiescence(board, alpha, beta); // optional
    auto moves = generateLegalMoves(board);
    if (moves.empty()) {
        if (isInCheck(board)) return (maximizing ? -MATE_VALUE : MATE_VALUE);
        else return 0; // stalemate
    }
    orderMoves(moves); // captures first, etc.
    for (auto& move : moves) {
        board.makeMove(move);
        int score = alphaBeta(board, depth-1, alpha, beta, !maximizing);
        board.undoMove(move);
        if (maximizing) {
            if (score > alpha) alpha = score;
            if (alpha >= beta) break; // beta cutoff
        } else {
            if (score < beta) beta = score;
            if (beta <= alpha) break; // alpha cutoff
        }
    }
    return maximizing ? alpha : beta;
}
```
- `quiescence` searches only captures to avoid horizon effect.
- Use iterative deepening to get a move under time control.

## 4. CODE REVIEW & BUG ANALYSIS

### 4.1 Logical Bugs / Risks

1. **Pawn move legality** – In `moveLegalityCheck`, the double push condition for white uses `!GetBit(blackPieces,to)` to check destination emptiness, but it should check all pieces. However, earlier in the function there is a check `if(isWhite && GetBit(whitePieces,to)) return false;` so a white piece at `to` would already be rejected. Therefore, the destination emptiness is effectively ensured (no white piece and no black piece). This works but is indirect; it’s safer to explicitly check `!GetBit(GetAllPieces(), to)`.

2. **Pawn capture condition** – For white pawn capture, it checks `GetBit(blackPieces,to)`. This is correct, but it does not handle en passant (missing).

3. **King safety after move** – `makeMove` finds the king square using `getPiecePosition(WK)` or `BK`. If the moving side’s king is not present (should never happen), it would return -1, causing undefined behavior. Ensure that the king always exists.

4. **`isMoveObstructed`** – It assumes `from` and `to` are aligned correctly. If called with a non‑sliding move, the loop might not terminate (e.g., knight move). However, it is only called after alignment checks in `moveLegalityCheck`, so safe.

5. **`isSquareAttacked`** – The sliding piece check includes both rook and queen in directions 0‑3, and bishop and queen in 4‑7. However, the loop checks `if(GetBit(rooks, idx) || GetBit(queens, idx))` for horizontal/vertical, and `if(GetBit(bishops, idx) || GetBit(queens, idx))` for diagonals. This is correct. But note that the king check is inside the same direction loop, so it will be executed for each direction. That’s fine.

6. **`getPiecePosition`** – Currently loops over 64 bits; use `std::countr_zero(piece)` for efficiency (C++20). The comment indicates intention to change.

7. **Missing castling rights and en passant state** – Not stored, so cannot be used.

8. **`abs` function** – Custom implementation; could just use `std::abs` from `<cstdlib>`. The current implementation works for 32‑bit ints but may have undefined behavior for `INT_MIN`. Not a major issue here.

### 4.2 Edge Cases

- **Move when king is in check** – `makeMove` correctly rejects moves that leave the king in check, but does it allow moves that block check or capture the checking piece? Yes, because it only checks the final king position.
- **Discovered checks** – Handled automatically because after the move, the king’s square is reevaluated.
- **En passant** – Not implemented, so illegal en passant captures are not generated, and the special capture rule is missing.
- **Castling through check** – Not implemented.
- **Promotion** – Not implemented; pawns reaching last rank will be unable to move, which is incorrect.
- **Stalemate** – Not detected because no move generation.

### 4.3 Performance Bottlenecks

- **`isSquareAttacked`** – Uses nested loops and repeated `GetBit` calls. For move generation, this function will be called many times (especially for king safety checks). Consider precomputing attack tables for knights, kings, and pawns. For sliding pieces, use magic bitboards or at least ray tables.
- **`moveLegalityCheck`** – Called for every attempted move; it computes file/rank differences and calls `isMoveObstructed` for sliding pieces. `isMoveObstructed` loops over intermediate squares, which is O(distance). This is acceptable for a simple engine, but can be optimized.
- **`getPiecePosition`** – Linear scan over 64 bits is inefficient; use `__builtin_ctzll` or `std::countr_zero`.

### 4.4 Suggested Optimizations

- Replace `getPiecePosition` loop with `std::countr_zero(piece)`.
- Precompute knight, king, and pawn attack bitboards for all squares.
- For sliding pieces, either:
  - Use magic bitboards (more complex but very fast), or
  - Precompute ray directions and use `_pext` if available, or
  - Keep the current loop but early exit.
- Use move ordering in search to improve alpha‑beta pruning.
- Store the board state in a structure that allows cheap copying or incremental undo.

## 5. PERFORMANCE & SCALABILITY

### 5.1 Memory Considerations
- Bitboards: 12 × 8 bytes = 96 bytes. Negligible.
- Move lists: per search node, a vector of up to ~200 moves. With iterative deepening, memory usage stays small.
- Transposition table: if implemented, can use several MB, but optional.
- Overall, memory is not a constraint.

### 5.2 Optimizations in Code
- Use bit intrinsics (`__builtin_ctzll`, `_pext`, etc.) for speed.
- Precompute attack tables for non‑sliding pieces.
- For sliding pieces, consider magic bitboards after the engine is functional.
- Implement move generation using bitboard iteration (`while (pieces) { int sq = pop_lsb(pieces); ... }`).
- In search, use iterative deepening and order moves (MVV‑LVA, killer, history).
- Use a simple quiescence search to avoid horizon effect.

## 6. FINAL ACTION PLAN

### Prioritized Checklist

- [ ] **1. Fix existing bugs**
  - Replace `getPiecePosition` loop with `std::countr_zero`.
  - (Optional) Refine pawn emptiness checks to use `GetAllPieces()` for clarity.
- [ ] **2. Add missing game state**
  - Store castling rights (4 booleans or bitmask).
  - Store en passant target square (int, -1 if none).
  - Store halfmove clock (for 50‑move rule) and fullmove number (optional).
- [ ] **3. Implement pseudo‑legal move generation**
  - Create `Move` struct.
  - Write functions: `generatePawnMoves`, `generateKnightMoves`, `generateSlidingMoves`, `generateKingMoves`.
  - Include en passant, castling, and promotion.
  - Fill `listOfValidMoves` to return all pseudo‑legal moves.
- [ ] **4. Implement legal move filtering**
  - After generating pseudo‑legal moves, filter those that leave king in check (using `makeMove` or a dedicated `wouldBeInCheck` function).
  - Provide `generateLegalMoves()`.
- [ ] **5. Add checkmate/stalemate detection**
  - In search, if no legal moves, evaluate as mate or draw.
- [ ] **6. Implement basic evaluation**
  - Material + piece‑square tables.
  - Return score relative to side to move.
- [ ] **7. Implement Minimax with alpha‑beta**
  - Start with fixed depth, then add iterative deepening.
  - Add move ordering.
- [ ] **8. Testing**
  - Test with known positions (perft tests for move generation).
  - Verify evaluation and search with simple endgames.
- [ ] **9. (Optional) Enhancements**
  - Quiescence search.
  - Transposition table.
  - Null‑move pruning, etc.

Follow this plan step by step, testing each component thoroughly before moving to the next.