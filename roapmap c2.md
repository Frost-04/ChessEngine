# Technical Analysis & Design Document for Your Minimax-Based Chess Engine

Your current code is **a good starting point for a rules-aware bitboard board model**, but it is **not yet a chess engine** in the Minimax sense. Right now, it supports:

*   initial board setup,
*   piece occupancy queries,
*   attack detection,
*   basic piece movement legality,
*   state mutation for a single move with self-check prevention,
*   board printing.

That means the code currently behaves more like a **position state container with partial move validation**, not a search engine.

The most important architectural conclusion is this:

> **Before implementing Minimax, you must finish the chess position model and legal move generation.**  
> Search depends on generating *all legal moves* correctly and being able to *make/unmake* them efficiently and safely.

If you skip that and go directly to Minimax, you will build search over an incorrect game model, which is the worst possible failure mode: **fast wrong answers**.

***

# SECTION 1: CURRENT SYSTEM ANALYSIS (Software Architect View)

***

## 1.1 What the current code does

### Board representation

The engine represents the board using **12 bitboards**:

*   White: `WP, WN, WB, WR, WQ, WK`
*   Black: `BP, BN, BB, BR, BQ, BK`

Each is a `uint64_t`, where each bit corresponds to one square.

This is a standard and efficient low-level representation for chess engines.

### Current capabilities implemented

The code currently implements:

1.  **Initial position setup**
    *   `ResetBoard()` initializes all 12 bitboards to the standard starting chess position.
    *   `whiteToMove` is initialized to `true`.

2.  **Occupancy aggregation**
    *   `GetWhitePieces()`
    *   `GetBlackPieces()`
    *   `GetAllPieces()`

3.  **Bit manipulation helpers**
    *   `GetBit(piece, i)`
    *   `SetBit(piece, i)`
    *   `ResetBit(piece, i)`

4.  **Single-piece square lookup**
    *   `getPiecePosition(uint64_t piece)`  
        Returns the least significant occupied square index by scanning from bit 0 to 63. Intended mainly for kings.

5.  **Square-to-bitboard ownership lookup**
    *   `getPieceBitboardAtSquare(int i)`  
        Returns a pointer to whichever bitboard contains square `i`.

6.  **Attack detection**
    *   `isSquareAttacked(int sq, bool byWhite)`  
        Checks whether a square is attacked by:
        *   pawns,
        *   knights,
        *   bishops,
        *   rooks,
        *   queens,
        *   king.

7.  **Sliding path obstruction test**
    *   `isMoveObstructed(int from, int to)`  
        Assumes the move is a valid sliding line and checks whether intermediate squares are occupied.

8.  **Pseudo-legal move validation (piece-rule level)**

    *   `moveLegalityCheck(uint64_t *piece, int from, int to)`  
        Verifies:
        *   indices are valid,
        *   `from != to`,
        *   piece exists at `from`,
        *   no own-piece capture,
        *   movement conforms to piece rule,
        *   sliders are not obstructed.

    It explicitly **does not** handle:

    *   castling,
    *   en passant,
    *   promotion,
    *   king safety (except indirectly in `makeMove`).

9.  **Move execution with self-check rejection**
    *   `makeMove(uint64_t *piece, int fromIdx, int toIdx)`  
        Performs:
        *   pseudo-legal validation,
        *   turn-color enforcement,
        *   capture removal,
        *   piece movement,
        *   own-king safety validation using `isSquareAttacked`,
        *   rollback if move leaves own king in check,
        *   toggles side to move on success.

10. **ASCII board rendering**
    *   `printBoard()`

### What it does **not** do

It does **not** currently:

*   generate legal move lists,
*   detect checkmate/stalemate,
*   evaluate positions,
*   search future positions,
*   select best moves,
*   handle full chess rules,
*   maintain move history,
*   support undo,
*   manage search depth/time.

So this is not yet a Minimax engine; it is a **partial legal-state transition layer**.

***

## 1.2 Implemented components

Here is the precise component inventory.

### Implemented now

*   **Board state storage**: yes
*   **Bitboard occupancy logic**: yes
*   **Board initialization**: yes
*   **Piece lookup by square**: yes
*   **Attack detection**: partially yes
*   **Pseudo-legal move validation**: yes
*   **Move execution with own king safety check**: partially yes
*   **Turn handling**: yes
*   **Board display**: yes

### Only partially implemented

*   **Legality model**: partial  
    Some legal constraints are applied, but full legal chess is not implemented.
*   **King safety**: partial  
    Own king safety after move is enforced, but not all terminal/structural rules.
*   **Special move support**: absent
*   **Move generation**: absent (`listOfValidMoves()` is empty)

***

## 1.3 Missing components required for a Minimax chess engine (exhaustive)

Below is the full list of missing or incomplete subsystems required to turn this into a real chess engine.

***

### A. Position/state model gaps

These are required for full legal chess.

1.  **Castling rights**
    *   Need 4 flags:
        *   white kingside,
        *   white queenside,
        *   black kingside,
        *   black queenside.

2.  **En passant state**
    *   Need to track en passant target square after a double pawn push.

3.  **Promotion handling**
    *   Need promotion moves to Q/R/B/N.

4.  **Halfmove clock**
    *   Needed for 50-move rule.

5.  **Fullmove number**
    *   Useful for PGN/FEN and bookkeeping.

6.  **Move history / undo state**
    *   Required for:
        *   search,
        *   threefold repetition,
        *   debugging,
        *   UI/game replay.

7.  **Position hashing**
    *   Not strictly required initially, but effectively required for strong search/transposition table.

***

### B. Legal move generation gaps

1.  **Generate all pseudo-legal moves**
2.  **Filter to legal moves**
    *   or generate legal directly
3.  **King moves including check restrictions**
4.  **Castling legality**
    *   rook/king unmoved,
    *   empty path,
    *   king not in check,
    *   squares crossed not attacked.
5.  **En passant legality**
    *   including discovered-check edge case.
6.  **Pawn promotions**
7.  **Terminal move absence detection**
    *   no legal moves + in check = checkmate
    *   no legal moves + not in check = stalemate

***

### C. Search engine gaps

1.  **Move list representation**
    *   `Move` struct/class
2.  **Minimax / Negamax**
3.  **Alpha-beta pruning**
4.  **Search depth control**
5.  **Terminal node scoring**
6.  **Mate score convention**
7.  **Quiescence search** (important for tactical stability)
8.  **Iterative deepening**
9.  **Search statistics / node counting**
10. **Time control** (optional initially)

***

### D. Evaluation gaps

