# Chess Engine Status and Architecture Review

Date: 2026-03-19

## 1. Executive Summary

Your project has a solid bitboard-based foundation and already includes meaningful core logic:
- Board state in 12 piece bitboards.
- Initial position setup.
- Piece-level move legality checks for normal piece movement.
- Self-check prevention in `makeMove`.
- Attack detection used for king-safety validation.

What is still missing is the full "engine loop":
- Move generation API (list of legal moves),
- Evaluation function,
- Minimax/alpha-beta search,
- Game loop + move parsing,
- Special chess rules + terminal state handling.

Current maturity estimate:
- Board/state layer: 55%
- Rules/move system: 45%
- Search/evaluation layer: 0%
- UCI/CLI/game loop layer: 10%
- Test infrastructure: 0%
- Overall toward a playable minimax engine: about 30%

## 2. What Is Done (Verified from Code)

### 2.1 Board Representation
Implemented in `chess.cpp`:
- 12 bitboards:
  - White: `WP, WN, WB, WR, WQ, WK`
  - Black: `BP, BN, BB, BR, BQ, BK`
- Aggregates:
  - `GetWhitePieces()`
  - `GetBlackPieces()`
  - `GetAllPieces()`
- Side to move via `whiteToMove`.

Implemented in `constants.h`:
- Square bit masks `A1..H8` (`uint64_t` literal macros).

Assessment:
- Correct direction for performance-oriented engine design.
- Good baseline for future move generation and search.

### 2.2 Initialization
Implemented:
- `ResetBoard()` sets standard initial chess position.
- `whiteToMove = 1` at game start.

Assessment:
- Meets requirement for starting from classical opening position.
- No support yet for loading arbitrary positions (FEN), which is needed for testing/search benchmarking.

### 2.3 Bit Utilities and Piece Lookup
Implemented:
- `GetBit`, `SetBit`, `ResetBit`.
- `getPiecePosition(uint64_t piece)` to find first set bit.
- `getPieceBitboardAtSquare(int i)` returns bitboard pointer on square.

Assessment:
- Functional for current scale.
- `getPieceBitboardAtSquare` is simple and readable, but pointer-return style should later be replaced with typed piece representation to reduce bug risk.

### 2.4 Attack Detection
Implemented:
- `isSquareAttacked(int sq, bool byWhite)` checks:
  - pawn attacks,
  - knight attacks,
  - sliding rook/bishop/queen rays,
  - adjacent king attacks.

Assessment:
- Very important milestone already done.
- Enables legal move validation by king safety check.
- This function will be reused heavily by legal move generation and checkmate/stalemate detection.

### 2.5 Piece Movement and Legality Rules
Implemented:
- `moveLegalityCheck(uint64_t* piece, int from, int to)` includes:
  - board bounds,
  - own-piece collision rejection,
  - piece movement geometry:
    - pawn single/double pushes and captures,
    - knight L-moves,
    - bishop/rook/queen with obstruction checks,
    - king one-square moves.
- `isMoveObstructed(from, to)` for sliding move path blocking.

Assessment:
- Good first legal-move validation core.
- Special rules intentionally missing (expected at this stage).

### 2.6 Move Application with King Safety
Implemented:
- `makeMove(uint64_t* piece, int fromIdx, int toIdx)`:
  - validates move geometry,
  - enforces side-to-move,
  - applies capture + movement,
  - checks own king safety (`isSquareAttacked`),
  - reverts on illegal self-check,
  - toggles side on success.

Assessment:
- This is an advanced step for a first engine.
- You already do legality verification beyond pure pseudo-legal movement.

### 2.7 Board Rendering
Implemented:
- `printBoard()` prints piece characters on 8x8 grid.

Assessment:
- Helpful for debugging.
- There is one compile issue currently (see Section 4.1).

## 3. What Is Left (Detailed)

## 3.1 Mandatory for a Real Chess Engine

### A. Move List Generation API
Missing:
- A function that returns all legal moves for side to move.

