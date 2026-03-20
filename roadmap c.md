# Technical Review & Implementation Plan for Your Minimax Chess Engine

This document analyzes your current C++ chess code from three perspectives: **software architect**, **senior developer**, and **QA engineer**.  
The focus is practical: **turn the current board/move-validation prototype into a working, efficient Minimax-based chess engine** without overengineering.

***

# SECTION 1: CURRENT SYSTEM ANALYSIS (Software Architect View)

## 1.1 What the Current Code Does

Your current code implements a **bitboard-based chess board model** with:

*   **12 bitboards**: one per piece type and color
*   Initial board setup via `ResetBoard()`
*   Basic board queries:
    *   white occupancy
    *   black occupancy
    *   all occupancy
*   Bit operations:
    *   set bit
    *   clear bit
    *   test bit
*   Piece lookup:
    *   find the bitboard occupying a given square
    *   find the square of a king
*   **Attack detection** via `isSquareAttacked()`
*   **Pseudo-legal move validation** via `moveLegalityCheck()`
*   **Legal move execution** via `makeMove()` by rejecting moves that leave own king in check
*   Console visualization with `printBoard()`

In short, the code is currently a **board state container with partial legal move execution logic**, but **not yet a full chess engine**.

***

## 1.2 Implemented Components

### A. Board Representation

Implemented using **bitboards**:

```cpp
uint64_t WP, WN, WB, WR, WQ, WK;
uint64_t BP, BN, BB, BR, BQ, BK;
```

This is a strong choice for a chess engine because:

*   compact memory usage
*   efficient occupancy checks
*   natural fit for move generation/search optimization later

### B. Turn State

Implemented:

```cpp
bool whiteToMove;
```

### C. Board Initialization

Implemented in `ResetBoard()`.

### D. Occupancy Helpers

Implemented:

*   `GetWhitePieces()`
*   `GetBlackPieces()`
*   `GetAllPieces()`

### E. Square Bit Utilities

Implemented:

*   `GetBit()`
*   `SetBit()`
*   `ResetBit()`

### F. Piece Lookup

Implemented:

*   `getPiecePosition()` – finds least significant piece square
*   `getPieceBitboardAtSquare()` – finds which bitboard owns a square

### G. Attack Detection

Implemented:

*   pawn attacks
*   knight attacks
*   rook/queen rays
*   bishop/queen rays
*   king adjacency

This is one of the most important building blocks for legal move checking.

### H. Piece Movement Validation

Implemented in `moveLegalityCheck()` for:

*   pawns (single, double, capture)
*   knights
*   bishops
*   rooks
*   queens
*   kings (non-castling only)

### I. Legal Move Application

Implemented in `makeMove()`:

*   validates piece move
*   validates side to move
*   applies capture
*   applies move
*   rejects move if own king is attacked afterward

This makes the move **fully legal with respect to king safety**, except for missing special rules.

### J. Board Display

Implemented in `printBoard()`.

***

## 1.3 Missing Components Required for a Minimax Chess Engine

To become a real Minimax chess engine, the following components are still missing.

***

### A. Full Legal Move Generator

Currently missing:

*   `listOfValidMoves()` is empty
*   no move list returned for search
*   no move struct

This is the **highest-priority missing feature**.

Without legal move generation, you cannot run Minimax.

***

### B. Search Algorithm (Minimax / Alpha-Beta)

Missing:

*   recursive game tree search
*   maximizing/minimizing logic
*   alpha-beta pruning
*   depth control
*   best move selection

***

### C. Evaluation Function

Missing:

*   piece values
*   position scoring
*   basic endgame handling
*   mobility / king safety (optional/simple)

Without evaluation, Minimax cannot score non-terminal positions.

***

### D. Terminal State Detection

Missing:

*   check detection helper
*   checkmate detection
*   stalemate detection
*   draw logic (at minimum basic stalemate)

***

### E. Special Chess Rules

Missing:

*   **castling**
*   **en passant**
*   **promotion**

These are required for completeness.

***

### F. Game State Tracking

You need additional state beyond piece positions:

*   castling rights
*   en passant target square
*   halfmove clock
*   fullmove number
*   side to move already exists

Without these, special moves and some draw rules cannot be implemented correctly.

***

### G. Move Undo / Unmake

