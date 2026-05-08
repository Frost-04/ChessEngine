#pragma once
#include "board.h"

class MoveGenerator {
public:
    static bool nextValidMoves(Board& board, bool justCheck = false);
    static bool hasValidMoves(Board& board);
};