#pragma once

#include "structs.h"
#include <string>
#include <string_view>

class Board;

class AlgebraicParser {
public:
    static bool parse(Board& board, std::string_view text, Move& outMove, std::string* errorMessage = nullptr);
};
