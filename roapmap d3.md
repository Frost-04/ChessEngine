## SECTION 1: CURRENT SYSTEM ANALYSIS (Software Architect View)

### Summary of Current Code
The code defines a `Board` class that represents a chess position using 12 separate 64‑bit unsigned integers (bitboards) for each piece type and color. It provides:
- Board reset to the standard starting position.
- Basic bit manipulation (set, reset, test bits).
- Functions to retrieve combined bitboards for white pieces, black pieces, and all pieces.
- A method `getPiecePosition()` that returns the index of the least significant set bit (used only for the king, currently implemented with a loop but commented to use `std::countr_zero` later).
- `getPieceBitboardAtSquare(int i)` returns a pointer to the bitboard that contains a piece on square `i`, or `nullptr` if empty.
- `isSquareAttacked(int sq, bool byWhite)` determines if square `sq` is attacked by the given side. It checks pawn captures, knight jumps, sliding attacks (rook, bishop, queen) with obstruction checks, and the king’s adjacent squares.
- `isMoveObstructed(int from, int to)` checks if any intermediate squares between `from` and `to` are occupied (for sliding pieces).
- `moveLegalityCheck(uint64_t *piece, int from, int to)` validates a move for a given piece bitboard according to basic movement rules (including pawn double‑step but **not** en passant, castling, or promotion). It does **not** verify that the move leaves one’s own king in check.
- `makeMove(uint64_t *piece, int fromIdx, int toIdx)` attempts to execute a move. It first calls `moveLegalityCheck`, then ensures the moving piece belongs to the side whose turn it is, temporarily applies the move, checks whether the king remains safe using `isSquareAttacked`, and either commits the move (flipping `whiteToMove`) or rolls back the changes.
- `listOfValidMoves()` is declared but empty.
- `printBoard()` prints an ASCII representation of the board.
- `main()` simply creates a board, resets it, and prints it.

### Implemented Components
- **Board representation**: 12 bitboards, turn flag.
- **Basic bitboard utilities**: Get/Set/Reset bit, piece bitboard retrieval.
- **Attack detection**: `isSquareAttacked` (pawn, knight, sliding, king).
- **Move validation** (per piece): `moveLegalityCheck` for pawns, knights, bishops, rooks, queens, kings (excluding special moves).
- **Move execution with undo on king exposure**: `makeMove` with simple rollback logic.
- **Board printing**: `printBoard`.

### Missing Components (Exhaustive List)
To build a fully functional Minimax chess engine, the following are absent:
1. **Complete move generation** – generating **all** pseudo‑legal moves for a given side (including castling, en passant, pawn promotion).
2. **Special move handling**:
   - Castling (rights, legality, and execution).
   - En passant (tracking the target square and performing the capture).
   - Pawn promotion (choice of piece, handling in move generation).
3. **Move encoding** – a compact representation (e.g., 16‑bit or 32‑bit) that stores from/to square, promotion piece, and flags for special moves.
4. **Move list management** – storing generated moves for search.
5. **Check/checkmate/stalemate detection** – determining if the current side is in check, and if no legal moves exist (mate/stalemate).
6. **Draw detection** – fifty‑move rule, threefold repetition, insufficient material.
7. **Search algorithm** – Minimax (or Negamax) with alpha‑beta pruning.
8. **Evaluation function** – heuristics to score a position (material, piece‑square tables, mobility, pawn structure, king safety, etc.).
9. **Move ordering** – sorting moves to improve alpha‑beta pruning (captures, killer moves, history heuristic).
10. **Game loop / UCI interface** – accepting moves from a human or external GUI, displaying the board, and invoking the search.
11. **Time management** – controlling search depth/time per move.
12. **Transposition table** – caching previously evaluated positions to avoid redundant search (optional but important for performance).
13. **Opening book / endgame tablebases** (advanced, but can be added later).

