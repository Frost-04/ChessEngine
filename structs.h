#pragma once
#include "constants.h"
#include <cstdint>

enum class Piece : U8 {
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK,
    None
};

enum class GameResult : U8 {
    Ongoing,
    WhiteCheckmate,
    BlackCheckmate,
    Stalemate,
    Draw50Move,
    DrawInsufficientMaterial,
    DrawThreefoldRepetition
};

struct Move
{
    Piece movingPiece;
    Piece capturedPiece;
    int from;
    int to;
    U8 flag;
    Piece promotionPiece;
};

struct Undo
{
    U64 WP, WN, WB, WR, WQ, WK;
    U64 BP, BN, BB, BR, BQ, BK;

    U8 enPassantSquare;
    Piece pieceAt[64];

    bool blackInCheck;
    bool whiteInCheck;
    U8 fiftyMoveClock;
    int moveCount;
    bool isInsufficientMaterial;

    bool whiteToMove;
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    bool blackCastleKingSide;
    bool blackCastleQueenSide;

    U64 zorbistHash;
};