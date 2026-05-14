#pragma once
#include "board.h"

class Evaluator {
public:
    static int evaluate(Board& board);
private:
    static int materialScore(Board& board);
    static int pieceSquareScore(Board& board);
};