### Architecture Diagram (Text)
```
+-------------------+       +---------------------+
|      Board        |       |    Move Generator   | (to be built)
|-------------------|       |---------------------|
| - bitboards       |<----->| - generateMoves()   |
| - turn flag       |       | - pseudo‑legal list |
| + ResetBoard()    |       +---------------------+
| + isSquareAttacked|               |
| + moveLegalityCheck|              v
| + makeMove()      |       +---------------------+
| + getPiece...()   |       |      Move List      | (to be built)
+-------------------+       |---------------------|
        ^                   | - moves[]           |
        |                   | - size/count        |
        |                   +---------------------+
        |                              ^
        |                              |
+-------+-------+              +-------+-------+
|   Search       |              |   Evaluation   |
| (to be built)  |              | (to be built)  |
|----------------|              |----------------|
| - minimax()    |------------->| - evaluate()   |
| - alphaBeta()  |              +----------------+
| - orderMoves() |
+----------------+
        |
        v
+-------------------+
|    Game Loop      | (to be built)
|-------------------|
| - parse input     |
| - make move       |
| - call search     |
| - display board   |
+-------------------+
```
- **Board** provides the current state and low‑level manipulation.
- **Move Generator** queries the Board to produce a list of legal moves.
- **Search** uses the generator to explore the game tree, orders moves, and calls **Evaluation** at leaf nodes.
- **Game Loop** orchestrates user interaction and engine responses.

### Implicit Design Assumptions
- The board is always oriented with white on ranks 0‑3 and black on ranks 4‑7 (standard orientation).
- No support for Chess960 (castling assumes king and rook on original squares).
- `moveLegalityCheck` assumes that the piece bitboard passed in corresponds to the piece type at `from` (the caller ensures this).
- `makeMove` assumes that the move is legal according to `moveLegalityCheck` and only checks king safety afterwards.
- The engine will use a simple turn‑based model with no undo stack (only rollback on illegal king exposure).
- En passant, castling, and promotion are explicitly noted as missing.

### Architectural Weaknesses & Technical Debt
- **No move generation**: The most critical missing piece. `listOfValidMoves` is a stub.
- **Move legality tied to Board methods**: Validation and move execution are intertwined, making it harder to separate concerns and test.
- **No move representation**: Moves are passed as (piece*, from, to) which is fragile and lacks flags for special moves.
- **Inefficient bit scanning**: `getPiecePosition` uses a loop over 64 bits; should use `std::countr_zero` (already commented). Many places use loops where builtins or precomputed tables would be faster.
- **Hard‑coded offsets**: Pawn double‑step ranks (1 and 6) and knight offsets are hard‑coded; future variants would break.
- **Lack of precomputed attack tables**: `isSquareAttacked` loops over directions and multipliers each time; this will be called millions of times during search and is a major bottleneck.
- **No support for undo beyond a single move**: `makeMove` can only roll back the last move; no move stack for search.
- **Missing special move handling**: En passant, castling, and promotion are placeholders – the code will incorrectly reject these moves or behave unpredictably if attempted.
- **No check for stalemate**: `makeMove` only checks if the king is attacked after the move, but does not detect when a side has no legal moves.
- **Code duplication**: `isSquareAttacked` and `moveLegalityCheck` contain similar logic (e.g., pawn capture conditions). Could be refactored.
- **Absence of constants**: Piece types are not enumerated; magic numbers like 64, 8, etc., are scattered.

---

## SECTION 2: ROADMAP & DESIGN (Architect View)

### Phases and Justification
1. **Phase 1 – Core Move Generation**  
   Implement pseudo‑legal move generation for all pieces, including castling, en passant, and promotion.  
   *Why first?* Without moves, there is nothing to search. This phase also requires encoding moves and handling special rules properly.

2. **Phase 2 – Basic Evaluation & Search Stub**  
   Create a simple material‑based evaluation and a minimal Minimax (or Negamax) function to prove the move generator works.  
   *Why second?* Allows early testing of move generation through shallow searches.

3. **Phase 3 – Alpha‑Beta Pruning & Move Ordering**  
   Integrate alpha‑beta pruning and simple move ordering (captures first).  
   *Why third?* Dramatically increases search depth; essential for a playable engine.

4. **Phase 4 – Game Loop & UCI Interface**  
   Build a console interface that accepts moves (e.g., in algebraic notation) and calls the search.  
   *Why fourth?* After search works, you need a way to interact with it.

5. **Phase 5 – Advanced Optimizations**  
   Add transposition tables, iterative deepening, killer heuristics, etc.  
   *Why last?* These are performance enhancements that build on a stable search.

### Module Definitions & Interactions

