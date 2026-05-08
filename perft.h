#pragma once
#include "board.h"
#include "movegenerator.h"

class Perft{
public:
    static long long run(Board &board, int depth);
    static void divide(Board &board, int depth);
};