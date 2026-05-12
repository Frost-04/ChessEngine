#pragma once
#include "board.h"

class MoveGenerator {
public:
    static bool nextValidMoves(Board& board, MoveList &moves);
    static bool hasValidMoves(Board& board);
};