1.  **Material evaluation**
2.  **Piece-square tables**
3.  **Mobility**
4.  **King safety**
5.  **Pawn structure**
6.  **Passed pawns / doubled / isolated pawns**
7.  **Endgame handling**
8.  **Game phase interpolation** (nice-to-have)

***

### E. Search efficiency gaps

1.  **Move ordering**
    *   captures first,
    *   promotions,
    *   checks,
    *   killer heuristic,
    *   history heuristic,
    *   TT move.
2.  **Transposition table**
3.  **Zobrist hashing**
4.  **Efficient make/unmake**
5.  **Incremental evaluation** (later)
6.  **Better bit scan routines**
7.  **Fast move generation primitives**

***

### F. Game/UI/API gaps

1.  **Game loop**
    *   user vs engine / engine vs engine
2.  **Move input parser**
    *   e.g. `"e2e4"`, `"e7e8q"`
3.  **Move output formatter**
4.  **FEN import/export**
5.  **Terminal state reporting**
6.  **Illegal move feedback**
7.  **Testing harness / perft**

***

### G. QA / correctness infrastructure gaps

1.  **Unit tests**
2.  **Perft validation**
3.  **Illegal-state assertions**
4.  **Regression tests**
5.  **Move/unmove invariants**
6.  **Position consistency checks**

***

## 1.4 Architecture diagram in text form

A sensible architecture for your engine should look like this:

```text
+--------------------------------------------------+
|                   Game Loop/UI                   |
|--------------------------------------------------|
| Reads move input, prints board, calls engine     |
| Handles game progression, terminal states        |
+----------------------------+---------------------+
                             |
                             v
+--------------------------------------------------+
|                    Search Engine                 |
|--------------------------------------------------|
| Negamax / Minimax                               |
| Alpha-beta pruning                              |
| Iterative deepening                             |
| Quiescence search                               |
| Move ordering                                   |
| Best-move selection                             |
+----------------------------+---------------------+
                             |
                             v
+--------------------------------------------------+
|                    Evaluator                     |
|--------------------------------------------------|
| Material                                         |
| Piece-square tables                              |
| Mobility                                         |
| Pawn structure                                   |
| King safety                                      |
| Endgame scaling                                  |
+----------------------------+---------------------+
                             |
                             v
+--------------------------------------------------+
|                Move Generation Layer             |
|--------------------------------------------------|
| Generate pseudo-legal moves                      |
| Generate legal moves                             |
| Attack detection                                 |
| Check/checkmate/stalemate detection              |
| Special moves: castling, en passant, promotion   |
+----------------------------+---------------------+
                             |
                             v
+--------------------------------------------------+
|                  Position / Board                |
|--------------------------------------------------|
| 12 piece bitboards                               |
| side to move                                     |
| castling rights                                  |
| en passant square                                |
| halfmove/fullmove counters                        |
| make move / unmake move                          |
| occupancy bitboards                              |
| square queries                                   |
+----------------------------+---------------------+
                             |
                             v
+--------------------------------------------------+
|             Utility / Validation Layer           |
|--------------------------------------------------|
| Move struct                                      |
| Undo struct                                      |
| Zobrist hashing                                  |
| FEN I/O                                          |
| Perft                                            |
| Assertions / tests                               |
+--------------------------------------------------+
```

***

## 1.5 Implicit design assumptions in the current code

These assumptions are present even though not explicitly documented.

### 1. Square mapping assumption

The code assumes:

*   A1 is bit 0,
*   ranks increase upward,
*   file = `sq % 8`,
*   rank = `sq / 8`.

This is embedded in pawn movement, board printing, and attack math.

### 2. One bitboard per piece type per side

This implies:

*   piece type is inferred by **which bitboard pointer you passed**,
*   not by explicit piece enum.

### 3. Piece identity is pointer-based

`moveLegalityCheck` determines piece type using comparisons like:

*   `piece == &WP`
*   `piece == &BN`

This is a fragile API design assumption:

*   correctness depends on passing internal member addresses only.

### 4. King existence invariant

`makeMove` assumes own king exists and `getPiecePosition(WK/BK)` returns valid square.  
But no explicit invariant check enforces that both kings remain present.

### 5. Legal move validation is “apply then test”

The design assumes:

*   move rules first,
*   then mutate board,
*   then test king safety.

That is fine, but it strongly implies you will need **undo/makeMove discipline** as the engine grows.

### 6. The engine assumes external callers behave correctly

There are many internal helpers that rely on preconditions rather than robust contracts:

*   `isMoveObstructed` assumes sliding move
*   `getPiecePosition` assumes single-bit piece for kings
*   `getPieceBitboardAtSquare` assumes board consistency

***

## 1.6 Architectural weaknesses and technical debt

These are the most important current technical debt items.

***

### 1. No `Move` type

Current API is:

*   pointer to bitboard,
*   `from`,
*   `to`.

This is not scalable because a complete move must eventually encode:

*   source,
*   destination,
*   moving piece,
*   captured piece,
*   promotion piece,
*   castling flag,
*   en passant flag,
*   prior castling rights,
*   prior en passant state,
*   halfmove reset info.

**Impact:** high.  
Without a move representation, search and undo become error-prone.

***

### 2. No `Undo` / unmake infrastructure

Search requires making and unmaking millions of moves.  
Current rollback in `makeMove` is only enough for local legality rejection, not full recursive search.

**Impact:** critical.

***

### 3. Piece-type detection by pointer identity

This is the biggest API smell in the current design.

Example:

```cpp
if (piece == &WP) { ... }
```

This is brittle because:

*   internal representation leaks into callers,
*   it prevents clean abstractions,
*   it complicates move generation,
*   it makes refactoring harder.

**Preferred design:** `Move` contains piece type enum, or the board can query piece type by source square.

***

### 4. No explicit position metadata

A chess engine without:

*   castling rights,
*   en passant square,
*   halfmove clock

cannot represent legal chess positions fully.

This means search would explore an incomplete game tree.

***

### 5. `makeMove` mixes concerns

It currently handles:

*   pseudo-legal validation,
*   turn validation,
*   capture,
*   state mutation,
*   legal check validation,
*   rollback,
*   side-to-move update.

This is too much in one function.

Better separation:

*   `generatePseudoLegalMoves`
*   `makeMoveUnchecked`
*   `isKingInCheck`
*   `isLegalMove`
*   `makeMove` / `unmakeMove`

***

### 6. `getPieceBitboardAtSquare()` is O(12)

Every capture lookup scans 12 bitboards.  
Fine for now, but it is a symptom that the engine lacks:

*   either fast board mailbox overlay,
*   or piece-on-square lookup cache,
*   or a move generator that already knows capture targets.

***

### 7. Attack detection is correct-oriented but not optimized

