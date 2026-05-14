#pragma once
#include "board.h"
#include "structs.h"

struct SearchResult {
    Move  bestMove;
    int   score;
    int   depth;
    long long nodes;
};

class Search {
public:
    long long nodes;
    //TranspositionTable tt;  // add later; stub for now

    Search() : nodes(0), historySize(0) {}
    SearchResult bestMove(Board& board, int maxDepth);

private:
    U64 historyStack[512];
    int historySize;

    int  negamax(Board& board, int depth, int alpha, int beta, int ply);
    int  quiesce(Board& board, int alpha, int beta, int ply);
    bool isRepetition(U64 hash, int maxPlies) const;
    
};