| Module               | Responsibilities                                                                 | Input                                     | Output                                      |
|----------------------|-----------------------------------------------------------------------------------|--------------------------------------------|---------------------------------------------|
| **Board**            | Maintain bitboards, turn, castling rights, en passant target, move counters. Provide methods to make/unmake moves (with full state save). | Move (encoded)                             | Updated board state (or restored)           |
| **Move Generator**   | Generate all legal moves for the side to move. Filter out moves that leave king in check. | Const Board&, side                        | List of moves (encoded)                     |
| **Evaluation**       | Compute a static score for a position.                                           | Const Board&                              | Score (signed integer, positive favors white) |
| **Search**           | Perform Minimax with alpha‑beta pruning, using move ordering and transposition table. | Board, depth, alpha, beta                  | Best move and its score                      |
| **Move Ordering**    | Sort moves to improve pruning (captures, killer, history).                      | List of moves, search context               | Sorted list                                  |
| **Transposition Table**| Cache previously evaluated positions (hash table).                              | Zobrist hash, depth, score, bound type    | Stored entry or lookup result                |
| **Game Loop**        | Parse user input, update board, invoke search, display board.                   | User commands (UCI or algebraic)           | Move to play                                 |

### Data Flow
- The **Game Loop** reads a command (e.g., “e2e4”) and calls `Board::makeMove` with a decoded move. If the move is legal, it updates the board and then calls `Search::think()`.
- `Search::think()` initializes alpha‑beta search, possibly with iterative deepening. It repeatedly calls the **Move Generator** to obtain moves for the current position.
- Generated moves are passed to **Move Ordering** to obtain a sorted list.
- The search recursively explores moves, updating alpha/beta. At leaf nodes (depth = 0 or terminal), it calls **Evaluation** to get a score.
- When a move is played in the search, it calls `Board::makeMove` (with state saving) and later `Board::unmakeMove`.
- The **Transposition Table** is consulted before generating moves for a node; after evaluation, results are stored.

### Design Choices & Alternatives Rejected
- **Bitboards over Mailbox**: Bitboards enable fast attack and move generation via bitwise operations and are the industry standard for high‑performance engines. The current code already uses them, so we continue. Mailbox (8x8 array) would be simpler to implement but slower for sliding pieces.
- **Precomputed Attack Tables vs. Runtime Calculation**: For sliding pieces, we could compute attacks on the fly (as in `isSquareAttacked`), but that would be too slow during search. We will adopt **magic bitboards** for bishops and rooks, which provide instant attack sets after a few operations. This is more complex but necessary for performance.
- **Move Encoding**: We will use a 16‑bit integer (or 32‑bit) to encode from/to (6 bits each), promotion piece (2 bits), and flags for special moves (castling, en passant, pawn double‑push). This is compact and easy to store in lists. Alternative: using a struct with explicit fields (more readable but larger).
- **Negamax over Minimax**: Negamax simplifies code by using symmetry. We’ll implement Negamax with alpha‑beta.
- **Recursive Search vs. Iterative**: Recursive is simpler to implement and sufficient for depths up to ~10‑12 ply; we’ll use recursion with a stack limit.

### Trade‑offs
- **Magic Bitboards**: High implementation complexity vs. huge speed gain (must‑have for serious engine). We’ll accept the complexity because it’s a personal project aiming for efficiency.
- **Transposition Tables**: Memory usage vs. search speed. We’ll implement a fixed‑size hash table (e.g., 16 MB) that overwrites entries (depth‑preferred replacement).
- **Evaluation Complexity**: A simple material + piece‑square table is easy but weak; we can later add pawn structure and king safety. Starting simple allows early testing.
- **Move Ordering**: Captures first is trivial; killer and history heuristics require storing data per ply/history table. We’ll implement them step by step.

---

## SECTION 3: IMPLEMENTATION GUIDE (Senior Developer View)

For each missing component, I provide step‑by‑step instructions, pseudocode, complexity analysis, and best practices.

### 3.1 Move Generation (Pseudo‑Legal + Legal)
**Goal**: Generate all moves for a given side, then filter those that leave the king in check to obtain legal moves.

**Step‑by‑step**:
1. **Encode a move** – Define a type `Move` (e.g., `uint16_t`):
   - Bits 0‑5: from square (0‑63)
   - Bits 6‑11: to square (0‑63)
   - Bits 12‑13: promotion piece (0 = queen, 1 = rook, 2 = bishop, 3 = knight) or flag for special moves.
   - Bit 14: flag for castling? (Alternatively use bits 12‑15 for flags: double pawn push, en passant, castling).
2. **Precompute attack tables** (optional but recommended):
   - Knight attacks: `uint64_t knightAttacks[64]` using bitwise shifts.
   - King attacks: `uint64_t kingAttacks[64]`.
   - Pawn attacks: two tables (white and black) `uint64_t pawnAttacks[2][64]`.
   - Sliding pieces: implement magic bitboards for bishops and rooks (or use a simpler hyperbola quintessence if performance is less critical). We’ll outline magic bitboards.