`isSquareAttacked()` loops over rays and uses repeated `GetBit` checks.  
This is acceptable for early development but will become hot-path expensive during search.

***

### 8. No legal move generator

This is the biggest functional gap.  
Minimax cannot exist meaningfully until legal move generation exists.

***

### 9. No terminal-state detection

Without checkmate/stalemate detection, search cannot assign correct terminal values.

***

### 10. No testing strategy embedded in the architecture

For chess engines, **perft** is not optional.  
Without perft validation, you will almost certainly search incorrect trees.

***

# SECTION 2: ROADMAP & DESIGN (Architect View)

***

## 2.1 Recommended implementation phases

The ordering matters. This is the right build sequence for your current codebase.

***

### Phase 1 — Stabilize the board model and complete legal chess state

**Why first:** Search depends on correctness. If board state is incomplete, every later layer is wrong.

Implement:

*   `Move` struct
*   `UndoState` struct
*   castling rights
*   en passant square
*   promotion support
*   halfmove/fullmove counters
*   robust make/unmake move

**Justification:** This creates a correct, searchable position representation.

***

### Phase 2 — Build full move generation

**Why second:** Search requires legal children.  
This is the foundation for:

*   terminal detection,
*   evaluation mobility,
*   search branching.

Implement:

*   pseudo-legal move generation
*   legal move filtering
*   special move generation
*   check/checkmate/stalemate support

**Justification:** Without a full move generator, Minimax has nothing reliable to search.

***

### Phase 3 — Add evaluation

**Why third:** Once positions and legal moves are correct, you can score leaves.

Start with:

*   material
*   piece-square tables
*   simple mobility bonus

**Justification:** Keep it simple initially; good search with basic eval beats sophisticated eval over an incorrect move generator.

***

### Phase 4 — Implement Negamax + Alpha-Beta

**Why fourth:** By now you can:

*   generate legal moves,
*   make/unmake them,
*   score leaves.

Implement:

*   negamax(depth)
*   alpha-beta pruning
*   terminal scoring
*   mate score conventions

**Justification:** This is the first “engine” phase.

***

### Phase 5 — Improve search quality

Implement:

*   move ordering
*   quiescence search
*   iterative deepening
*   basic transposition table

**Justification:** This gives major strength gains per engineering hour.

***

### Phase 6 — Game loop and usability

Implement:

*   move parser
*   engine-vs-human loop
*   FEN import/export
*   perft and tests

**Justification:** Once the engine core works, make it usable and debuggable.

***

## 2.2 Required modules and definitions

***

## Module 1: Minimax / Negamax Algorithm

### Recommendation

Use **Negamax**, not classical Minimax.

Why:

*   chess is a zero-sum game,
*   Negamax is simpler,
*   it reduces duplicated max/min logic.

### Responsibility

Given a position and depth:

*   recursively explore legal moves,
*   score resulting positions,
*   propagate best value upward.

### Inputs

*   current `Board`
*   search depth
*   alpha, beta
*   side to move

### Outputs

*   score
*   optionally best move at root

### Rejected alternative

**Classical Minimax with separate max/min functions**

*   rejected because it duplicates logic and complicates maintenance.

***

## Module 2: Alpha-Beta Pruning

### Responsibility

Reduce the number of explored nodes by pruning branches that cannot affect final decision.

### Inputs

*   current search score bounds (`alpha`, `beta`)
*   ordered move list

### Outputs

*   same best score as full minimax, but faster

### Trade-off

*   adds complexity to search implementation,
*   but absolutely worth it.

**This is a must-have**, not an optional optimization.

***

## Module 3: Evaluation Function

### Responsibility

Assign a numeric score to non-terminal positions.

### Recommended initial components

1.  material
2.  piece-square tables
3.  mobility (small weight)
4.  checkmate/stalemate handling at terminal nodes

### Avoid initially

*   heavy pawn-structure logic,
*   king tropism,
*   tapered eval,
*   NNUE-like complexity.

### Trade-off

*   Simple eval is fast and easy to debug.
*   Rich eval improves strength but increases tuning/debug overhead.

For a personal project, begin with:

*   material + PST + terminal conditions.

***

## Module 4: Move Ordering

### Responsibility

Sort moves so alpha-beta cuts earlier.

### Initial ordering strategy

1.  TT move (later)
2.  promotions
3.  winning captures (MVV-LVA)
4.  checks
5.  killer moves
6.  history heuristic
7.  quiet moves

### Trade-off

*   Strong move ordering dramatically improves search speed,
*   but heuristics complicate move generation and bookkeeping.

For your stage:

*   start with **captures/promotions/checks first**,
*   add killer/history later.

***

## Module 5: Game Loop

### Responsibility

Orchestrate the interaction between user and engine:

*   show board,
*   accept move,
*   validate it,
*   ask engine for reply,
*   detect game over.

### Inputs

*   player move string or GUI command
*   engine side
*   position

### Outputs

*   board updates
*   game result
*   engine move

### Design note

Keep this layer thin.  
It should not contain chess logic; it should call board/search modules.

***

## 2.3 Explicit module interaction and data flow

Here is the intended data flow.

```text
[Game Loop]
    |
    | user move / engine turn
    v
[Board / Position]
    |
    | generate legal moves
    v
[Move Generator]
    |
    | vector<Move>
    v
[Search Engine]
    |
    | for each move: make move -> recurse -> unmake move
    v
[Board / Position]
    |
    | if depth == 0 or terminal
    v
[Evaluator]
    |
    | returns integer score
    v
[Search Engine]
    |
    | best move + best score
    v
[Game Loop]
```

### More detailed recursion flow

```text
search(position, depth, alpha, beta):
    if terminal(position): return terminal_score
    if depth == 0: return evaluate(position)

    moves = generateLegalMoves(position)
    order(moves)

    for move in moves:
        undo = makeMove(position, move)
        score = -search(position, depth - 1, -beta, -alpha)
        unmakeMove(position, move, undo)

        update best score / alpha
        if alpha >= beta: prune
```

***

## 2.4 Data flow between components (input/output contracts)

This matters a lot for clean implementation.

***

### `Board`

**Input:** move application requests  
**Output:** updated position state

Should expose:

*   occupancy queries
*   side to move
*   king square lookup
*   make/unmake
*   move generation entry point

***

### `MoveGenerator`

**Input:** immutable or mutable board reference  
**Output:** `vector<Move>`

Two possible APIs:

```cpp
std::vector<Move> generatePseudoLegalMoves(const Board&);
std::vector<Move> generateLegalMoves(Board&);
```

***

### `Evaluator`

**Input:** `const Board&`  
**Output:** integer score from side-to-move perspective or white perspective

Be consistent:

*   either always white-positive
*   or current-player-positive

I recommend:

*   **white-positive** at evaluation layer,
*   Negamax handles sign by side-to-move.

***

### `Search`

**Input:** board, depth, alpha, beta  
**Output:** best score / best move

Root interface:

```cpp
Move findBestMove(Board& board, int depth);
```

***

### `GameLoop`

**Input:** commands / user move strings  
**Output:** board update / printed move / result

***

## 2.5 Design choices and rejected alternatives

***

### Choice: Bitboards

**Accepted.**  
They are efficient and appropriate.

**Rejected alternative:** full mailbox-only representation.  
Reason:

*   easier to reason about initially,
*   but slower for move generation and attack work in a serious engine.

### Choice: Keep bitboards, but consider a small piece-on-square helper

You may add:

```cpp
Piece pieceAt[64];
```

or derive via helper.

**Trade-off:**

*   More state duplication,
*   but simpler move generation and debugging.

For a personal project, a **hybrid bitboard + mailbox cache** is often pragmatic.

***

### Choice: Negamax instead of Minimax

**Accepted.**

**Rejected alternative:** separate max/min functions  
Reason:

*   more code,
*   more duplicated bugs.

***

### Choice: Generate pseudo-legal then filter legal

**Recommended for your stage.**

**Alternative rejected initially:** generate only legal moves directly.  
Reason:

*   possible, but more complex,
*   harder to debug,
*   pseudo-legal + make/unmake legality test is simpler to validate.

Trade-off:

*   slower than highly optimized legal generation,
*   but much easier to implement correctly.

For a personal engine, this is the right trade.

***

### Choice: Make/unmake rather than copy board each node

**Accepted.**

**Rejected alternative:** copy full `Board` for recursion  
Reason:

*   much simpler conceptually,
*   but becomes expensive at search depth.

Trade-off:

*   make/unmake is more bug-prone,
*   but essential for decent performance.

For your project:

*   start with careful make/unmake,
*   not full board copies.

***

# SECTION 3: IMPLEMENTATION GUIDE (Senior Developer View)

***

This section covers the missing components in the order I recommend you implement them.

***

## 3.1 Component: Move representation

### Why this is first

Your current API (`uint64_t* piece, int from, int to`) is insufficient for search and special rules.

***

### What to implement

Define at minimum:

```cpp
enum PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum Color { WHITE, BLACK };

struct Move {
    uint8_t from;
    uint8_t to;
    PieceType piece;
    PieceType captured;
    PieceType promotion;   // NONE if not promotion
    bool isCapture;
    bool isEnPassant;
    bool isCastleKingSide;
    bool isCastleQueenSide;
    bool isDoublePawnPush;
};

struct UndoState {
    uint64_t WP, WN, WB, WR, WQ, WK;
    uint64_t BP, BN, BB, BR, BQ, BK;
    bool whiteToMove;
    uint8_t castlingRights;
    int8_t enPassantSquare;
    uint16_t halfmoveClock;
    uint16_t fullmoveNumber;
};
```

For a personal project, a full-state `UndoState` copy is acceptable initially.

***

### Step-by-step implementation

1.  Add metadata fields to `Board`
    *   `uint8_t castlingRights`
    *   `int8_t enPassantSquare` (`-1` means none)
    *   `halfmoveClock`
    *   `fullmoveNumber`

2.  Define `PieceType` and `Move`.

3.  Create helpers:
    *   `PieceType pieceTypeAt(int sq)`
    *   `Color colorAt(int sq)`

4.  Replace pointer-based APIs progressively.

***

### Pseudocode

```text
function pieceTypeAt(square):
    if WP has square or BP has square: return PAWN
    if WN has square or BN has square: return KNIGHT
    ...
    return NONE

function colorAt(square):
    if whitePieces has square: return WHITE
    if blackPieces has square: return BLACK
    return NONE
```

***

### Complexity

*   `pieceTypeAt`: currently O(12)
*   acceptable for now
*   later can optimize with mailbox cache

***

### Best practices

*   Keep `Move` plain and compact.
*   Do not infer too much later from board state if it can be stored in move.
*   Prefer explicit flags over “magic interpretation.”

***

### Common mistakes

*   Forgetting to encode promotion piece
*   Not preserving enough state to undo correctly
*   Treating en passant as normal capture
*   Updating castling rights incorrectly

***

### Debugging strategies

*   Print moves in algebraic-like `e2e4`, `e7e8q`
*   Assert post-move invariants:
    *   both kings exist,
    *   no square occupied by multiple bitboards,
    *   white and black occupancies do not overlap

***

## 3.2 Component: Full move generation

This is the single most important missing subsystem.

***

### Goal

Generate **all legal moves** for side to move.

### Recommended approach

1.  Generate **pseudo-legal** moves
2.  For each move:
    *   make it
    *   reject if own king in check
    *   unmake it

This is simpler and robust.

***

### Step-by-step implementation

#### Step 1: Implement pseudo-legal generators per piece type

Create functions:

*   `generatePawnMoves(...)`
*   `generateKnightMoves(...)`
*   `generateBishopMoves(...)`
*   `generateRookMoves(...)`
*   `generateQueenMoves(...)`
*   `generateKingMoves(...)`

#### Step 2: Add special moves

*   promotions
*   en passant
*   castling

#### Step 3: Legal filtering

*   for each pseudo-legal move:
    *   `undo = makeMove(move)`
    *   if own king not in check → keep
    *   `unmakeMove(move, undo)`

***

### Pseudocode

```text
function generateLegalMoves(board):
    pseudoMoves = generatePseudoLegalMoves(board)
    legalMoves = []

    for move in pseudoMoves:
        undo = board.makeMove(move)
        if undo is valid:
            if not board.isKingInCheck(sideJustMoved):
                legalMoves.push(move)
            board.unmakeMove(move, undo)

    return legalMoves
```

#### Pawn generation pseudocode

```text
for each pawn square:
    oneStep = from + forward
    if empty(oneStep):
        if promotion rank:
            add 4 promotion moves
        else:
            add move

        if on start rank and empty(twoStep):
            add double-push move

    for each capture target diagonal:
        if enemy on target:
            if promotion rank:
                add 4 promotion captures
            else:
                add capture

        if target == enPassantSquare:
            add en passant move
```

#### King castling pseudocode

```text
if side not currently in check:
    if kingside rights and path empty and transit squares not attacked:
        add kingside castle
    if queenside rights and path empty and transit squares not attacked:
        add queenside castle
```

***

### Complexity

Let:

*   `b` = branching factor,
*   `n` = number of generated pseudo-legal moves.

Move generation is roughly O(n) per node, with additional legality checks.

For a simple engine, this is fine.

***

### Trade-offs