Minimax requires repeated move simulation.  
Current `makeMove()` modifies board state directly and only rolls back internally for illegal self-check.

You need:

*   a `Move` structure
*   an `UndoState` / move history
*   `makeMove(move)`
*   `unmakeMove(move, state)`

This is essential for efficient search.

***

### H. Engine Interface

Missing:

*   method to ask engine for best move
*   optional algebraic/UCI-style parsing/formatting

Not required for core engine logic, but useful.

***

### I. Testing Infrastructure

Missing:

*   unit tests for move legality
*   perft testing
*   special-rule tests
*   tactical search verification

For chess engines, **perft** is the single most useful correctness test.

***

## 1.4 Current Architecture Diagram (Text Form)

## Current Code Architecture

```text
+--------------------------------------------------+
|                     Board                        |
+--------------------------------------------------+
| Data:                                            |
| - WP, WN, WB, WR, WQ, WK                         |
| - BP, BN, BB, BR, BQ, BK                         |
| - whiteToMove                                    |
+--------------------------------------------------+
| Responsibilities:                                |
| - Initialize board                               |
| - Query occupancy                                |
| - Read/write bits                                |
| - Determine piece at square                      |
| - Detect attacks                                 |
| - Validate piece moves                           |
| - Make legal move (king-safe only)               |
| - Print board                                    |
+--------------------------------------------------+

Main
 └── creates Board
     ├── ResetBoard()
     └── printBoard()
```

***

## Target Architecture for a Complete Minimax Engine

```text
+------------------------------------------------------+
|                    Chess Engine                      |
+------------------------------------------------------+
| 1. Board / Position                                  |
|    - Bitboards                                       |
|    - Side to move                                    |
|    - Castling rights                                 |
|    - En passant square                               |
|    - Halfmove / fullmove                             |
|                                                      |
| 2. Move Representation                               |
|    - from, to, piece, captured, promotion, flags     |
|                                                      |
| 3. Move Generator                                    |
|    - Generate pseudo-legal moves                     |
|    - Filter to legal moves                           |
|    - Generate captures / quiets                      |
|                                                      |
| 4. Move Execution                                    |
|    - makeMove(move)                                  |
|    - unmakeMove(move, undoState)                     |
|                                                      |
| 5. Rules / State Validation                          |
|    - isSquareAttacked()                              |
|    - inCheck()                                       |
|    - checkmate/stalemate                             |
|    - castling / en passant / promotion               |
|                                                      |
| 6. Evaluation                                        |
|    - material                                        |
|    - simple piece-square tables                      |
|    - optional mobility                               |
|                                                      |
| 7. Search                                            |
|    - minimax                                         |
|    - alpha-beta pruning                              |
|    - move ordering                                   |
|                                                      |
| 8. Testing                                           |
|    - unit tests                                      |
|    - perft                                           |
+------------------------------------------------------+
```

***

# SECTION 2: ROADMAP & DESIGN (Architect View)

## 2.1 Recommended Development Phases

The project should be completed in the following practical phases.

***

## Phase 1 — Stabilize Core Position Representation

Goal: make the current board state suitable for search.

### Tasks

*   Add a `Move` struct
*   Add castling rights
*   Add en passant square
*   Add halfmove/fullmove counters
*   Refactor `makeMove()` into:
    *   `makeMove(Move move, UndoState& undo)`
    *   `unmakeMove(Move move, const UndoState& undo)`

### Why first?

Because every other subsystem (move generation, evaluation, search) depends on reliable state transitions.

***

## Phase 2 — Implement Legal Move Generation

Goal: generate **all legal moves** for side to move.

### Tasks

*   Generate pawn moves
*   Generate knight moves
*   Generate bishop/rook/queen sliding moves
*   Generate king moves
*   Add promotion moves
*   Add en passant moves
*   Add castling moves
*   Filter pseudo-legal moves to legal moves

This phase is the engine’s foundation.

***

## Phase 3 — Add Terminal Detection

Goal: determine whether a position is:

*   normal
*   check
*   checkmate
*   stalemate

Optional later:

*   fifty-move rule
*   repetition

***

## Phase 4 — Add Evaluation Function

Goal: score positions for search.

### Start simple

*   material values
*   piece-square tables
*   checkmate/stalemate terminal scores

Optional later:

*   mobility
*   king safety
*   pawn structure

***

## Phase 5 — Implement Minimax + Alpha-Beta

Goal: search game tree and return best move.

### Tasks

*   recursive search
*   alpha-beta pruning
*   fixed depth
*   best move return
*   mate score handling

***

## Phase 6 — Performance Improvements

Goal: make search practical.

### Tasks

*   move ordering
*   faster piece iteration
*   avoid unnecessary scanning
*   optional transposition table
*   optional iterative deepening

***

## Phase 7 — Testing & Validation

Goal: correctness and stability.

### Tasks

*   perft test positions
*   rule-specific unit tests
*   self-play sanity tests
*   tactical positions

***

## 2.2 Required Modules

Below is the recommended module breakdown.

***

### 1. `Position` / `Board`

**Responsibility**

*   store bitboards
*   occupancy
*   side to move
*   castling rights
*   en passant square
*   counters

**Owns**

*   all game state needed to reconstruct a position

***

### 2. `Move`

**Responsibility**

*   represent a chess move compactly

Recommended fields:

*   `from`
*   `to`
*   `movingPiece`
*   `capturedPiece`
*   `promotionPiece`
*   `flags`

Flags example:

*   capture
*   double pawn push
*   en passant
*   castling
*   promotion

***

### 3. `UndoState`

**Responsibility**

*   store reversible information for unmake:
    *   captured piece
    *   old castling rights
    *   old en passant square
    *   old halfmove clock
    *   old side to move

***

### 4. `MoveGenerator`

**Responsibility**

*   generate pseudo-legal moves
*   generate legal moves
*   optionally generate captures only

***

### 5. `Rules`

**Responsibility**

*   `isSquareAttacked()`
*   `inCheck(side)`
*   validate castling rules
*   terminal status

***

### 6. `Evaluator`

**Responsibility**

*   score a position

***

### 7. `Search`

**Responsibility**

*   minimax / alpha-beta recursion
*   best move selection
*   mate/draw score propagation

***

### 8. `Tests`

**Responsibility**

*   perft
*   move generation correctness
*   edge-case coverage

***

## 2.3 Module Interaction

```text
Position
 ├── used by MoveGenerator
 ├── used by Rules
 ├── used by Evaluator
 └── modified by Search through make/unmake

MoveGenerator
 ├── queries Position
 ├── uses Rules::isSquareAttacked / inCheck
 └── returns vector<Move>

Search
 ├── asks MoveGenerator for legal moves
 ├── uses Position::makeMove()
 ├── uses Position::unmakeMove()
 ├── uses Evaluator at leaf nodes
 └── returns best score / best move

Evaluator
 └── reads Position only

Tests
 ├── validate MoveGenerator
 ├── validate Rules
 ├── validate make/unmake
 └── validate Search on known positions
```

***

## 2.4 Data Flow Between Components

### Search Cycle Data Flow

```text
Current Position
   ↓
Generate Legal Moves
   ↓
For each move:
   ↓
makeMove(move, undo)
   ↓
if depth == 0 or terminal:
      evaluate(position)
else:
      recurse
   ↓
unmakeMove(move, undo)
   ↓
choose best score
   ↓
return best move
```

This **make → recurse → unmake** loop is the heart of the engine.

***

# SECTION 3: IMPLEMENTATION GUIDE (Senior Developer View)

This section explains how to implement the missing components practically.

***

## 3.1 Add a Proper Move Structure

### Why needed

Right now, moves are represented as `(piece pointer, from, to)`.  
This is not enough for:

*   promotion
*   en passant
*   castling
*   undo
*   search history

### Recommended structure

```text
Move:
    from
    to
    movingPiece
    capturedPiece
    promotionPiece
    flags
```

### Example flags

```text
CAPTURE
DOUBLE_PAWN_PUSH
EN_PASSANT
KING_CASTLE
QUEEN_CASTLE
PROMOTION
```

### Pseudocode

```text
struct Move:
    int from
    int to
    Piece movingPiece
    Piece capturedPiece
    Piece promotionPiece
    int flags
```

### Complexity

*   Memory per move: very small
*   Essential for search

### Best Practices

*   Use enums for piece types and flags
*   Avoid raw `uint64_t*` as the public move interface
*   Keep move representation immutable once created