3. **Generate pseudo‑legal moves**:
   - Iterate over each piece bitboard of the side to move.
   - For pawns: generate single push, double push (if on starting rank), and captures left/right. Include en passant if a target square exists.
   - For knights: lookup `knightAttacks[from]` and AND with `~ownPieces`.
   - For bishops, rooks, queens: use magic bitboards to get attack mask, then AND with `~ownPieces`, and then for each target, check if the move is allowed (queens combine rook and bishop).
   - For king: lookup `kingAttacks[from]` AND with `~ownPieces`, and also generate castling moves if rights exist and squares are safe.
4. **Filter legal moves**:
   - For each pseudo‑legal move, make it on a temporary board (or use `makeMove` with undo), then check if the king of the moving side is attacked. If not, add to legal move list.

**Pseudocode**:
```cpp
vector<Move> generateMoves(const Board& board, bool white) {
    vector<Move> moves;
    uint64_t ownPieces = white ? board.GetWhitePieces() : board.GetBlackPieces();
    uint64_t oppPieces = white ? board.GetBlackPieces() : board.GetWhitePieces();
    uint64_t allPieces = ownPieces | oppPieces;

    // Pawns
    uint64_t pawns = white ? board.WP : board.BP;
    while (pawns) {
        int from = pop_lsb(pawns);
        // generate pawn moves (pseudo‑legal)...
    }
    // Knights, bishops, rooks, queens, king similarly...

    // Filter
    vector<Move> legal;
    for (Move m : moves) {
        Board temp = board;
        if (temp.makeMove(m)) // makeMove now handles full legality?
            legal.push_back(m);
    }
    return legal;
}
```
**Time Complexity**: O(number of pieces × average moves). With magic bitboards, move generation is very fast (a few hundred operations per position).

**Common Mistakes**:
- Forgetting to exclude moves that leave king in check (especially en passant, which removes the pawn behind the capturing pawn).
- Incorrectly handling castling through attacked squares.
- Not resetting en passant target after a non‑pawn move.
- Using the same bitboard for both white and black pawn attacks without differentiating.

**Debugging Strategy**:
- Use perft (performance test) to compare move counts with known values at shallow depths.
- Write unit tests for each piece’s move generation from specific positions.

### 3.2 Special Moves: Castling, En Passant, Promotion
**Castling**:
- Add castling rights (e.g., `bool castleWK, castleWQ, castleBK, castleBQ` in `Board`).
- In `makeMove`, update rights when king or rook moves.
- In move generation: if rights exist, check that squares between king and rook are empty, not attacked, and king not in check.
- Encode castling as a move with a special flag (e.g., `from` = king square, `to` = rook square, but we need to know which rook). Alternatively, use `to` = king’s destination (g1 for white kingside) and derive rook move.

**En Passant**:
- Store en passant target square (`int enPassantTarget`) – the square behind the pawn that just pushed two steps.
- In pawn move generation, if the pawn is on the fifth rank (for white) and an adjacent enemy pawn just made a double push, allow capture to the target square.
- In `makeMove`, when an en passant capture occurs, remove the captured pawn from the correct square (not the destination).
- Clear `enPassantTarget` after any non‑pawn move.

**Promotion**:
- When a pawn reaches the last rank, the move must specify a promotion piece (usually queen, but allow others). In UCI, promotion piece is indicated by a letter (q, r, b, n).
- Encode promotion piece in the move (bits 12‑13). In `makeMove`, replace the pawn with the chosen piece.

### 3.3 Evaluation Function
**Step‑by‑step**:
1. **Material**:
   - Define piece values: pawn = 100, knight = 320, bishop = 330, rook = 500, queen = 900.
   - Sum values for each side, return (white - black) for white’s perspective.
2. **Piece‑Square Tables**:
   - Create arrays `int mg_pawn_table[64]` etc. (midgame values). Use symmetric tables for white/black.
   - Add the difference of positional bonuses.
3. **Simple mobility** (optional initially): count number of moves each side has, add small bonus.
4. Return the sum, optionally scaled to avoid fractions.

**Pseudocode**:
```cpp
int evaluate(const Board& board) {
    int score = 0;
    // Material
    score += popcount(board.WP) * 100;
    score += popcount(board.WN) * 320;
    // ... other pieces
    score -= popcount(board.BP) * 100;
    // ...

    // Piece‑square tables (white perspective)
    uint64_t wPawns = board.WP;
    while (wPawns) {
        int sq = pop_lsb(wPawns);
        score += pawn_table[sq];
    }
    // similarly for black (using mirrored table)
    // ... other pieces

    return (board.whiteToMove ? score : -score); // return from perspective of side to move? Usually evaluation is from white's perspective, and search uses negamax.
}
```
**Time Complexity**: O(number of pieces) – very fast.