**Pseudo-legal + filter**

*   easier to implement,
*   easier to debug,
*   slower than optimized direct legal generation.

**Direct legal generation**

*   faster,
*   much more complex.

For your project, pseudo-legal + filter is the correct choice.

***

### Best practices

*   Generate moves into a preallocated vector if performance matters.
*   Keep move generation deterministic for easier debugging.
*   Separate generation from execution.

***

### Common mistakes

*   Missing promotions on captures
*   Allowing illegal castling through check
*   Mishandling en passant when it exposes own king to rook/queen attack
*   Forgetting that pinned pieces can have pseudo-legal but not legal moves

***

### Debugging strategies

*   Implement **perft** immediately after move generation
*   Compare node counts against known chess perft positions
*   Test each special rule in isolation

***

## 3.3 Component: Make/Unmake move

This is required before serious search.

***

### Goal

Efficiently apply and revert moves without corrupting board state.

***

### Step-by-step implementation

1.  Create `UndoState` to store pre-move state.
2.  `makeMove(move)`:
    *   save undo,
    *   remove captured piece,
    *   move piece,
    *   handle en passant capture,
    *   handle promotion replacement,
    *   handle castling rook movement,
    *   update castling rights,
    *   update en passant square,
    *   update halfmove/fullmove,
    *   toggle side to move.
3.  `unmakeMove(move, undo)`:
    *   restore all fields from `undo`.

For your stage, restoring full-board snapshots is acceptable. Later you can store only deltas.

***

### Pseudocode

```text
function makeMove(move):
    undo = snapshot(board)

    apply piece movement
    if capture: remove captured piece
    if en passant: remove pawn behind target square
    if promotion: replace pawn with promotion piece
    if castling: move rook as well

    update castling rights
    update enPassantSquare
    update clocks
    toggle sideToMove

    if own king is in check:
        restore(undo)
        return invalid

    return undo
```

***

### Complexity

*   With full snapshot undo: O(1) practical, but larger memory traffic
*   With delta undo: O(1) and more efficient, but more bug-prone

***

### Trade-offs

**Full-state undo**

*   easiest to implement correctly,
*   enough for early personal project.

**Delta undo**

*   better performance,
*   more difficult to debug.

Start with full-state undo.

***

### Common mistakes

*   Not restoring castling rights
*   Not restoring en passant square
*   Incorrect rook movement on castling
*   Removing wrong pawn on en passant
*   Failing to update king square consistently

***

### Debugging strategies

*   Round-trip test:
    *   position P
    *   make move M
    *   unmake move M
    *   assert position == P
*   Run this for every legal move in many positions.

***

## 3.4 Component: Evaluation function

***

### Goal

Return a numeric score for a position.

### Recommended first version

Start simple:

```text
score =
    material
  + pieceSquareTables
  + mobilityBonus
```

Use white-positive scores:

*   positive = white better
*   negative = black better

At search time:

*   if black to move, Negamax sign handles it.

***

### Step-by-step implementation

1.  Assign material values
    *   Pawn = 100
    *   Knight = 320
    *   Bishop = 330
    *   Rook = 500
    *   Queen = 900
    *   King = 0 (not valued materially)

2.  Add piece-square tables for:
    *   pawns
    *   knights
    *   bishops
    *   rooks
    *   queens
    *   king (middlegame table only initially)

3.  Add small mobility term:
    *   number of legal/pseudo-legal moves difference

4.  Handle terminal scores outside evaluator:
    *   mate,
    *   stalemate.

***

### Pseudocode

```text
function evaluate(board):
    score = 0

    score += 100 * (#whitePawns - #blackPawns)
    score += 320 * (#whiteKnights - #blackKnights)
    score += ...

    for each white piece:
        score += PST[piece][square]
    for each black piece:
        score -= PST[piece][mirror(square)]

    score += mobilityWeight * (whiteMobility - blackMobility)

    return score
```

***

### Complexity

*   O(number of pieces + generated moves if mobility used)
*   Fine for early engine

***

### Trade-offs

**Simple evaluation**

*   fast,
*   easy to tune,
*   weak strategically.

**Rich evaluation**

*   stronger,
*   harder to tune/debug.

For a personal project, the correct path is:

1.  material,
2.  PST,
3.  mobility,
4.  later pawn structure.

***

### Common mistakes

*   Evaluating from inconsistent perspective
*   Forgetting to mirror black PST indices
*   Overweighting mobility and making engine suicidal for activity
*   Scoring terminal states inside generic evaluator inconsistently

***

### Debugging strategies

*   Print evaluation breakdown by term
*   Compare symmetrical positions → score should be near zero
*   Remove one term at a time to isolate instability

***

## 3.5 Component: Minimax / Negamax

***

### Goal

Search the move tree to choose the best move.

### Recommendation

Implement **Negamax with alpha-beta** immediately; don’t build plain Minimax first.

***

### Step-by-step implementation

1.  Base cases:
    *   if no legal moves:
        *   if in check → mate score
        *   else stalemate score = 0
    *   if depth == 0 → evaluate

2.  Generate legal moves

3.  Loop through moves

4.  Make move

5.  Recurse with negated bounds

6.  Unmake move

7.  Track best score

8.  At root, remember best move

***

### Pseudocode

```text
function negamax(board, depth, alpha, beta):
    legalMoves = generateLegalMoves(board)

    if legalMoves.empty():
        if board.sideToMoveInCheck():
            return -MATE_SCORE + ply
        else:
            return 0

    if depth == 0:
        return evaluate(board) * (board.whiteToMove ? 1 : -1)

    best = -INF

    orderMoves(legalMoves)

    for move in legalMoves:
        undo = makeMove(move)
        score = -negamax(board, depth - 1, -beta, -alpha)
        unmakeMove(move, undo)

        if score > best:
            best = score

        if best > alpha:
            alpha = best

        if alpha >= beta:
            break

    return best
```

Root:

```text
function findBestMove(board, depth):
    moves = generateLegalMoves(board)
    orderMoves(moves)

    bestMove = null
    bestScore = -INF
    alpha = -INF
    beta = INF

    for move in moves:
        undo = makeMove(move)
        score = -negamax(board, depth - 1, -beta, -alpha)
        unmakeMove(move, undo)

        if score > bestScore:
            bestScore = score
            bestMove = move

        alpha = max(alpha, bestScore)

    return bestMove
```

***

### Complexity

*   Plain minimax: O(b^d)
*   Alpha-beta best case: roughly O(b^(d/2)) with good move ordering
*   Real outcome depends heavily on ordering quality

***

### Best practices

*   Use mate scores with ply adjustment:
    *   `MATE_SCORE - ply`
    *   prefers faster mates, avoids slower ones