***

## 3.2 Extend Game State

### Add fields

You need these in `Board` / `Position`:

```text
bool whiteToMove
int castlingRights      // bitmask: WK, WQ, BK, BQ
int enPassantSquare     // -1 if none
int halfmoveClock
int fullmoveNumber
```

### Why needed

*   castling legality depends on castling rights, not just current board placement
*   en passant only exists for one move
*   search and future GUI/protocol support benefit from complete state

### Pseudocode

```text
Position:
    piece bitboards
    whiteToMove
    castlingRights
    enPassantSquare
    halfmoveClock
    fullmoveNumber
```

### Best Practices

*   Initialize all state in `ResetBoard()`
*   Treat this state as part of position identity
*   If you add transposition later, this state must be included in hashing

***

## 3.3 Implement Make/Unmake Move

This is one of the most important changes.

### Current problem

Your `makeMove()`:

*   performs validation and move
*   partially rolls back illegal self-check
*   is not structured for search recursion

### Target design

Split responsibilities:

1.  **Move generation creates candidate moves**
2.  **makeMove(move, undo)** applies a move
3.  **unmakeMove(move, undo)** restores previous state

### Step-by-step

#### Step 1: Create `UndoState`

Store:

*   previous castling rights
*   previous en passant square
*   previous halfmove clock
*   captured piece info
*   previous side to move

#### Step 2: `makeMove()`

*   save undo state
*   clear moving piece from `from`
*   handle capture
*   handle special moves
*   update state
*   switch side
*   reject if own king in check

#### Step 3: `unmakeMove()`

*   reverse all changes exactly

### Pseudocode

```text
function makeMove(move, undo):
    undo.save(current state)

    remove moving piece from move.from

    if move is capture:
        remove captured piece

    if move is en passant:
        remove pawn behind destination

    if move is castling:
        move rook as well

    if move is promotion:
        place promoted piece on destination
    else:
        place moving piece on destination

    update castling rights
    update en passant square
    update halfmove/fullmove
    switch side to move

    if own king is in check:
        unmakeMove(move, undo)
        return false

    return true
```

```text
function unmakeMove(move, undo):
    restore side to move
    restore castling rights
    restore en passant square
    restore halfmove/fullmove

    undo piece movement
    restore captured piece if any
    undo rook movement for castling
    restore pawn for en passant
    undo promotion
```

### Complexity

*   `makeMove`: O(1) except king-check verification
*   `unmakeMove`: O(1)

### Trade-offs

*   Make/unmake is faster than copying the full board each node
*   Requires careful implementation and testing

### Best Practices

*   `makeMove()` and `unmakeMove()` must be perfectly symmetric
*   Write tests specifically for make/unmake reversibility
*   Never generate search children by copying the entire board unless for debugging only

***

## 3.4 Implement Full Move Generation

You need **legal move generation**.

### Recommended strategy

Generate **pseudo-legal moves**, then filter with `makeMove()`.

This is simpler and suitable for your project.

***

### 3.4.1 Pawn Move Generation

Need to generate:

*   single push
*   double push
*   captures
*   promotion
*   en passant

### Pseudocode

```text
for each pawn square:
    oneStep = square + forward
    if empty(oneStep):
        if oneStep is promotion rank:
            add 4 promotion moves
        else:
            add quiet move

        if pawn on start rank and twoStep empty:
            add double pawn push

    for each diagonal capture square:
        if enemy piece exists:
            if promotion rank:
                add 4 promotion capture moves
            else:
                add capture move

        if target == enPassantSquare:
            add en passant move
```

### Complexity

*   O(number of pawns)

### Trade-offs

*   Explicit generation is easy and readable
*   Bitboard bulk generation is faster but more complex

### Best Practices

*   Generate all 4 promotion pieces: queen, rook, bishop, knight
*   In search, queen promotion will dominate, but legal move generation must include all

***

### 3.4.2 Knight Move Generation

### Pseudocode

```text
for each knight square:
    for each of 8 knight offsets:
        target = square + offset
        if on board and not occupied by friendly piece:
            add move
```

### Complexity

*   O(number of knights × 8)

### Best Practices

*   Prefer iterating actual set bits rather than scanning 64 squares every time

***

### 3.4.3 Bishop / Rook / Queen Generation