Needed now:
- `std::vector<Move> generatePseudoLegalMoves() const`
- `std::vector<Move> generateLegalMoves()`

Why critical:
- Minimax needs a move list at every node.

### B. Move Object and Flags
Missing:
- Dedicated move type with encoded metadata.

Needed now:
- `struct Move { uint8_t from, to, piece, captured, promotion; uint8_t flags; };`
- Flags for capture, promotion, en passant, castling, double pawn push.

Why critical:
- Search and undo/redo need deterministic move semantics.

### C. Full Chess Rules
Missing:
- Castling rights tracking and castling move legality.
- En passant target + capture legality.
- Promotion generation/application.
- Halfmove clock and fullmove number.

Why critical:
- Without these rules, search output can be illegal or strategically wrong.

### D. Endgame Terminal Detection
Missing:
- Checkmate detection.
- Stalemate detection.
- Draw conditions:
  - insufficient material,
  - fifty-move rule,
  - threefold repetition.

Why critical:
- Search score correctness depends on terminal-node handling.

### E. Evaluation
Missing:
- Any position scoring function.

Needed first version:
- Material score only.

Needed second version:
- Piece-square tables,
- pawn structure,
- mobility,
- king safety.

### F. Search
Missing:
- Minimax/Negamax recursion.
- Alpha-beta pruning.
- Root move selection (`findBestMove`).

Needed later for strength:
- Iterative deepening,
- transposition table (Zobrist hash),
- move ordering,
- quiescence search.

### G. Interface / Control Loop
Missing:
- Input parser (`e2e4`, `g1f3`, promotions).
- Turn loop (human vs engine / engine vs engine).
- Outcome messaging.

### H. Testing and Verification
Missing:
- Unit tests.
- Perft tests against known move counts.
- Regression test suite for legality.

Why critical:
- Move generation bugs are common; perft catches them early.

## 3.2 Architecture Gaps

### State completeness gap
Current board state does not yet include:
- castling rights,
- en passant square,
- halfmove clock,
- fullmove number,
- repetition/hash key.

### Mutation model gap
Current move application relies on direct piece pointers and partial rollback.
- Works now, but will become fragile with special rules and deep search.

Recommended:
- explicit `makeMove(Move, UndoState&)` and `unmakeMove(Move, const UndoState&)`.

### Separation of concerns gap
`Board` currently mixes:
- state storage,
- move validation,
- attack detection,
- display.

Recommended:
- split into modules (see Section 5).

## 4. Findings and Risks (Code Review)

### 4.1 Compile Error (High Priority)
File: `chess.cpp`
- `cout` is used without qualification or `using std::cout`.
- Errors reported around print lines in `printBoard()`.

Fix options:
- Replace `cout` with `std::cout`, or
- re-enable `using namespace std;` (not recommended in headers/global style).

### 4.2 Naming/Style Consistency
- Method names mix `ResetBoard` and `getPiecePosition` styles.
- Use one naming convention (e.g., `camelCase` for methods and members).

### 4.3 Safety and Scale Risks
- `getPieceBitboardAtSquare` returning raw mutable pointer is convenient but error-prone as system grows.
- `getPiecePosition` linear scan is fine for king now, but should move to bit tricks later.

### 4.4 Performance Considerations
- Current legality checking via direct state mutation and king attack test is acceptable for baseline.
- Search will require optimized move generation, move ordering, and undo model.

## 5. Recommended Architecture (Software Architect View)

Use a layered architecture with explicit domain types and low coupling.

### 5.1 Proposed Module Layout

- `types.h/.cpp`
  - `enum class Color`, `PieceType`, `Piece`, `MoveFlag`
  - `struct Move`
  - square conversion helpers

- `position.h/.cpp`
  - `struct Position` (all bitboards + side + castling + ep + counters + hash)
  - occupancy cache helpers

- `movegen.h/.cpp`
  - pseudo-legal generation by piece
  - legal filtering
  - attack map helpers