*   Keep evaluation perspective consistent
*   Track node counts for profiling

***

### Common mistakes

*   Returning mate score with wrong sign
*   Forgetting stalemate handling
*   Using inconsistent evaluation perspective
*   Not unmaking move on every code path
*   Using pseudo-legal moves without legality filtering

***

### Debugging strategies

*   Start depth = 1, verify obvious captures
*   Then depth = 2, verify tactical responses
*   Log principal variation at each depth
*   Compare engine move choices in trivial positions

***

## 3.6 Component: Move ordering

***

### Goal

Improve alpha-beta effectiveness.

### Recommended first implementation

Score moves before search:

1.  Promotions
2.  Captures using MVV-LVA
3.  Checking moves
4.  Quiet moves

Later add:

*   killer moves,
*   history heuristic,
*   TT move.

***

### Pseudocode

```text
function scoreMove(move):
    score = 0

    if move.promotion != NONE:
        score += 100000

    if move.isCapture:
        score += 10000 + victimValue(move.captured) - attackerValue(move.piece)

    if move.givesCheck:
        score += 500

    score += killer/history bonuses if available

    return score
```

***

### Complexity

*   O(n log n) if full sort
*   O(n) selection methods possible later

***

### Trade-offs

*   Full sort is simple.
*   More advanced partial ordering is faster but unnecessary early.

***

### Common mistakes

*   Spending too much time on move ordering before correctness
*   Treating all captures as good (losing captures can be bad)
*   Not recomputing check information consistently

***

### Debugging strategies

*   Print ordered move scores
*   Compare node counts before/after ordering
*   Validate same best move is returned

***

## 3.7 Component: Game loop

***

### Goal

Make the engine playable and testable interactively.

### Step-by-step implementation

1.  Print board
2.  If human turn:
    *   read move string (`e2e4`, `e7e8q`)
    *   parse into `Move`
    *   validate against generated legal moves
3.  If engine turn:
    *   call `findBestMove(depth)`
    *   make move
4.  Check terminal conditions
5.  Repeat

***

### Pseudocode

```text
while true:
    printBoard()

    legalMoves = generateLegalMoves(board)
    if legalMoves.empty():
        if inCheck(board.sideToMove):
            print("checkmate")
        else:
            print("stalemate")
        break

    if humanTurn(board):
        input = readMove()
        move = parseMove(input)
        if move not in legalMoves:
            print("illegal")
            continue
        makeMove(move)
    else:
        move = findBestMove(board, depth)
        print(moveToString(move))
        makeMove(move)
```

***

### Common mistakes

*   Parsing a move that is syntactically valid but not actually legal
*   Allowing user to bypass promotion selection
*   Using engine-generated pseudo-legal moves for input validation

***

### Debugging strategies

*   Validate parsed moves against actual legal move list
*   Log move history
*   Allow FEN load for scenario reproduction

***

# SECTION 4: CODE REVIEW & BUG ANALYSIS (QA Engineer View)

***

## 4.1 Logical bugs or risks in the current code

Below are the specific issues in the code as written.

***

### Bug/Risk 1: `listOfValidMoves()` is empty

This is not just a missing feature — it means the engine cannot:

*   detect mate/stalemate,
*   enumerate candidate moves for search,
*   validate user choices against all legal options.

**Impact:** critical blocker for engine development.

***

### Bug/Risk 2: Capturing the enemy king appears possible

Your logic allows moving onto any enemy-occupied square if piece movement rules permit it. There is no explicit prohibition on capturing the enemy king.

In legal chess:

*   kings are never “captured”;
*   positions are resolved by checkmate before that.

**Failure scenario**

*   White queen has line to black king square.
*   `moveLegalityCheck` accepts destination if movement is legal.
*   `makeMove` removes black king bit.
*   Board enters impossible state.

**Fix**

*   Forbid moves whose destination contains the opponent king.
*   Legal move generation should instead recognize checkmate based on no legal responses.

***

### Bug/Risk 3: No castling support

Kings and rooks can never castle.

**Impact**

*   search tree is incomplete,
*   legal move counts are wrong,
*   many real positions are mis-modeled.

***

### Bug/Risk 4: No promotion support

A pawn reaching the last rank remains a pawn or can move illegally depending on subsequent logic.

**Failure scenario**

*   White pawn moves from 6th to 7th rank and then to 8th.
*   No promotion choice appears.
*   Position becomes illegal.

***

### Bug/Risk 5: No en passant support

Engine will reject legal en passant moves and cannot represent en passant targets.

**Impact**

*   move generation incorrect,
*   search tree incomplete,
*   certain tactical lines impossible.

***

### Bug/Risk 6: No castling-rights state

Even if you later add castling movement, current board state does not track whether:

*   king moved,
*   rooks moved,
*   rook was captured.

So castling legality cannot be determined from current state alone.

***

### Bug/Risk 7: Pointer-based piece typing is fragile

`moveLegalityCheck` uses `piece == &WP`, etc.

**Failure scenario**
If you later:

*   refactor piece access,
*   pass a copied bitboard,
*   or store piece references differently,

then move validation silently breaks.

**Fix**

*   use `Move.piece` or `pieceTypeAt(from)`.

***

### Bug/Risk 8: `makeMove` relies on global re-scan after mutation

It computes:

```cpp
int kingSq = isWhite ? getPiecePosition(WK): getPiecePosition(BK);
```

This is okay now, but inefficient and potentially fragile.

**Risk**

*   if king bitboard gets corrupted, search behaves unpredictably.
*   there is no assertion that each side has exactly one king.

***

### Bug/Risk 9: No invariant validation

Nothing checks:

*   overlapping pieces,
*   both kings present,
*   exactly one king per side,
*   side-to-move consistency.

These are crucial in a search engine.

***

### Bug/Risk 10: `getPiecePosition()` scans linearly

Not wrong, but hot-path inefficient if used often.

**Fix later**

*   `std::countr_zero(bitboard)` for non-zero bitboards
*   or maintain explicit king square fields

***

### Bug/Risk 11: Unused variable in `isSquareAttacked`

```cpp
uint64_t attackers=byWhite?GetWhitePieces():GetBlackPieces();
```

Unused.

Not a correctness bug, but indicates incomplete/refactored logic.

***

### Bug/Risk 12: Board printing and indexing conventions are implicit

The engine assumes a specific square mapping but does not expose conversion utilities.  
That will cause bugs when adding:

*   move parsing,
*   FEN,
*   debugging tools.

***

## 4.2 Edge cases not handled

These must be addressed before search.