### Pseudocode

```text
for each slider square:
    for each direction:
        step outward until:
            off board -> stop
            friendly piece -> stop
            enemy piece -> add capture, stop
            empty square -> add move, continue
```

### Complexity

*   O(total ray lengths), practically acceptable

### Trade-offs

*   Simple ray-walking is easy now
*   Magic bitboards / attack tables can come later if needed

### Best Practices

*   Reuse your current obstruction and attack concepts
*   But for generation, direct ray-walking is better than repeatedly calling `isMoveObstructed()`

***

### 3.4.4 King Move Generation

Need:

*   single-square moves
*   castling

### Castling conditions

For white kingside:

*   castling rights available
*   squares between king and rook empty
*   king not in check
*   squares king crosses are not attacked
*   destination not attacked

Same for queenside and black.

### Pseudocode

```text
generate normal king moves to adjacent squares not occupied by friendly pieces

if castling rights allow:
    if path empty and not attacked:
        add castling move
```

### Complexity

*   O(8) plus a few attack checks

### Best Practices

*   Never treat castling as a normal king move
*   Validate every rule explicitly

***

### 3.4.5 Legal Move Filtering

### Pseudocode

```text
function generateLegalMoves(position):
    pseudoMoves = generatePseudoLegalMoves(position)
    legalMoves = []

    for move in pseudoMoves:
        if makeMove(move, undo):
            legalMoves.push(move)
            unmakeMove(move, undo)

    return legalMoves
```

### Complexity

*   Roughly O(number\_of\_pseudo\_moves × legality\_check)

This is acceptable for a first engine.

***

## 3.5 Implement Check / Checkmate / Stalemate Detection

### Required methods

```text
inCheck(side)
generateLegalMoves()
```

### Logic

*   If legal move count = 0 and side is in check → checkmate
*   If legal move count = 0 and side is not in check → stalemate

### Pseudocode

```text
function inCheck(side):
    kingSq = square of side's king
    return isSquareAttacked(kingSq, opposite side)

function getGameState():
    moves = generateLegalMoves(current position)
    if moves is not empty:
        return NORMAL
    if inCheck(sideToMove):
        return CHECKMATE
    return STALEMATE
```

### Complexity

*   Depends on legal move generation

### Best Practices

*   Keep terminal detection centralized
*   Reuse it inside search

***

## 3.6 Implement Evaluation Function

For your project, keep evaluation simple and effective.

### Recommended v1 evaluation

1.  Material
2.  Piece-square tables
3.  Optional mobility later

***

### Piece values

Start with:

```text
Pawn   = 100
Knight = 320
Bishop = 330
Rook   = 500
Queen  = 900
King   = 0
```

### Piece-square tables

Add bonuses for:

*   central knights
*   advanced pawns
*   active bishops
*   rook on open/semi-open file (optional later)
*   king safety (simple table)

### Pseudocode

```text
function evaluate(position):
    if checkmate for sideToMove:
        return very negative
    if checkmate for opponent:
        return very positive
    if stalemate:
        return 0

    score = 0

    score += material(white) - material(black)
    score += pstScore(white) - pstScore(black)

    return whiteToMove ? score : -score
```

### Complexity

*   O(number of pieces)

### Trade-offs

*   Material + PST is easy and strong enough for a first engine
*   Pawn structure and king safety can wait

### Best Practices

*   Make evaluation deterministic
*   Keep scores in centipawns
*   Use a large mate score, e.g. `±100000`

***

## 3.7 Implement Minimax with Alpha-Beta

### Why alpha-beta?

Plain minimax quickly becomes too slow.  
Alpha-beta is the minimum practical search optimization.

### Core algorithm

```text
function search(depth, alpha, beta):
    if depth == 0 or terminal:
        return evaluate(position)

    moves = generateLegalMoves(position)

    if no legal moves:
        return evaluate terminal position

    if sideToMove is maximizing:
        best = -INF
        for move in moves:
            makeMove(move, undo)
            score = -search(depth - 1, -beta, -alpha)
            unmakeMove(move, undo)

            best = max(best, score)
            alpha = max(alpha, best)
            if alpha >= beta:
                break
        return best
```

This is actually **negamax**, which is simpler than separate min/max functions and recommended.

### Best move search wrapper