**Common Mistakes**:
- Forgetting to mirror tables for black.
- Using too large values that cause overflow.
- Not handling the perspective correctly in negamax (the sign should be consistent).

### 3.4 Minimax with Alpha‑Beta Pruning
**Step‑by‑step**:
1. Implement **Negamax** framework: `int negamax(Board& board, int depth, int alpha, int beta)`.
2. Base cases:
   - If depth == 0, return `evaluate(board)`.
   - If no legal moves, return checkmate score (e.g., -infinity + ply) or stalemate (0).
3. Generate moves, order them.
4. For each move:
   - Make move, get new board.
   - score = -negamax(newBoard, depth-1, -beta, -alpha);
   - Unmake move.
   - If score >= beta, return beta (prune).
   - If score > alpha, update alpha.
5. Return alpha.

**Pseudocode**:
```cpp
int negamax(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) return evaluate(board);
    vector<Move> moves = generateMoves(board, board.whiteToMove);
    if (moves.empty()) {
        // check if in check -> mate, else stalemate
        return (board.inCheck() ? -MATE_SCORE : 0);
    }
    orderMoves(moves, board); // e.g., captures first
    for (Move m : moves) {
        board.makeMove(m);
        int score = -negamax(board, depth-1, -beta, -alpha);
        board.unmakeMove(m);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}
```
**Time Complexity**: O(b^(d/2)) with perfect ordering, where b is branching factor.

**Common Mistakes**:
- Not handling mate/stalemate correctly (returning 0 for stalemate, not -infinity for losing mate).
- Forgetting to negate the score when switching sides.
- Incorrect alpha‑beta bounds (should be alpha < beta at root).

### 3.5 Move Ordering
**Step‑by‑step**:
1. **Captures**: Score captures by MVV‑LVA (Most Valuable Victim – Least Valuable Aggressor). Create a table `int mvv_lva[6][6]`.
2. **Killer moves**: Store two killer moves per ply (quiet moves that caused a beta cutoff). In `orderMoves`, give a high score to moves matching the killers.
3. **History heuristic**: Maintain a table `int history[64][64]` (or piece/to) that increments when a quiet move causes a beta cutoff. Use this score for remaining quiet moves.
4. Sort moves descending by score.

**Pseudocode**:
```cpp
void orderMoves(vector<Move>& moves, const Board& board, int ply, int killers[][2]) {
    for (Move& m : moves) {
        int score = 0;
        if (isCapture(m)) {
            int victim = pieceTypeAt(board, to(m));
            int attacker = pieceTypeAt(board, from(m));
            score = mvv_lva[victim][attacker] + 10000; // large bonus
        } else {
            if (m == killers[ply][0]) score = 9000;
            else if (m == killers[ply][1]) score = 8000;
            else score = history[from(m)][to(m)];
        }
        m.setScore(score);
    }
    sort(moves.begin(), moves.end(), greaterByScore());
}
```
**Time Complexity**: O(moves log moves) – acceptable.

**Common Mistakes**:
- Updating killers only for quiet moves.
- Not resetting history between searches.
- Over‑counting captures (e.g., capturing a pawn with a queen should be less valuable than capturing a queen with a pawn).

### 3.6 Game Loop (UCI or Simple Console)
**Step‑by‑step**:
1. Parse input: if UCI, implement `uci`, `isready`, `position`, `go` commands. For simple console, accept moves in algebraic (e.g., “e2e4”).
2. Validate the move (check if it exists in legal moves).
3. Apply the move using `board.makeMove`.
4. If it’s the engine’s turn, call search with a time limit (or fixed depth) and play the best move.
5. Repeat.

**Pseudocode** (simplified):
```cpp
int main() {
    Board board;
    board.ResetBoard();
    while (true) {
        board.printBoard();
        if (board.whiteToMove == humanIsWhite) {
            string input;
            cin >> input;
            Move m = parseMove(input);
            if (!board.makeMove(m)) continue;
        } else {
            Move best = search(board, depthLimit);
            board.makeMove(best);
            cout << "Engine plays: " << moveToUCI(best) << endl;
        }
    }
}
```

---

## SECTION 4: CODE REVIEW & BUG ANALYSIS (QA Engineer View)

