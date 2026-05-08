#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class Board;

class FenLoader {
public:
    // Returns true and modifies `board` on success.
    // Returns false and leaves `board` unmodified on failure.
    bool load(Board& board, std::string_view fen, std::string* errorMessage = nullptr);

    // Validates FEN format (well-formedness + a few basic sanity checks).
    bool isValid(std::string_view fen, std::string* errorMessage = nullptr);

private:
    struct ParsedFen {
        std::uint64_t WP = 0, WN = 0, WB = 0, WR = 0, WQ = 0, WK = 0;
        std::uint64_t BP = 0, BN = 0, BB = 0, BR = 0, BQ = 0, BK = 0;

        std::uint8_t enPassantSquare = 0;

        bool whiteToMove = true;
        bool whiteCastleKingSide = false;
        bool whiteCastleQueenSide = false;
        bool blackCastleKingSide = false;
        bool blackCastleQueenSide = false;

        std::uint8_t fiftyMoveClock = 0;
        int moveCount = 1;
    };

    bool parse(std::string_view fen, ParsedFen& out, std::string* errorMessage);
    void applyToBoard(Board& board, const ParsedFen& parsed);

    bool parseUnsignedInt(std::string_view text, int& out);
    void setError(std::string* errorMessage, const std::string& message);
    bool addPieceBit(ParsedFen& out, char piece, int squareIndex);
};
