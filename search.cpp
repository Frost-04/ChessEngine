// search.cpp
#include "search.h"
#include "evaluator.h"
#include "movegenerator.h"
#include <algorithm>
#include <climits>

static constexpr int INF      = 1000000;
static constexpr int MATE_VAL = 900000;  // above all material

// MVV-LVA table: [attacker][victim]
// Piece order: P=0 N=1 B=2 R=3 Q=4 K=5  (map from your Piece enum)
static constexpr int MVV_LVA[6][6] = {
    // victim:  P    N    B    R    Q    K
    /* P */  { 105, 205, 305, 405, 505, 605 },
    /* N */  { 104, 204, 304, 404, 504, 604 },
    /* B */  { 103, 203, 303, 403, 503, 603 },
    /* R */  { 102, 202, 302, 402, 502, 602 },
    /* Q */  { 101, 201, 301, 401, 501, 601 },
    /* K */  { 100, 200, 300, 400, 500, 600 },
};

// Map Piece enum to 0-5 index (strip color: WP=0,WN=1... BP=0,BN=1...)
static inline int pieceTypeIndex(Piece p) {
    return static_cast<int>(p) % 6;  // WP=0,WN=1,...WK=5, BP=0,...BK=5
}
bool Search::isRepetition(U64 hash) const {
    // Walk back every 2 plies (same side to move), stop at captures/pawn moves
    // For simplicity now: check all entries in path
    for (int i = historySize - 2; i >= 0; i--) {
        if (historyStack[i] == hash) return true;
    }
    return false;
}
int Search::quiesce(Board& board, int alpha, int beta)
{
    nodes++;
    int standPat = Evaluator::evaluate(board);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    MoveList moves;
    MoveGenerator::nextValidMoves(board, moves);

    for (auto& move : moves) {
        if (!(move.flag & CAPTURE)) continue;
        if (!board.makeMove(move)) continue;

        int score = -quiesce(board, -beta, -alpha);
        board.undoMove(move);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}
// void Search::scoreMoves(MoveList& moves)
// {
//     // We attach scores by sorting — store scores in a parallel array
//     // Actually we'll sort inline using a lambda in negamax. 
//     // This function left as a hook for later (killer moves, history).
// }

// Sort moves: captures first by MVV-LVA, then quiet moves
static void orderMoves(MoveList& moves, Board& board)
{
    // Simple insertion-sort style: stable enough for small move lists
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        bool aCapture = a.flag & CAPTURE;
        bool bCapture = b.flag & CAPTURE;
        if (aCapture && !bCapture) return true;
        if (!aCapture && bCapture) return false;
        if (aCapture && bCapture) {
            int aScore = MVV_LVA[pieceTypeIndex(a.movingPiece)]
                                 [pieceTypeIndex(a.capturedPiece)];
            int bScore = MVV_LVA[pieceTypeIndex(b.movingPiece)]
                                 [pieceTypeIndex(b.capturedPiece)];
            return aScore > bScore;
        }
        return false;
    });
}


int Search::negamax(Board& board, int depth, int alpha, int beta, int ply)
{
    nodes++;

    // --- Repetition detection (draw = 0) ---
    if (ply > 0 && isRepetition(board.zorbistHash))
        return 0;

    // --- Leaf node ---
    if (depth == 0)
        return quiesce(board, alpha, beta);

    MoveList moves;
    MoveGenerator::nextValidMoves(board, moves);

    // Sort: captures first by MVV-LVA
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        bool ac = a.flag & CAPTURE, bc = b.flag & CAPTURE;
        if (ac && !bc) return true;
        if (!ac && bc) return false;
        if (ac && bc) {
            // higher victim value first, lower attacker value first
            return (static_cast<int>(a.capturedPiece) % 6) >
                   (static_cast<int>(b.capturedPiece) % 6);
        }
        return false;
    });

    bool anyLegal = false;
    Move bestLocal{};

    for (auto& move : moves) {
        if (!board.makeMove(move)) continue;
        anyLegal = true;

        historyStack[historySize++] = board.zorbistHash;
        int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        historySize--;

        board.undoMove(move);

        if (score >= beta) return beta;
        if (score > alpha) {
            alpha = score;
            bestLocal = move;
        }
    }

    if (!anyLegal) {
        int kingSq = board.getPiecePosition(
            board.whiteToMove ? board.WK : board.BK);
        bool inCheck = board.isSquareAttacked(kingSq, !board.whiteToMove);
        return inCheck ? -(MATE_VAL - ply) : 0;
    }

    return alpha;
}


SearchResult Search::bestMove(Board& board, int maxDepth)
{
    nodes = 0;
    historySize = 0;

    // Seed history with the actual game path so far
    // (board's positionHistory keys aren't ordered, so we just
    //  start fresh — repetition vs. game history handled by board)
    historyStack[historySize++] = board.zorbistHash;

    SearchResult result{};
    result.score = -INF;

    for (int depth = 1; depth <= maxDepth; depth++) {
        MoveList moves;
        MoveGenerator::nextValidMoves(board, moves);

        int alpha = -INF, beta = INF;
        Move bestAtDepth{};
        int  bestScore = -INF;

        for (auto& move : moves) {
            if (!board.makeMove(move)) continue;

            historyStack[historySize++] = board.zorbistHash;
            int score = -negamax(board, depth - 1, -beta, -alpha, 1);
            historySize--;

            board.undoMove(move);

            if (score > bestScore) {
                bestScore    = score;
                bestAtDepth  = move;
            }
            if (score > alpha) alpha = score;
        }

        result.bestMove = bestAtDepth;
        result.score    = bestScore;
        result.depth    = depth;
        result.nodes    = nodes;
    }

    // Pop the root hash we pushed
    historySize--;
    return result;
}