```text
function findBestMove(depth):
    bestMove = null
    bestScore = -INF

    moves = generateLegalMoves(position)
    orderMoves(moves)

    for move in moves:
        makeMove(move, undo)
        score = -search(depth - 1, -INF, INF)
        unmakeMove(move, undo)

        if score > bestScore:
            bestScore = score
            bestMove = move

    return bestMove
```

### Complexity

*   Worst case: O(b^d)
*   With alpha-beta and decent move ordering: much better in practice

### Trade-offs

*   Negamax simplifies implementation
*   Fixed-depth search is enough initially
*   Iterative deepening can be added later

### Best Practices

*   Search should not allocate excessive memory per node
*   Use move ordering (captures first) early
*   Keep evaluation fast

***

## 3.8 Add Move Ordering

Move ordering is a high-value, low-complexity optimization.

### Start simple

Order moves by:

1.  promotions
2.  captures
3.  checks (optional later)
4.  quiet moves

### Easy capture ordering

Use **MVV-LVA** style:

*   Most Valuable Victim
*   Least Valuable Attacker

### Pseudocode

```text
scoreMove(move):
    if promotion: high score
    else if capture: victimValue - attackerValue bonus
    else: low score
```

### Complexity

*   Sorting adds overhead, but improves pruning significantly

### Best Practices

*   Start with cheap heuristics
*   Do not overengineer history/killer heuristics until base engine works

***

## 3.9 Add Perft Testing

This is crucial.

### What perft does

Counts the number of legal positions reachable in N plies.

If your move generator is wrong, perft will reveal it quickly.

### Pseudocode

```text
function perft(depth):
    if depth == 0:
        return 1

    nodes = 0
    moves = generateLegalMoves(position)

    for move in moves:
        makeMove(move, undo)
        nodes += perft(depth - 1)
        unmakeMove(move, undo)

    return nodes
```

### Best Practices

*   Test initial position perft 1..5
*   Test castling positions
*   Test en passant positions
*   Test promotion positions
*   Test positions with checks/pins

***

# SECTION 4: CODE REVIEW & BUG ANALYSIS (QA Engineer View)

## 4.1 Logical Bugs / Risks in Current Code

***

### 1. No Special Move Support

Missing:

*   castling
*   en passant
*   promotion

This is the biggest correctness gap.

#### Impact

*   engine cannot play legal chess fully
*   move generation and terminal detection will be wrong in many positions

***

### 2. `listOfValidMoves()` is Empty

This means:

*   no move enumeration
*   no search possible
*   no checkmate/stalemate detection possible

***

### 3. `makeMove()` Interface is Not Search-Friendly

Current signature:

```cpp
bool makeMove(uint64_t *piece, int fromIdx, int toIdx)
```

#### Risks

*   uses raw bitboard pointer as piece identity
*   not extensible to promotion/castling/en passant
*   not suitable for undo history
*   difficult to integrate with search

This should be replaced with a `Move` object.

***

### 4. Promotion Is Currently Illegal by Omission

A pawn can move to the last rank according to current movement rules, but no promotion occurs.

#### Result

*   invalid board state: pawn remains on back rank
*   subsequent logic becomes incorrect

***

### 5. No Castling Rights Tracking

Even if castling were added later, there is currently no state tracking for:

*   whether king moved
*   whether rook moved
*   whether rook was captured

This must be added as explicit state.

***

### 6. No En Passant State Tracking

A double pawn push does not record an en passant target square.

***

### 7. `getPieceBitboardAtSquare()` Is Repeatedly Expensive

It scans all 12 bitboards every time.

#### Impact

*   acceptable for prototype
*   costly during search where millions of calls can occur

***

### 8. `getPiecePosition()` Uses Linear Scan

For kings this is acceptable because there is only one king, but it is still less efficient than using:

*   `std::countr_zero(piece)`

Your comment already notes this.

***

### 9. `moveLegalityCheck()` Only Checks Piece Movement Rules

It does **not** fully represent legal move rules:

*   no castling
*   no promotion
*   no en passant
*   king safety deferred to `makeMove()`

This is okay for pseudo-legal validation, but should be clearly separated conceptually.

***

### 10. Use of Raw Member Pointer for Piece Type

You infer piece type by comparing pointer identity:

```cpp
if (piece == &WP) ...
```