1.  **Check**
2.  **Double check**
3.  **Pinned pieces**
4.  **Checkmate**
5.  **Stalemate**
6.  **Castling out of check**
7.  **Castling through attacked squares**
8.  **Castling after rook/king moved**
9.  **En passant exposing own king**
10. **Promotion choices**
11. **Insufficient material**
12. **Threefold repetition**
13. **50-move rule**
14. **Illegal positions from king capture**
15. **Adjacent kings**

Some are mandatory immediately, some later:

*   **mandatory now:** check, double check, checkmate, stalemate, castling, promotion, en passant.
*   **later:** repetition, 50-move rule, insufficient material.

***

## 4.3 Performance bottlenecks in the current code

***

### 1. `getPieceBitboardAtSquare()` scans 12 bitboards

Called for capture resolution.

**Cost:** O(12) per lookup  
Fine now, but grows expensive in search.

***

### 2. `getPiecePosition()` scans 64 squares

Used for king lookup.

**Cost:** O(64)  
Can be replaced with:

*   explicit king square fields, or
*   `countr_zero`.

***

### 3. `isSquareAttacked()` is ray-scanning based

Repeatedly called during legality checking.

In search, this becomes hot.

***

### 4. No move generation cache or direct iteration over set bits

When you implement move generation, iterating all 64 squares naively will be slower than scanning set bits from piece bitboards.

Preferred pattern:

```cpp
while (bitboard) {
    int sq = std::countr_zero(bitboard);
    bitboard &= bitboard - 1;
}
```

***

## 4.4 Concrete failure scenarios

Here are examples you should test explicitly.

***

### Scenario 1: Illegal king capture

**Position:** White queen attacks black king directly.  
**Expected:** move should not be represented as king capture; engine should detect check/checkmate instead.  
**Current likely behavior:** king can be removed.

***

### Scenario 2: Missing promotion

**Position:** white pawn on `e7`, empty `e8`.  
**Expected:** moves `e7e8q`, `e7e8r`, `e7e8b`, `e7e8n`.  
**Current behavior:** not supported.

***

### Scenario 3: Castling through check

**Position:** white king e1, rook h1, black bishop attacking f1.  
**Expected:** white cannot castle kingside.  
**Current behavior:** castling not supported at all.

***

### Scenario 4: En passant discovered self-check

**Position:** en passant capture opens rook attack on own king.  
**Expected:** en passant should be illegal.  
**Current behavior:** en passant not supported.

***

### Scenario 5: Stalemate

**Position:** side to move has no legal moves but is not in check.  
**Expected:** draw.  
**Current behavior:** cannot detect because no legal move generator.

***

### Scenario 6: Pinned piece movement

**Position:** knight pinned to king by rook/bishop.  
**Expected:** pseudo-legal move exists, legal move does not.  
**Current behavior:** move legality is only enforced if caller tries a specific move; no legal generator exists to filter all.

***

## 4.5 Suggested test cases (unit + integration)

***

## Unit tests

### Board state

*   `ResetBoard()` initializes exactly 32 pieces
*   white occupancy count = 16
*   black occupancy count = 16
*   both kings exist

### Attack detection

*   pawn attacks on edge files
*   knight attacks from center and corner
*   bishop rook queen rays with blockers
*   adjacent king attack

### Move legality

*   each piece movement pattern
*   illegal self-capture
*   slider obstruction
*   king cannot move adjacent to enemy king
*   pinned-piece move rejected by `makeMove`

### Special rules

*   castling legal
*   castling illegal due to attack
*   promotion generation
*   en passant legal
*   en passant illegal due to self-check

### Make/unmake

*   after make+unmake, board is identical
*   repeat for all moves in a position

***

## Integration tests

### Perft

Run standard perft positions:

*   starting position depth 1,2,3,4
*   castling-rich positions
*   en passant positions
*   promotion positions

### Search sanity

*   mate in 1
*   forced capture test
*   avoid hanging queen at depth 2+
*   stalemate avoidance

### FEN-based regression

Store tricky positions and ensure:

*   legal move count stable
*   best move stable at fixed depth
*   no crashes

***

# SECTION 5: PERFORMANCE & SCALABILITY

***

## 5.1 How to optimize Minimax effectively

Here are the optimizations that matter most for your project.

***

## Must-have optimizations

### 1. Alpha-beta pruning

**Impact:** extremely high  
**Difficulty:** medium  
**Reason:** biggest search reduction for modest complexity.

***

### 2. Move ordering

**Impact:** extremely high (because alpha-beta depends on it)  
**Difficulty:** medium  
**Start with:** promotions, captures, checks.

***

### 3. Make/unmake instead of copying full board recursively

**Impact:** high  
**Difficulty:** medium  
**Note:** full-state undo is okay first; avoid cloning whole board at each node.

***

### 4. Quiescence search

**Impact:** high on tactical stability  
**Difficulty:** medium  
**Reason:** avoids horizon effect by extending capture sequences.

***

### 5. Efficient bit iteration

Use bit scans instead of scanning all 64 squares.

**Impact:** medium-high  
**Difficulty:** easy

***

## Nice-to-have optimizations

### 6. Transposition table

**Impact:** very high  
**Difficulty:** hard  
**Reason:** avoids re-searching repeated positions.

Requires:

*   Zobrist hashing
*   bound storage
*   replacement policy

***

### 7. Iterative deepening

**Impact:** high  
**Difficulty:** medium  
**Reason:** better move ordering, time control, anytime search.

***

### 8. Killer/history heuristics

**Impact:** medium-high  
**Difficulty:** medium  
**Reason:** improves quiet move ordering.

***

### 9. Incremental evaluation

**Impact:** medium  
**Difficulty:** hard  
**Reason:** faster but more bug-prone.

***

### 10. Magic bitboards / attack tables

**Impact:** medium-high  
**Difficulty:** hard  
**Reason:** faster sliders, but implementation complexity rises.

For your personal project, postpone this.

***

## 5.2 Memory considerations

***

### Current state

Your board state itself is tiny (12 x 64-bit bitboards + metadata).  
Memory is not the issue now; correctness is.

### When memory matters

Once you add:

*   transposition table,
*   undo stack,
*   iterative deepening history,
    memory becomes relevant.

### Practical guidance

*   `UndoState` full snapshots are okay initially.
*   Do not optimize undo memory prematurely.
*   If you add a TT:
    *   start small (e.g. a few MB),
    *   store hash, depth, score, flag, best move.

***

## 5.3 Future improvements

Here is the pragmatic roadmap beyond the initial engine.

### Strong near-term improvements

1.  Quiescence search
2.  Iterative deepening
3.  Transposition table
4.  Killer/history heuristics
5.  FEN support
6.  Perft harness

### Later / advanced

1.  Null-move pruning
2.  Late move reductions
3.  Principal variation search
4.  Static exchange evaluation
5.  tapered eval
6.  NNUE (far later, not recommended now)

