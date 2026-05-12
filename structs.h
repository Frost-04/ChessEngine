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
    int count=0;
    MoveList()
    {
        count=0;
    }
    Move *begin()
    {
        return moveArray;
    }
    Move *end()
    {
        return moveArray + count;
    }
    const Move *begin() const
    {
        return moveArray;
    }
    const Move *end() const
    {
        return moveArray + count;
    }
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
        count=0;
    }
    void push_back(const Move &move)
    {
        moveArray[count++]=move;
    }
    void pop_back()
    {
        count--;
    }
    bool empty()
    {
        return count<=0;
    }
};