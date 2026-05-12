#pragma once
#include "constants.h"
#include "structs.h"
#include <cstdint>
#include <stack>
#include <vector>
#include <unordered_map>
class Board {
public:
    U64 WP, WN, WB, WR, WQ, WK;
    U64 BP, BN, BB, BR, BQ, BK;
    U64 NULL_PIECE;
    U8 enPassantSquare;
    Piece pieceAt[64];

    bool whiteToMove;
    bool whiteCastleKingSide;
    bool whiteCastleQueenSide;
    bool blackCastleKingSide;
    bool blackCastleQueenSide;

    bool blackInCheck;
    bool whiteInCheck;
    U8 fiftyMoveClock;
    int moveCount;
    bool isInsufficientMaterial;

    U64 zorbistHash;
    std::unordered_map<U64,int> positionHistory;
    bool trackHistory; //just for 

    std::stack<Undo> undoStack;    

    Board() { ResetBoard(); }   //generator
    void ResetBoard();
    void initializePieceTable();
    inline U64 GetWhitePieces() { return (WP|WR|WN|WB|WQ|WK); }
    inline U64 GetBlackPieces() { return (BP|BR|BN|BB|BQ|BK); }
    inline U64 GetAllPieces() { return GetWhitePieces() | GetBlackPieces(); }
    inline bool GetBit(U64 piece, int i) { return i>=0 && i < 64 && ((piece >> i) & 1ULL); }
    inline void SetBit(U64 &piece, int i) { piece|=(1ULL<<i); }
    inline void ResetBit(U64 &piece, int i) { piece&= ~(1ULL << i); }
    inline bool hasFlag(U8 flags, U8 flag) { return (flags&flag); }
    inline void setFlag(U8 &flags, U8 flag) { flags=flags|flag; }
    void saveUndoState();
    void undoMove(const Move &move);
    void restoreState(const Move &move);
    GameResult hasGameEnded();
    U64 computeInitialHash();
    void updateHash(const Move &move);


    int getPiecePosition(U64 piece);
    std::vector<int> getPieceAllPosition(Piece pieceId);
    U64& getPieceBB(Piece piece);
    bool isSquareAttacked(int sq, bool byWhite);
    bool isPathClear(int from, int to);
    bool moveLegalityCheck(Move move);
    bool makeMove(Move move);
    void printBoard();
};