### Logical Bugs or Risks
1. **`isSquareAttacked` – Pawn Attack Off‑by‑One**  
   For white pawns: `int idx=(rank-1)*8+file;` then check `idx+1` and `idx-1`. This correctly places the pawn on the rank below. However, if the target square is on rank 7 (the back rank), a white pawn cannot attack it because pawns only attack forward. The code correctly guards with `rank-1>=0`. But if the target is on rank 0 (white’s back rank), the condition `rank-1>=0` fails, so no white pawn can attack from below – correct. The same logic holds for black pawns. **No bug here**.

2. **`isSquareAttacked` – Sliding Pieces Ignore Blockers Before King?**  
   The king check is performed *after* the inner multiplier loop, so it correctly checks only the immediate neighbor. However, if a sliding piece attacks the square but there is a blocker between the attacker and the target, the function correctly breaks the loop when it encounters any piece. But what if the king itself is behind a blocker? That case cannot happen because the king is not a sliding piece; we only check the king as an immediate neighbor. So no issue.

3. **`isMoveObstructed` – Non‑sliding Moves**  
   The function is only called for sliding pieces, but if a bug elsewhere calls it for a knight move, the while loop will never terminate because the step direction (e.g., (1,2)) will not align with the target (difference (2,1)). This could cause an infinite loop. **Risk**: Ensure it’s only called for valid sliding moves. Add an assertion or early return.

4. **`makeMove` – King Safety Check After Move**  
   After moving, it calls `isSquareAttacked(kingSq, !isWhite)`. This correctly detects if the king is left in check. However, if the move was a castling move (not yet implemented), this check would not verify that the king did not pass through an attacked square. So when castling is added, the legality check must include that.

5. **`makeMove` – Rollback Logic**  
   If the king is left in check, it restores `*piece` and `*pieceToCapture`. But what if `pieceToCapture` was the same as `piece`? That cannot happen because a piece cannot capture itself. However, if the move is a castling move that involves two pieces (king and rook), the current `makeMove` only handles one piece. So castling will require a different approach.

6. **`getPiecePosition` – Uses Loop Instead of `std::countr_zero`**  
   The loop is inefficient but correct. The comment indicates they plan to change it. **Risk**: If the loop is left, performance will suffer during search.

7. **`moveLegalityCheck` – Pawn Double Push Obstruction**  
   For white: `!GetBit(whitePieces|blackPieces,to-8)`. This checks the square between the pawn and the destination (to-8). But if the pawn is on rank 1 (starting rank) and pushes to rank 3, the intermediate square is rank 2, correct. However, they use `whitePieces|blackPieces` which is the same as `allPieces`. So it’s fine. But they should use `GetAllPieces()` for clarity.

8. **`moveLegalityCheck` – Pawn Capture Check**  
   For white: `if (toRank-fromRank==1 && df==1 && GetBit(blackPieces,to))`. This correctly allows capturing a black piece. However, it does not check if the destination square is empty for a non‑capture move? That’s handled earlier: `if (!df && !GetBit(blackPieces,to) && ...`. For captures, they don’t check emptiness because they require a black piece. Good.

9. **`moveLegalityCheck` – Knight Movement**  
   Uses `(df==2 && dr==1) || (df==1 && dr==2)`. This is correct.

10. **`moveLegalityCheck` – King Movement**  
    Uses `(df<=1 && dr<=1)`. Correct, but does not consider castling.

11. **No check for moving into check for non‑king pieces** – This is handled later in `makeMove` by checking king safety. That’s acceptable.

### Edge Cases
- **Pawn double push blocked by a piece on the intermediate square**: `moveLegalityCheck` correctly checks obstruction.
- **En passant**: Not handled; any attempt will be rejected as illegal.
- **Castling**: Not handled; any attempt will be rejected or cause incorrect behavior.
- **Promotion**: Not handled; a pawn reaching the last rank will have no legal moves according to current code (since `moveLegalityCheck` only allows single/double push and capture, none of which work on last rank).
- **Stalemate**: If a side has no legal moves but is not in check, the game should end in a draw. Currently, `makeMove` would simply return false for any move, and the game would continue with the same player? Actually, if no moves are generated, the engine would not know. Need to detect terminal conditions.
- **Threefold repetition**: Not tracked.
- **Fifty‑move rule**: Not tracked.