#### Risk

This is fragile design:

*   difficult to test
*   not serializable
*   awkward in move lists
*   easy to misuse externally

Replace with enum-based piece IDs.

***

## 4.2 Edge Cases You Must Handle

Below are the important chess edge cases currently missing or risky.

***

### A. Illegal Moves Into Check

Currently handled in `makeMove()`, which is good.

Must continue to support:

*   pinned piece moving illegally
*   king moving into attacked square
*   discovered attacks after move

***

### B. Checkmate

Need to detect:

*   side to move has no legal moves
*   side to move is in check

***

### C. Stalemate

Need to detect:

*   side to move has no legal moves
*   side to move is not in check

***

### D. Castling Through Check

Must reject castling if:

*   king is in check
*   transit square is attacked
*   destination square is attacked

***

### E. En Passant Self-Check

This is a classic bug source.

Example:

*   en passant captures a pawn
*   removing that pawn opens a rook/queen line onto your king

So en passant must still pass full king safety verification.

***

### F. Promotion Choices

Must include all:

*   queen
*   rook
*   bishop
*   knight

Not just queen.

***

### G. Capturing King

Your move system should never allow actual “king capture” as a legal move concept.  
Instead:

*   legal move generation prevents positions where side to move leaves opponent king “capturable”
*   checkmate ends the game

***

### H. Pinned Pieces

A pinned piece may appear pseudo-legal but be illegal because it exposes the king.
Your future legal move filtering via `makeMove()` will handle this.

***

### I. Empty or Corrupted Position States

Potentially helpful later:

*   assert exactly one white king and one black king
*   detect impossible states during testing

***

## 4.3 Performance Bottlenecks

***

### 1. Full-Square Scans

Several functions scan squares linearly:

*   `getPiecePosition()`
*   `getPieceBitboardAtSquare()`
*   likely future move generation if implemented naively

#### Optimization

Iterate set bits directly:

```text
while (bitboard != 0):
    sq = countr_zero(bitboard)
    bitboard &= bitboard - 1
```

***

### 2. Repeated Occupancy Recalculation

`GetWhitePieces()`, `GetBlackPieces()`, `GetAllPieces()` recompute unions every call.

#### Optimization

Optionally cache:

*   white occupancy
*   black occupancy
*   all occupancy

and update incrementally during moves.

For now, recomputation is okay; later optimize if profiling shows it matters.

***

### 3. `getPieceBitboardAtSquare()` O(12)

During search, this becomes expensive.

#### Better options

*   maintain a 64-square piece array in addition to bitboards
*   or use a function returning piece enum from square cache

A **hybrid board representation** is often practical:

*   bitboards for speed
*   array\[64] for direct square lookup

This is highly recommended for your project because it simplifies:

*   captures
*   promotions
*   debug printing
*   move encoding

***

### 4. Attack Detection Ray-Walking Repeatedly

`isSquareAttacked()` walks rays each call.  
That is fine initially, but search will call it often.

#### Recommendation

Keep current approach first.  
Only optimize further once move generation is correct.

***

## 4.4 Suggested Optimizations

### High-value optimizations first

1.  add legal move generator
2.  add make/unmake
3.  iterate set bits instead of scanning 64 squares
4.  order captures/promotions first
5.  use alpha-beta

### Low-priority / later

*   transposition table
*   killer moves/history
*   precomputed attack tables
*   magic bitboards

For your project, these are **nice-to-have**, not mandatory for a good result.

***

# SECTION 5: PERFORMANCE & SCALABILITY

## 5.1 Memory Considerations

### Current memory usage

Very low:

*   12 bitboards = 96 bytes
*   a few extra fields

This is excellent.

### After adding move search

Memory usage will come from:

*   move lists per node
*   undo stack
*   recursion stack
*   optional transposition table

### Practical approach

For your project, memory will still remain modest if you avoid copying full board states.

***

## 5.2 Recommended Memory Strategy

### A. Use Make/Unmake Instead of Copying Boards

This is the most important memory optimization.

**Good**

```text
make move
search
unmake move
```

**Avoid**

```text
copy full board for every child node
```

***

### B. Use Compact Move Representation

A move can fit in a few integers / flags.  
No need for heavy objects.

***

### C. Use Undo Stack

Store only changed state, not entire board snapshots.

