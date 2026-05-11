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
    U64 zorbistHash;
    U8 enPassantSquare;
    U8 fiftyMoveClock;

    bool blackInCheck;
    bool whiteInCheck;

    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    bool blackCastleKingSide;
    bool blackCastleQueenSide;
};

struct MoveList{
    Move moveArray[266];
    int top=-1;
    Move &operator[](int index)
    {
        return moveArray[index];
    }
    const Move &operator[](int index) const
    {
        return moveArray[index];
    }
    void clear()
    {
        top=-1;
    }
    void push_back(const Move &move)
    {
        top++;
        moveArray[top]=move;
    }
    void pop_back()
    {
        top--;
    }
};