### Performance Bottlenecks
- **`isSquareAttacked`**: Called in `makeMove` and will be called heavily during move generation (to filter legal moves). The nested loops with up to 8 directions * 7 steps = 56 iterations per call, plus knight loop. For a position with many pieces, this will be slow. Must be replaced with precomputed attack tables.
- **`getPiecePosition`**: Loop over 64 bits each time the king’s position is needed (multiple times per move generation). Use `std::countr_zero`.
- **`moveLegalityCheck` for sliding pieces** calls `isMoveObstructed`, which loops over squares between from and to. This is also inefficient; better to use magic bitboards to check if the target is in the attack set.
- **`makeMove`** searches for the piece at destination using linear scans over 12 bitboards (`getPieceBitboardAtSquare`). This is O(12) and acceptable, but can be optimized by storing a piece‑type array (mailbox) alongside bitboards for fast lookup.

### Optimizations (Immediate)
- Replace `getPiecePosition` with `std::countr_zero`.
- Precompute knight and king attack tables.
- Store a piece‑type array (e.g., `int board[64]`) for O(1) piece type lookup. Update it in `makeMove`.
- Use magic bitboards for sliding pieces to avoid loops in `isSquareAttacked` and `moveLegalityCheck`.

### Concrete Failure Scenarios
1. **Scenario: White pawn on e2, black pawn on d3, white to move.**  
   White attempts e2‑e4. Current code: `moveLegalityCheck` allows double push if e4 is empty and e3 is empty. It will succeed. But if black had a pawn on e3, it would block. Works.
2. **Scenario: White king on e1, black rook on e8, no pieces in between, white to move.**  
   If white tries to move king to e2, `moveLegalityCheck` allows (king moves one square). `makeMove` will move king, then check if king on e2 is attacked. It is attacked by the rook on e8? `isSquareAttacked` for black rook: from e8 to e2, the ray is vertical; it will check each square e7, e6, e5, e4, e3, e2. Since none are occupied, it will see the rook at e8? Wait, in `isSquareAttacked` for sliding pieces, they only check up to the first occupied square. They start from the target and move outward. For a rook attacking e2 from e8, the squares in between (e3‑e7) are empty, so the loop will eventually reach e8 and see the rook. So it correctly returns true. Good.
3. **Scenario: En passant attempt.**  
   Position: white pawn on e5, black pawn on d5 (just moved d7‑d5). White tries exd6 e.p. Current code: `moveLegalityCheck` for pawn: it will see `df==1 && toRank-fromRank==1` (from e5 to d6) and check if there is a black piece on d6. There is no black piece on d6 (the black pawn is on d5). So it returns false. The move is rejected – correct because en passant is not implemented.
4. **Scenario: Castling attempt.**  
   White tries 0‑0. `moveLegalityCheck` for king: king moves from e1 to g1. `df=2, dr=0` – king movement rule `df<=1 && dr<=1` returns false. So move rejected – correct.

### Suggested Test Cases
**Unit Tests**:
- Starting position: verify that `GetWhitePieces()` has 16 bits set, `GetBlackPieces()` has 16 bits.
- After `ResetBoard`, verify each piece bitboard has correct bits.
- `GetBit` with valid/invalid indices.
- `getPiecePosition` for non‑empty and empty bitboards.
- `isSquareAttacked` on e1 (white king) from various attackers.
- `moveLegalityCheck` for each piece type from various positions.

**Integration Tests** (using perft):
- Perft(1) from start: should be 20 (white moves).
- Perft(2) from start: 400.
- Perft(3) from start: 8902.
- After implementing special moves, test positions like Kiwipete (r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -) to verify castling and en passant counts.

---

## SECTION 5: PERFORMANCE & SCALABILITY

### How to Optimize Minimax
1. **Alpha‑Beta Pruning** – Essential. Already planned.
2. **Move Ordering** – Essential to make alpha‑beta effective. Implement MVV‑LVA captures, killer moves, history heuristic.
3. **Transposition Table** – Stores evaluated positions to avoid re‑searching. Use Zobrist hashing. Replacement scheme: depth‑preferred (replace if new depth >= stored depth).
4. **Iterative Deepening** – Search deeper progressively, using previous best move for move ordering. Allows time management.
5. **Null‑Move Pruning** – If the side to move has a big advantage, skip a move to see if the position is still good; reduces search depth. Risky, but can be added later.
6. **Futility Pruning** – At low depths, if evaluation is far below alpha, prune.
7. **Late Move Reductions** – Reduce depth for moves that appear later in ordering.
8. **Quiescence Search** – Extend search at leaf nodes to avoid horizon effect (only captures and checks). This is a must‑have for tactical accuracy.