***

## 5.3 Core Runtime Optimizations

### 1. Alpha-Beta Pruning

This is the highest-value optimization.

### 2. Move Ordering

Even simple ordering gives large gains.

### 3. Set-Bit Iteration

Avoid board-wide scans where possible.

### 4. Hybrid Representation (Recommended)

Use:

*   bitboards for move logic
*   `pieceOn[64]` array for direct lookup

This simplifies many operations and reduces repeated bitboard scanning.

### 5. Fast King Lookup

Store king squares explicitly:

```text
int whiteKingSquare
int blackKingSquare
```

Update them during make/unmake rather than recomputing.

This is a very practical optimization.

***

## 5.4 Scalability Expectations

With a simple but clean implementation:

*   legal move generation
*   alpha-beta
*   basic move ordering
*   material + PST evaluation

You should be able to search a few plies reasonably well for a personal project.

That is enough to build a **good, functional chess engine** without needing advanced engine theory.

***

# SECTION 6: FINAL ACTION PLAN

Below is the **prioritized checklist** of what you should do next.

***

## Priority 1 — Refactor Position and Move Handling

### Do next

*   [ ] Create `enum Piece`
*   [ ] Create `struct Move`
*   [ ] Create `struct UndoState`
*   [ ] Add castling rights
*   [ ] Add en passant square
*   [ ] Add halfmove/fullmove counters
*   [ ] Replace `makeMove(uint64_t*, from, to)` with `makeMove(Move, UndoState&)`
*   [ ] Implement `unmakeMove(Move, UndoState&)`

**Reason:** search is not feasible without this.

***

## Priority 2 — Implement Legal Move Generation

### Do next

*   [ ] Generate pawn moves (single, double, captures)
*   [ ] Add promotions
*   [ ] Add en passant generation
*   [ ] Generate knight moves
*   [ ] Generate bishop/rook/queen moves
*   [ ] Generate king moves
*   [ ] Add castling generation
*   [ ] Filter pseudo-legal moves into legal moves

**Reason:** this is the engine’s operational core.

***

## Priority 3 — Add Game State Detection

### Do next

*   [ ] Implement `inCheck(side)`
*   [ ] Implement legal move count check
*   [ ] Implement checkmate detection
*   [ ] Implement stalemate detection

***

## Priority 4 — Add Evaluation

### Do next

*   [ ] Add material scoring
*   [ ] Add simple piece-square tables
*   [ ] Return centipawn score from side-to-move perspective

***

## Priority 5 — Implement Search

### Do next

*   [ ] Implement negamax
*   [ ] Add alpha-beta pruning
*   [ ] Add best move search wrapper
*   [ ] Add mate score handling

***

## Priority 6 — Add Basic Optimizations

### Do next

*   [ ] Order promotions and captures first
*   [ ] Iterate set bits directly
*   [ ] Store king squares explicitly
*   [ ] Consider `pieceOn[64]` array for fast square lookup

***

## Priority 7 — Build Testing

### Do next

*   [ ] Write perft function
*   [ ] Test initial position perft
*   [ ] Test castling positions
*   [ ] Test en passant positions
*   [ ] Test promotion positions
*   [ ] Test pinned pieces / checks

***

# Final Engineering Summary

## What you already have

You already built a solid **board-state prototype** with:

*   bitboard representation
*   attack detection
*   piece-specific movement rules
*   legal move execution with self-check rejection

That is a strong starting point.

## What is still missing

To become a real Minimax engine, you must add:

1.  **move representation**
2.  **legal move generation**
3.  **special rules**
4.  **make/unmake**
5.  **evaluation**
6.  **search**
7.  **testing**

## Best practical path

For a personal project, the best architecture is:

*   **bitboards + optional square array**
*   **pseudo-legal generation + legal filtering**
*   **make/unmake**
*   **negamax + alpha-beta**
*   **simple material + PST evaluation**
*   **perft-based validation**

That will be **good enough, efficient, and maintainable** without becoming overly theoretical.

***

If you want, I can do one of these next:

1.  **design the exact `Move`, `UndoState`, and `Position` structs for your codebase**,
2.  **write the `generateLegalMoves()` architecture in C++-style pseudocode**, or
3.  **help you refactor your current class into a search-ready engine layout**.