***

## 5.4 Rank optimizations by impact vs implementation difficulty

***

### Tier A — Must-have

| Optimization                   |    Impact |  Difficulty | Recommendation |
| ------------------------------ | --------: | ----------: | -------------- |
| Legal move generation          |  Critical |        Hard | Implement now  |
| Make/unmake                    |  Critical |      Medium | Implement now  |
| Alpha-beta pruning             | Very high |      Medium | Implement now  |
| Basic move ordering            | Very high |      Medium | Implement now  |
| Perft validation               |  Critical | Easy/Medium | Implement now  |
| Promotions/castling/en passant |  Critical |      Medium | Implement now  |

***

### Tier B — Strongly recommended

| Optimization         |      Impact | Difficulty | Recommendation      |
| -------------------- | ----------: | ---------: | ------------------- |
| Quiescence search    |        High |     Medium | After alpha-beta    |
| Iterative deepening  |        High |     Medium | After stable search |
| Bit-scan iteration   | Medium/High |       Easy | Early               |
| Basic PST evaluation |        High |       Easy | Early               |

***

### Tier C — Nice-to-have

| Optimization              |      Impact | Difficulty | Recommendation         |
| ------------------------- | ----------: | ---------: | ---------------------- |
| Transposition table       |   Very high |       Hard | After engine is stable |
| Killer/history heuristics | Medium/High |     Medium | After alpha-beta       |
| Incremental eval          |      Medium |       Hard | Later                  |
| Magic bitboards           | Medium/High |       Hard | Later                  |

***

# SECTION 6: FINAL ACTION PLAN

***

## 6.1 Prioritized checklist

This is the practical sequence I would recommend for your personal project.

***

### Priority 1 — Fix the architecture before search

**Difficulty:** Medium  
**Critical path:** Yes

*   [ ] Introduce `Move` struct
*   [ ] Introduce `UndoState`
*   [ ] Add board metadata:
    *   [ ] castling rights
    *   [ ] en passant square
    *   [ ] halfmove clock
    *   [ ] fullmove number
*   [ ] Replace pointer-identity-based move typing with explicit piece typing

**Why first:** everything else depends on this.

***

### Priority 2 — Implement complete legal move generation

**Difficulty:** Hard  
**Critical path:** Yes

*   [ ] Generate pseudo-legal moves for all pieces
*   [ ] Add promotions
*   [ ] Add castling
*   [ ] Add en passant
*   [ ] Filter pseudo-legal to legal with make/unmake + king safety
*   [ ] Implement `isInCheck()`
*   [ ] Implement checkmate/stalemate detection

**Why now:** Minimax cannot start until this is correct.

***

### Priority 3 — Build perft and correctness tests

**Difficulty:** Medium  
**Critical path:** Yes

*   [ ] Implement `perft(depth)`
*   [ ] Validate starting position counts
*   [ ] Add tricky rule positions
*   [ ] Add make/unmake round-trip tests
*   [ ] Add board invariant assertions

**Why now:** this is your safety net before search.

***

### Priority 4 — Implement evaluation

**Difficulty:** Easy/Medium  
**Critical path:** Yes

*   [ ] Material scoring
*   [ ] Piece-square tables
*   [ ] Simple mobility
*   [ ] Terminal score conventions

**Why now:** leaf nodes need scoring.

***

### Priority 5 — Implement Negamax + Alpha-Beta

**Difficulty:** Medium  
**Critical path:** Yes

*   [ ] `findBestMove(depth)`
*   [ ] `negamax(depth, alpha, beta)`
*   [ ] mate/stalemate handling
*   [ ] node counter
*   [ ] root best move tracking

**Why now:** this creates the first playable engine.

***

### Priority 6 — Add move ordering

**Difficulty:** Medium  
**Critical path:** Strongly recommended

*   [ ] promotions first
*   [ ] MVV-LVA captures
*   [ ] check-giving moves
*   [ ] optionally killer/history later

**Why:** huge search gain for small complexity.

***

### Priority 7 — Add quiescence search

**Difficulty:** Medium  
**Critical path:** No, but high value

*   [ ] stand-pat evaluation
*   [ ] capture-only extensions
*   [ ] alpha-beta in quiescence

**Why:** reduces tactical blindness.

***

### Priority 8 — Make it usable

**Difficulty:** Easy/Medium  
**Critical path:** No

*   [ ] parse moves like `e2e4`
*   [ ] print move strings
*   [ ] FEN import/export
*   [ ] engine-vs-human loop

***

### Priority 9 — Add advanced performance features

**Difficulty:** Hard  
**Critical path:** No

*   [ ] iterative deepening
*   [ ] transposition table
*   [ ] Zobrist hashing
*   [ ] killer/history heuristics

***

## 6.2 Critical path items

These are the tasks that block everything else:

1.  **Move struct + board metadata**
2.  **Complete legal move generation**
3.  **Make/unmake**
4.  **Perft/testing**
5.  **Evaluation**
6.  **Negamax + alpha-beta**

If these are wrong, every later optimization is wasted effort.

***

## 6.3 Difficulty estimates

### Easy

*   material evaluation
*   bit-scan iteration
*   board invariant checks
*   move string formatting
*   node counting

### Medium

*   `Move` / `UndoState`
*   basic make/unmake
*   alpha-beta negamax
*   move ordering
*   PST evaluation
*   game loop
*   FEN I/O

### Hard

*   full legal move generation
*   castling / en passant correctness
*   quiescence search
*   transposition table
*   repetition handling
*   advanced pruning/reductions

***

# Final Engineering Judgment

Your current code is **not far away from being a solid engine core**, but it is currently at the **“position rules prototype”** stage, not the “search engine” stage.

If I were reviewing this as an architecture lead, my strongest recommendation would be:

> **Do not implement Minimax yet.**
>
> First finish:
>
> 1.  full legal move generation,
> 2.  complete position state,
> 3.  make/unmake,
> 4.  perft.

That sequence gives you a correct foundation.  
Only then add:

*   evaluation,
*   Negamax + alpha-beta,
*   move ordering.

If you do that in order, you can realistically build a personal chess engine that is:

*   correct,
*   understandable,
*   decently efficient,
*   extensible without major rewrites.

***

If you want, I can next produce one of these three artifacts:

1.  **A proposed refactored class design in C++** (`Board`, `Move`, `UndoState`, `MoveGenerator`, `Search`)
2.  **A concrete implementation plan for `generateLegalMoves()`** based on your exact bitboard layout
3.  **A code rewrite of your current class skeleton** into an engine-ready architecture

If you want the most useful next step, I recommend **#3**.