### Memory Considerations
- **Transposition Table**: Size chosen based on available RAM. For a personal project, 16‑64 MB is reasonable. Each entry can be 16‑24 bytes (hash, depth, score, move, flags). With 64 MB, you can store ~2‑4 million entries.
- **History Table**: `uint16_t history[64][64]` is 64*64*2 = 8192 bytes – negligible.
- **Killer Moves**: `Move killers[MAX_PLY][2]` – small.
- **Piece‑Square Tables**: `int table[64]` for each piece type – small.
- **Zobrist Keys**: An array of 64*12*2 (square, piece, color) plus castling and en passant – a few KB.

### Future Improvements (Ranked by Impact vs. Difficulty)

| Improvement               | Impact | Implementation Difficulty | Must‑have / Nice‑to‑have |
|---------------------------|--------|---------------------------|---------------------------|
| Alpha‑Beta Pruning        | High   | Medium                    | Must‑have                 |
| Move Ordering (captures)  | High   | Easy                      | Must‑have                 |
| Quiescence Search         | High   | Medium                    | Must‑have                 |
| Transposition Table       | High   | Hard                      | Nice‑to‑have (but highly recommended) |
| Iterative Deepening       | Medium | Easy                      | Nice‑to‑have              |
| Killer / History Heuristics| Medium| Medium                    | Nice‑to‑have              |
| Magic Bitboards           | High   | Hard                      | Must‑have (for speed)     |
| Null‑Move Pruning         | Medium | Medium                    | Nice‑to‑have              |
| Late Move Reductions      | Medium | Hard                      | Nice‑to‑have              |
| Opening Book              | Low    | Easy                      | Optional                  |
| Endgame Tablebases        | Low    | Very Hard                 | Optional                  |

**Explanation**:
- Magic bitboards are rated “must‑have” because they are fundamental to fast move generation and attack checking. Without them, the engine will be too slow for any reasonable search depth.
- Alpha‑beta and move ordering are also essential for a playable engine.
- Quiescence search prevents horizon effect and is crucial for tactical strength.
- Transposition tables provide a huge boost but require careful implementation (hash collisions, replacement). They are still highly recommended.

---

## SECTION 6: FINAL ACTION PLAN

### Prioritized Checklist (Next Steps)

1. **[Medium] Refactor current Board class**  
   - Add `std::countr_zero` for `getPiecePosition`.  
   - Introduce piece‑type array `int mailbox[64]` for O(1) piece lookup.  
   - Add castling rights (`bool castleWK, castleWQ, castleBK, castleBQ`) and en passant target (`int epTarget`).  
   - Implement Zobrist hashing (if planning transposition tables later).  

2. **[Hard] Implement magic bitboards for sliding pieces**  
   - Precompute rook and bishop attack tables using magic numbers.  
   - Integrate into `isSquareAttacked` and move generation.  

3. **[Medium] Complete move generation**  
   - Create `Move` encoding (16‑bit).  
   - Implement `generatePseudoLegalMoves()` for all pieces, including castling, en passant, promotion.  
   - Implement `generateLegalMoves()` by filtering with king safety check (using `makeMove` with undo).  

4. **[Easy] Basic evaluation**  
   - Material + piece‑square tables.  

5. **[Medium] Negamax with alpha‑beta**  
   - Write `negamax()` with depth limit, terminal node detection (checkmate/stalemate).  

6. **[Medium] Move ordering**  
   - Implement MVV‑LVA for captures.  
   - Add killer moves and history heuristic.  

7. **[Easy] Simple game loop**  
   - Accept moves in algebraic notation (e.g., “e2e4”).  
   - Integrate search to play a move.  

8. **[Hard] Transposition table & iterative deepening**  
   - Add Zobrist hashing (if not already).  
   - Implement hash table with replacement.  
   - Modify search to use iterative deepening and time management.  

9. **[Medium] Quiescence search**  
   - Extend search at depth 0 with captures.  

10. **[Optional] UCI protocol**  
    - Implement UCI commands for compatibility with GUIs.

### Critical Path Items
- **Magic bitboards** and **move generation** are the foundation – without them, nothing else works.  
- **Negamax with alpha‑beta** and **evaluation** are next to get a working (though weak) engine.  
- **Move ordering** should be added soon after to improve depth.

### Implementation Difficulty Estimates
- **Easy**: Basic evaluation, game loop, iterative deepening (once search exists).  
- **Medium**: Move generation, move ordering, quiescence search, alpha‑beta.  
- **Hard**: Magic bitboards, transposition table, UCI protocol (due to parsing complexity).

Start with **magic bitboards** and **move generation** – this will take the most effort but is the most rewarding. Once you have a move generator, you can test it with perft and then gradually build the search on top.