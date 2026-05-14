// evaluator.cpp
#include "evaluator.h"
#include <bit>

// Centipawn values
static constexpr int PAWN_VAL   = 100;
static constexpr int KNIGHT_VAL = 320;
static constexpr int BISHOP_VAL = 330;
static constexpr int ROOK_VAL   = 500;
static constexpr int QUEEN_VAL  = 900;

// Piece-square tables (from White's perspective, A1=index 0)
// These are the well-tested "PeSTO" midgame values, flattened
static constexpr int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};
static constexpr int PST_KNIGHT[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};
static constexpr int PST_BISHOP[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};
static constexpr int PST_ROOK[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};
static constexpr int PST_QUEEN[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
     0,  0,  5,  5,  5,  5,  0, -5,
   -10,  5,  5,  5,  5,  5,  0,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};
static constexpr int PST_KING_MG[64] = {
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

// Mirror a white-perspective square to black's perspective
static inline int mirror(int sq) { return (7 - sq / 8) * 8 + (sq % 8); }

// Score one side's pieces using material + PST
static int scoreSide(U64 pawns, U64 knights, U64 bishops,
                     U64 rooks, U64 queens, U64 king, bool isWhite)
{
    int score = 0;
    auto accumulate = [&](U64 bb, int val, const int* pst) {
        while (bb) {
            int sq = std::countr_zero(bb); bb &= bb - 1;
            score += val;
            score += pst[isWhite ? sq : mirror(sq)];
        }
    };
    accumulate(pawns,   PAWN_VAL,   PST_PAWN);
    accumulate(knights, KNIGHT_VAL, PST_KNIGHT);
    accumulate(bishops, BISHOP_VAL, PST_BISHOP);
    accumulate(rooks,   ROOK_VAL,   PST_ROOK);
    accumulate(queens,  QUEEN_VAL,  PST_QUEEN);
    // King uses its own PST (no material value)
    if (king) {
        int sq = std::countr_zero(king);
        score += PST_KING_MG[isWhite ? sq : mirror(sq)];
    }
    return score;
}

int Evaluator::evaluate(Board& board)
{
    int white = scoreSide(board.WP, board.WN, board.WB,
                          board.WR, board.WQ, board.WK, true);
    int black = scoreSide(board.BP, board.BN, board.BB,
                          board.BR, board.BQ, board.BK, false);
    int score = white - black;
    // Return from the perspective of the side to move (what negamax expects)
    return board.whiteToMove ? score : -score;
}