- `makemove.h/.cpp`
  - `makeMove`, `unmakeMove`
  - `UndoState`

- `eval.h/.cpp`
  - `evaluate(const Position&)`
  - start with material, then positional terms

- `search.h/.cpp`
  - `negamax`, `alphaBeta`, `quiescence`
  - iterative deepening and move ordering

- `io.h/.cpp`
  - board printer
  - move parser and formatter
  - FEN parser

- `main.cpp`
  - game loop / command loop

### 5.2 Design Patterns to Apply

1. Strategy Pattern
- Use for evaluation strategy and search strategy.
- Example:
  - `IEvaluator` with `evaluate(Position)`.
  - `MaterialEvaluator`, `TaperedEvaluator`.
- Benefit:
  - swap evaluation models without touching search.

2. Command Pattern (with Undo)
- Treat each move as an executable command with reversible state.
- `makeMove` stores `UndoState`; `unmakeMove` restores.
- Benefit:
  - robust deep tree traversal.

3. Template Method (or policy-based design) for search framework
- Common search skeleton with pluggable components:
  - move ordering policy,
  - pruning policy,
  - evaluation policy.
- Benefit:
  - easier experimentation while keeping one search flow.

4. Factory Method (optional)
- For constructing engine modes:
  - debug engine,
  - fast-play engine,
  - deep-analysis engine.
- Benefit:
  - cleaner runtime configuration.

### 5.3 Data Model You Should Adopt Now

Extend current board into a complete `Position`:
- Piece bitboards: 12
- `Color sideToMove`
- `uint8_t castlingRights` (4 bits)
- `int epSquare` (-1 if none)
- `int halfmoveClock`
- `int fullmoveNumber`
- `uint64_t zobristKey`

Without this, special rules and transposition table integration become hard.

## 6. Suggested Build Order (Practical Roadmap)

### Milestone 1: Stabilize Core State and Move Type
- Introduce `Move` and `Position` structs.
- Add castling/ep/counters to state.
- Replace raw piece pointer move API with typed move API.

### Milestone 2: Complete Legal Move Generation
- Generate pseudo-legal moves for all pieces.
- Add special moves.
- Filter illegal moves by king safety.
- Add perft(1..5) validation.

### Milestone 3: Evaluation v1
- Material score.
- Basic PST (piece-square tables).

### Milestone 4: Search v1
- Negamax + alpha-beta.
- Root best move selection.
- Time/depth control.

### Milestone 5: Search v2 Strength
- Move ordering:
  - captures first,
  - killer/history heuristics.
- Quiescence search.
- Transposition table.
- Iterative deepening.

### Milestone 6: Interface and UX
- CLI loop with move parsing and legality feedback.
- Optional UCI support for GUI compatibility.

## 7. Definition of Done Checklist

Engine MVP is done when all are true:
- Generates all legal moves in normal positions including castling, en passant, promotion.
- Passes perft reference positions (depth 1-5 baseline).
- Detects checkmate and stalemate.
- Evaluates material and basic positional factors.
- Plays complete legal game from start to finish.
- Returns best move from alpha-beta search at fixed depth.

## 8. Immediate Next Actions (Highest ROI)

1. Fix compile issue in `printBoard` (`std::cout`).
2. Introduce `Move` struct and replace pointer-based move API.
3. Add `Position` fields: castling rights, ep square, halfmove/fullmove.
4. Implement `generateLegalMoves()` and perft tests before writing minimax.
5. Add `evaluate()` (material only), then implement negamax alpha-beta.

## 9. Final Assessment

You are past the "toy board printer" stage and already have meaningful legal-move infrastructure. The biggest architectural step now is to formalize position state and move representation. Once that is done, adding minimax/alpha-beta becomes straightforward and maintainable.

If you follow the module split and pattern choices above (Strategy + Command/Undo + layered architecture), you will avoid the most common chess-engine rewrite trap: tightly coupled board/search code that becomes impossible to extend.
