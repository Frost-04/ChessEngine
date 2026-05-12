#include "board.h"
#include "structs.h"
#include "constants.h"
#include "movegenerator.h"
#include "zorbist.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <bit>

void Board::ResetBoard()
{
    whiteToMove=1;
    whiteCastleKingSide=1;
    whiteCastleQueenSide=1;
    blackCastleKingSide=1;
    blackCastleQueenSide=1;

    blackInCheck=0;
    whiteInCheck=0;
    moveCount=1;
    fiftyMoveClock=0;
    isInsufficientMaterial=0;

    undoStack= std::stack<Undo>();

    enPassantSquare=NO_SQUARE;

    WP = A2 | B2 | C2 | D2 | E2 | F2 | G2 | H2;
    WR = A1 | H1;
    WN = B1 | G1;
    WB = C1 | F1;
    WQ = D1;
    WK = E1;

    BP = A7 | B7 | C7 | D7 | E7 | F7 | G7 | H7;
    BR = A8 | H8;
    BN = B8 | G8;
    BB = C8 | F8;
    BQ = D8;
    BK = E8;
    NULL_PIECE=0;

    initializePieceTable();//should be just after bitboards have been declared and properly initialized.

    zorbistHash=computeInitialHash();

    positionHistory.clear();
    positionHistory[zorbistHash]++;
    trackHistory=1;
}

void Board::initializePieceTable()
{
    for(int i=0;i<64;i++)
    {
        U64 mask=(1ULL<<i);
        if(WP & mask)  pieceAt[i]=Piece::WP;
        else if(WK & mask) pieceAt[i]=Piece::WK;
        else if(WQ & mask) pieceAt[i]=Piece::WQ;
        else if(WR & mask) pieceAt[i]=Piece::WR;
        else if(WN & mask) pieceAt[i]=Piece::WN;
        else if(WB & mask) pieceAt[i]=Piece::WB;
        else if(BK & mask) pieceAt[i]=Piece::BK;
        else if(BP & mask) pieceAt[i]=Piece::BP;
        else if(BQ & mask) pieceAt[i]=Piece::BQ;
        else if(BR & mask) pieceAt[i]=Piece::BR;
        else if(BN & mask) pieceAt[i]=Piece::BN;
        else if(BB & mask) pieceAt[i]=Piece::BB;
        else pieceAt[i]=Piece::None;
    }
}

void Board::saveUndoState()
{
    Undo undo;

    undo.enPassantSquare=enPassantSquare;

    undo.blackInCheck=blackInCheck;
    undo.whiteInCheck=whiteInCheck;
    undo.fiftyMoveClock=fiftyMoveClock;

    undo.whiteCastleKingSide = whiteCastleKingSide;
    undo.whiteCastleQueenSide = whiteCastleQueenSide;
    undo.blackCastleKingSide = blackCastleKingSide;
    undo.blackCastleQueenSide = blackCastleQueenSide;

    undo.zorbistHash=zorbistHash;

    undoStack.push(undo);
}

void Board::undoMove(const Move &move)
{
    if(undoStack.empty())
        return;
    Undo undo = undoStack.top();
    undoStack.pop();
    enPassantSquare   = undo.enPassantSquare;
    fiftyMoveClock    = undo.fiftyMoveClock;
    whiteCastleKingSide  = undo.whiteCastleKingSide;
    whiteCastleQueenSide = undo.whiteCastleQueenSide;
    blackCastleKingSide  = undo.blackCastleKingSide;
    blackCastleQueenSide = undo.blackCastleQueenSide;

    // 2. Flip side back (undo happens before the flip in makeMove)
    whiteToMove = !whiteToMove;
    if (!whiteToMove) moveCount--;   // black's move is being undone

    // 3. Move the piece back
    U64& movingBB = getPieceBB(move.movingPiece);
    ResetBit(movingBB, move.to);
    SetBit(movingBB,   move.from);
    pieceAt[move.to]   = Piece::None;
    pieceAt[move.from] = move.movingPiece;

    // 4. Undo promotion (replace promoted piece with pawn)
    if (hasFlag(move.flag, PROMOTION)) {
        ResetBit(getPieceBB(move.promotionPiece), move.to);
        // movingBB (the pawn BB) was already set to from above — correct
    }

    // 5. Restore captured piece
    if (hasFlag(move.flag, CAPTURE) && !hasFlag(move.flag, EN_PASSANT)) {
        SetBit(getPieceBB(move.capturedPiece), move.to);
        pieceAt[move.to] = move.capturedPiece;
    }

    // 6. Undo en passant capture
    if (hasFlag(move.flag, EN_PASSANT)) {
        int capturedSq = whiteToMove ? (move.to - 8) : (move.to + 8);
        U64& capturedPawnBB = whiteToMove ? BP : WP;
        Piece capturedPawn  = whiteToMove ? Piece::BP : Piece::WP;
        SetBit(capturedPawnBB, capturedSq);
        pieceAt[capturedSq] = capturedPawn;
    }

    // 7. Undo castling rook move
    if (hasFlag(move.flag, CASTLE_KING)) {
        if (whiteToMove) {
            ResetBit(WR, IDX_F1); SetBit(WR, IDX_H1);
            pieceAt[IDX_F1] = Piece::None; pieceAt[IDX_H1] = Piece::WR;
        } else {
            ResetBit(BR, IDX_F8); SetBit(BR, IDX_H8);
            pieceAt[IDX_F8] = Piece::None; pieceAt[IDX_H8] = Piece::BR;
        }
    }
    if (hasFlag(move.flag, CASTLE_QUEEN)) {
        if (whiteToMove) {
            ResetBit(WR, IDX_D1); SetBit(WR, IDX_A1);
            pieceAt[IDX_D1] = Piece::None; pieceAt[IDX_A1] = Piece::WR;
        } else {
            ResetBit(BR, IDX_D8); SetBit(BR, IDX_A8);
            pieceAt[IDX_D8] = Piece::None; pieceAt[IDX_A8] = Piece::BR;
        }
    }

    // 8. Restore hash
    if (trackHistory) positionHistory[zorbistHash]--;
    zorbistHash = undo.zorbistHash;

}
void Board::restoreState(const Move &move)  //same as undoMove but does not touch positionHistory
{
   if(undoStack.empty())
        return;
    Undo undo = undoStack.top();
    undoStack.pop();
     enPassantSquare   = undo.enPassantSquare;
    fiftyMoveClock    = undo.fiftyMoveClock;
    whiteCastleKingSide  = undo.whiteCastleKingSide;
    whiteCastleQueenSide = undo.whiteCastleQueenSide;
    blackCastleKingSide  = undo.blackCastleKingSide;
    blackCastleQueenSide = undo.blackCastleQueenSide;

    // 3. Move the piece back
    U64& movingBB = getPieceBB(move.movingPiece);
    ResetBit(movingBB, move.to);
    SetBit(movingBB,   move.from);
    pieceAt[move.to]   = Piece::None;
    pieceAt[move.from] = move.movingPiece;

    // 4. Undo promotion (replace promoted piece with pawn)
    if (hasFlag(move.flag, PROMOTION)) {
        ResetBit(getPieceBB(move.promotionPiece), move.to);
        // movingBB (the pawn BB) was already set to from above — correct
    }

    // 5. Restore captured piece
    if (hasFlag(move.flag, CAPTURE) && !hasFlag(move.flag, EN_PASSANT)) {
        SetBit(getPieceBB(move.capturedPiece), move.to);
        pieceAt[move.to] = move.capturedPiece;
    }

    // 6. Undo en passant capture
    if (hasFlag(move.flag, EN_PASSANT)) {
        int capturedSq = whiteToMove ? (move.to - 8) : (move.to + 8);
        U64& capturedPawnBB = whiteToMove ? BP : WP;
        Piece capturedPawn  = whiteToMove ? Piece::BP : Piece::WP;
        SetBit(capturedPawnBB, capturedSq);
        pieceAt[capturedSq] = capturedPawn;
    }

    // 7. Undo castling rook move
    if (hasFlag(move.flag, CASTLE_KING)) {
        if (whiteToMove) {
            ResetBit(WR, IDX_F1); SetBit(WR, IDX_H1);
            pieceAt[IDX_F1] = Piece::None; pieceAt[IDX_H1] = Piece::WR;
        } else {
            ResetBit(BR, IDX_F8); SetBit(BR, IDX_H8);
            pieceAt[IDX_F8] = Piece::None; pieceAt[IDX_H8] = Piece::BR;
        }
    }
    if (hasFlag(move.flag, CASTLE_QUEEN)) {
        if (whiteToMove) {
            ResetBit(WR, IDX_D1); SetBit(WR, IDX_A1);
            pieceAt[IDX_D1] = Piece::None; pieceAt[IDX_A1] = Piece::WR;
        } else {
            ResetBit(BR, IDX_D8); SetBit(BR, IDX_A8);
            pieceAt[IDX_D8] = Piece::None; pieceAt[IDX_A8] = Piece::BR;
        }
    }
    zorbistHash = undo.zorbistHash;
}

GameResult Board::hasGameEnded()    //call this after the side to move has been flipped
{
    if(!MoveGenerator::hasValidMoves(*this))
    {
        if(!whiteToMove && blackInCheck)
            return GameResult::BlackCheckmate;
        else if(whiteToMove && whiteInCheck)
            return GameResult::WhiteCheckmate;
        else
            return GameResult::Stalemate;
    }
    //fifty move counter
    if(fiftyMoveClock>=100)
        return GameResult::Draw50Move;

    //3 fold repetition
    if(trackHistory)
    {
        auto it=positionHistory.find(zorbistHash);
        if(it!=positionHistory.end() && it->second>=3)
            return GameResult::DrawThreefoldRepetition;
    }
    //calculating insufficient material
    if(WQ==0 && WR==0 && WP==0 && BQ==0 && BR==0 && BP==0)
    {
        if(__builtin_popcountll(WB)+__builtin_popcountll(WN)<=1 && __builtin_popcountll(BN)+__builtin_popcountll(BB)<=1)
            return GameResult::DrawInsufficientMaterial;
    }

    return GameResult::Ongoing;
}

U64 Board::computeInitialHash()
{
    U64 hash=0;
    for(int i=0;i<64;i++)
    {
        Piece p=pieceAt[i];
        if(p!=Piece::None)
            hash^=ZORBIST.pieces[(int)p][i];
    }
    if(!whiteToMove)    //hash only on blacktomove
        hash^=ZORBIST.sideToMove;

    if(whiteCastleKingSide)
        hash^=ZORBIST.castling[0];
    if(whiteCastleQueenSide)
        hash^=ZORBIST.castling[1];
    if(blackCastleKingSide)
        hash^=ZORBIST.castling[2];
    if(blackCastleQueenSide)
        hash^=ZORBIST.castling[3];
    
    if(enPassantSquare!=NO_SQUARE)
        hash^=ZORBIST.enPassantFile[enPassantSquare%8];
    
    return hash;

}

void Board::updateHash(const Move &move)
{
    //for moving piece
    zorbistHash^=ZORBIST.pieces[(int)move.movingPiece][move.from];
    zorbistHash^=ZORBIST.pieces[(int)move.movingPiece][move.to];

    //capture
    if(hasFlag(move.flag,CAPTURE))
        zorbistHash^=ZORBIST.pieces[(int)move.capturedPiece][move.to];
    //enpassant
    if(hasFlag(move.flag,DOUBLE_PAWN))
        zorbistHash^=ZORBIST.enPassantFile[move.to%8];
    if(hasFlag(move.flag,EN_PASSANT))
    {
        if(whiteToMove)
            zorbistHash^=ZORBIST.pieces[(int)move.capturedPiece][move.to-8];
        else
            zorbistHash^=ZORBIST.pieces[(int)move.capturedPiece][move.to+8];
    }

    //castling rights
    if(whiteCastleKingSide)
        zorbistHash^=ZORBIST.castling[0];
    if(whiteCastleQueenSide)
        zorbistHash^=ZORBIST.castling[1];
    if(blackCastleKingSide)
        zorbistHash^=ZORBIST.castling[2];
    if(blackCastleQueenSide)
        zorbistHash^=ZORBIST.castling[3];

    //promotion
    if(hasFlag(move.flag,PROMOTION))
    {
        zorbistHash^=ZORBIST.pieces[(int)move.movingPiece][move.to];    //removal of added pawn on the last rank
        zorbistHash^=ZORBIST.pieces[(int)move.promotionPiece][move.to];
    }

    //rook movement on castling
    if(hasFlag(move.flag,CASTLE_KING))
    {
        if(whiteToMove)
        {
            zorbistHash^=ZORBIST.pieces[(int)Piece::WR][IDX_H1];
            zorbistHash^=ZORBIST.pieces[(int)Piece::WR][IDX_F1];
        }
        else
        {
            zorbistHash^=ZORBIST.pieces[(int)Piece::BR][IDX_H8];
            zorbistHash^=ZORBIST.pieces[(int)Piece::BR][IDX_F8];
        }
    }
    if(hasFlag(move.flag,CASTLE_QUEEN))
    {
        if(whiteToMove)
        {
            zorbistHash^=ZORBIST.pieces[(int)Piece::WR][IDX_A1];
            zorbistHash^=ZORBIST.pieces[(int)Piece::WR][IDX_D1];
        }
        else
        {
            zorbistHash^=ZORBIST.pieces[(int)Piece::BR][IDX_A8];
            zorbistHash^=ZORBIST.pieces[(int)Piece::BR][IDX_D8];
        }
    }
    //side to move
    zorbistHash^=ZORBIST.sideToMove;
}

//returns induex of least signinficant "set" bit in piece,, -1 if nones
int Board::getPiecePosition(U64 piece)
{
    if (piece == 0) return -1;
    return std::countr_zero(piece);
}

std::vector<int> Board::getPieceAllPosition(Piece pieceId)
{
    std::vector<int> positions;
    if (pieceId == Piece::None) return positions;
    U64 piece = getPieceBB(pieceId);
    positions.reserve(static_cast<int>(std::popcount(piece)));
    while (piece) {
        int idx = std::countr_zero(piece);
        positions.push_back(idx);
        piece &= (piece - 1);
    }
    return positions;
}


U64& Board::getPieceBB(Piece piece)
{
    switch (piece)
    {
        case Piece::WP: return WP;
        case Piece::WN: return WN;
        case Piece::WB: return WB;
        case Piece::WR: return WR;
        case Piece::WQ: return WQ;
        case Piece::WK: return WK;

        case Piece::BP: return BP;
        case Piece::BN: return BN;
        case Piece::BB: return BB;
        case Piece::BR: return BR;
        case Piece::BQ: return BQ;
        case Piece::BK: return BK;

        default: return NULL_PIECE;
    }
}



// Returns true if `sq` is attacked by White (byWhite=true) or Black.
// Checks pawn capture squares, knight jumps, sliding rays for rook/bishop/queen
// (stopping at blockers), and adjacent king squares.
bool Board::isSquareAttacked(int sq, bool byWhite)
{
    const int file= sq%8, rank= sq/8;  //making it int instead of U8 because some checks check if value is -ve
    U64 allPieces=GetAllPieces();
    U64 pawns=byWhite?WP:BP;
    U64 knights = byWhite ? WN : BN;
    U64 bishops = byWhite ? WB : BB;
    U64 rooks   = byWhite ? WR : BR;
    U64 queens  = byWhite ? WQ : BQ;
    U64 king    = byWhite ? WK : BK;

    //white pawn
    if(byWhite && rank-1>=0)
    {
        int idx=(rank-1)*8+file;
        if(file+1<=7 && GetBit(pawns, idx+1))
            return true;
        if(file-1>=0 && GetBit(pawns, idx-1))
            return true;
    }
    //black pawn
    else if(!byWhite && rank+1<=7)
    {
        int idx=(rank+1)*8+file;
        if(file+1<=7 && GetBit(pawns, idx+1))
            return true;
        if(file-1>=0 && GetBit(pawns, idx-1))
            return true;
    }

    //Knight

    for(int i=0; i<8; i++)
    {
        int knightRank=rank+knightOffsets[i][0];
        int knightFile=file+knightOffsets[i][1];
        if(knightRank>7 || knightRank<0 || knightFile>7 || knightFile<0)
            continue;

        int idx=knightRank*8 + knightFile;

        if(GetBit(knights, idx))
            return true;
    }

    //Straight movement (Rook and Queen)
    for(int i=0; i<4; i++)
    {
        for(int multiplier=1; multiplier<8; multiplier++)
        {
            int rookRank=rank+rookOffsets[i][0]*multiplier, rookFile=file+rookOffsets[i][1]*multiplier;
            if(rookRank<0 || rookRank>7 || rookFile<0 || rookFile>7)
                break;
            int idx=rookRank*8 + rookFile;
            if(GetBit(rooks, idx) || GetBit(queens, idx))
                return true;
            else if(GetBit(allPieces, idx))
                break;
        }
    }

    //Diagonal movement (Bishop and queen)
    for(int i=0; i<4; i++)
    {
        for(int multiplier=1; multiplier<8; multiplier++)
        {
            int bishopRank=rank+bishopOffsets[i][0]*multiplier, bishopFile=file+bishopOffsets[i][1]*multiplier;
            if(bishopRank<0 || bishopRank>7 || bishopFile<0 || bishopFile>7)
                break;
            int idx=bishopRank*8 + bishopFile;
            if(GetBit(bishops, idx) || GetBit(queens, idx))
                return true;
            else if(GetBit(allPieces, idx))
                break;
        }
    }

    //KING
    for(int i=0; i<8; i++)
    {
        int kingRank=rank+kingOffsets[i][0], kingFile=file+kingOffsets[i][1];
        if(kingRank<0 || kingRank>7 || kingFile<0 || kingFile>7)
            continue;
        if(GetBit(king, kingRank*8+kingFile))
            return true;
    }

    return false;
}

// For sliding moves: walks from `from` towards `to` and returns false if path is not clear
// intermediate square is occupied (destination `to` is not checked here).
// NOTE: does not check if piece is sliding or not!!!!
bool Board::isPathClear(int from, int to)
{
    int fromFile = from % 8, fromRank = from / 8;
    int toFile   = to % 8, toRank   = to / 8;
    int diffRank=toRank-fromRank, diffFile=toFile-fromFile;
    int moveRank= (diffRank > 0) - (diffRank < 0); //sign of diffRank: 1 if positive, -1 if negative, and 0 if zero.
    int moveFile= (diffFile > 0) - (diffFile < 0); //sign of diffRank: 1 if positive, -1 if negative, and 0 if zero.
    U64 allPieces=GetAllPieces();
    fromFile+=moveFile;
    fromRank+=moveRank;
    while(fromFile!=toFile || fromRank!=toRank)
    {
        if(GetBit(allPieces,(fromFile+(8*fromRank))))
            return false;
        fromFile+=moveFile;
        fromRank+=moveRank;
    }
    return true;
}

// piece's movement rules (including obstruction checks for sliders, and also king and pawn even though its trivial).
//Note legality doesnt check about king safety, check: dont know what happen if capture piece is king?can it be king?
bool Board::moveLegalityCheck(Move move)
{
    U64 whitePieces=GetWhitePieces();
    U64 blackPieces=GetBlackPieces();
    U64 allPieces=GetAllPieces();

    //Caching bitboards
    const U64& movingPiece=getPieceBB(move.movingPiece);
    const U64& capturedPiece=getPieceBB(move.capturedPiece);

    //Calculating ranks and files
    int fromFile = move.from % 8, fromRank = move.from / 8;
    int toFile   = move.to % 8, toRank   = move.to / 8;
    int df = std::abs(toFile - fromFile);
    int dr = std::abs(toRank - fromRank);

    if(move.from<0 || move.from>63 || move.to<0 || move.to>63 || move.from==move.to || !GetBit(movingPiece,move.from))
        return false;
    if((whiteToMove && !(whitePieces & movingPiece)) || (!whiteToMove && !(blackPieces & movingPiece))) //checks if we are moving our own piece
        return false;
    if(move.capturedPiece!=Piece::None && hasFlag(move.flag,CAPTURE | EN_PASSANT)) //checks captured piece is opponent's
        if((whiteToMove && !(capturedPiece&blackPieces)) || (!whiteToMove && !(capturedPiece&whitePieces)))
            return false;
    if(move.movingPiece!=Piece::BN && move.movingPiece!=Piece::WN && !isPathClear(move.from,move.to))
        return false;
    if(hasFlag(move.flag, CAPTURE)==0 && GetBit(allPieces, move.to))
        return false;
    if (move.capturedPiece == Piece::WK || move.capturedPiece == Piece::BK)
        return false;

    if(move.flag!=0)
    {
        if(hasFlag(move.flag, CAPTURE))
        {
            if(!GetBit(capturedPiece,move.to))
                return false;
        }

        if(hasFlag(move.flag, PROMOTION))
        {
            if(whiteToMove && toRank!=7)
                return false;
            if(!whiteToMove && toRank!=0)
                return false;
            if(move.movingPiece!=Piece::WP && move.movingPiece !=Piece::BP)
                return false;
            //here add a condition to check if promoted piece cant be WK BK WP BP
            if(whiteToMove && !(move.promotionPiece==Piece::WQ || move.promotionPiece==Piece::WR || move.promotionPiece==Piece::WB || move.promotionPiece==Piece::WN))
                return false;
            if(!whiteToMove && !(move.promotionPiece==Piece::BQ || move.promotionPiece==Piece::BR || move.promotionPiece==Piece::BB || move.promotionPiece==Piece::BN))
                return false;

        }
        else if(hasFlag(move.flag, EN_PASSANT))
        {
            if(df!=1 || dr!=1 || GetBit(allPieces,move.to))
                return false;
            if(whiteToMove && !(GetBit(BP, move.to-8) && move.to==enPassantSquare))
                return false;
            if(!whiteToMove && !(GetBit(WP, move.to+8) && move.to==enPassantSquare))
                return false;
        }
        else if(hasFlag(move.flag, CASTLE_KING))
        {
            if(whiteToMove && (!whiteCastleKingSide || GetBit(allPieces, IDX_F1) || GetBit(allPieces, IDX_G1) || isSquareAttacked(IDX_E1,0) || isSquareAttacked(IDX_F1,0) || isSquareAttacked(IDX_G1,0)))
                return false;
            if(!whiteToMove && (!blackCastleKingSide || GetBit(allPieces, IDX_F8) || GetBit(allPieces, IDX_G8) || isSquareAttacked(IDX_E8,1) || isSquareAttacked(IDX_F8,1) || isSquareAttacked(IDX_G8,1)))
                return false;
        }
        else if(hasFlag(move.flag, CASTLE_QUEEN))
        {
            if(whiteToMove && (!whiteCastleQueenSide || GetBit(allPieces, IDX_B1) || GetBit(allPieces, IDX_C1) || GetBit(allPieces, IDX_D1) || isSquareAttacked(IDX_E1,0) || isSquareAttacked(IDX_D1,0) || isSquareAttacked(IDX_C1,0)))
                return false;
            if(!whiteToMove && (!blackCastleQueenSide || GetBit(allPieces, IDX_B8) || GetBit(allPieces, IDX_C8) || GetBit(allPieces, IDX_D8) || isSquareAttacked(IDX_E8,1) || isSquareAttacked(IDX_D8,1) || isSquareAttacked(IDX_C8,1)))
                return false;
        }

        else if(hasFlag(move.flag, DOUBLE_PAWN))
        {
            if(dr!=2 || df!=0)
                return false;
            if(whiteToMove && fromRank!=1)
                return false;
            if(!whiteToMove && fromRank!=6)
                return false;
        }
    }
    if (move.movingPiece == Piece::WP)
    {
        if(toRank==7 && !hasFlag(move.flag,PROMOTION))
            return false;
        if((move.flag==0 || hasFlag(move.flag,PROMOTION)) && !df && (toRank-fromRank==1))
            return true;
        if(hasFlag(move.flag,CAPTURE | EN_PASSANT) && df==1 && (toRank-fromRank==1))
            return true;
        if(hasFlag(move.flag, DOUBLE_PAWN) && !df && (toRank-fromRank==2))
            return true;
        return false;
    }
    else if(move.movingPiece == Piece::BP)
    {
        if(toRank==0 && !hasFlag(move.flag,PROMOTION))
            return false;
        if((move.flag==0 || hasFlag(move.flag,PROMOTION)) && !df && (toRank-fromRank==-1))
            return true;
        if(hasFlag(move.flag,CAPTURE | EN_PASSANT) && df==1 && (toRank-fromRank==-1))
            return true;
        if(hasFlag(move.flag, DOUBLE_PAWN) && !df && (toRank-fromRank==-2))
            return true;

        return false;
    }
    else if (move.movingPiece == Piece::WN || move.movingPiece == Piece::BN)
    {
        return (df==2 && dr==1) || (df==1 && dr==2);
    }
    else if (move.movingPiece == Piece::WB || move.movingPiece == Piece::BB)
    {
        return df==dr;
    }
    else if (move.movingPiece == Piece::WR || move.movingPiece == Piece::BR)
    {
        return (fromFile==toFile || fromRank==toRank);
    }
    else if (move.movingPiece == Piece::WQ || move.movingPiece == Piece::BQ)
    {
        return (fromFile==toFile || fromRank==toRank || df==dr);
    }
    else if (move.movingPiece == Piece::WK)
    {
        if(hasFlag(move.flag,CASTLE_KING) && move.from==IDX_E1 && move.to==IDX_G1 && GetBit(WR, IDX_H1))
            return true;
        else if(hasFlag(move.flag,CASTLE_QUEEN) && move.from==IDX_E1 && move.to==IDX_C1 && GetBit(WR, IDX_A1))
            return true;
        else
            return (df<=1 && dr<=1);
    }
    else if (move.movingPiece == Piece::BK)
    {
        if(hasFlag(move.flag,CASTLE_KING) && move.from==IDX_E8 && move.to==IDX_G8 && GetBit(BR, IDX_H8))
            return true;
        else if(hasFlag(move.flag,CASTLE_QUEEN) && move.from==IDX_E8 && move.to==IDX_C8 && GetBit(BR, IDX_A8))
            return true;
        else
            return (df<=1 && dr<=1);
    }

    //need to implement that king shouldnt be in check after move
    return true;
}

bool Board::makeMove(Move move)
{
    // if(!moveLegalityCheck(move))    //note: also checking legality on make move side
    //     return false;
    
    //saving undo state
    saveUndoState();

    //Zorbist: Removing old castling rights
    if(trackHistory)
    {
        if(whiteCastleKingSide)
            zorbistHash^=ZORBIST.castling[0];
        if(whiteCastleQueenSide)
            zorbistHash^=ZORBIST.castling[1];
        if(blackCastleKingSide)
            zorbistHash^=ZORBIST.castling[2];
        if(blackCastleQueenSide)
            zorbistHash^=ZORBIST.castling[3];
        //remove old enpassant square
        if (enPassantSquare != NO_SQUARE)
            zorbistHash ^= ZORBIST.enPassantFile[enPassantSquare % 8];
    }

    // Fetching BitBoards
    U64& movingPieceBB = getPieceBB(move.movingPiece);
    U64& capturedPieceBB = getPieceBB(move.capturedPiece);
    U64& promotionPieceBB = getPieceBB(move.promotionPiece);

    //move moving piece to desired location
    ResetBit(movingPieceBB, move.from);
    SetBit(movingPieceBB,move.to);

    pieceAt[move.from]=Piece::None;
    pieceAt[move.to]=move.movingPiece;

    if(move.flag!=0)
    {
        //handeling captures and En-passant
        if(hasFlag(move.flag,CAPTURE))
        {
            ResetBit(capturedPieceBB, move.to);
        }

        //handeling Promotion
        if(hasFlag(move.flag,PROMOTION))
        {
            ResetBit(movingPieceBB,move.to);
            SetBit(promotionPieceBB,move.to);
            pieceAt[move.to]=move.promotionPiece;
        }

        //handeling enpassant
        else if(hasFlag(move.flag,EN_PASSANT))
        {
            int offset = whiteToMove ? (-8) : 8;
            U64& capturedPawnBB = whiteToMove ? BP : WP;
            ResetBit(capturedPawnBB, move.to + offset);
            pieceAt[move.to + offset] = Piece::None;
        }

        //handeling CastleKingSide
        else if(hasFlag(move.flag,CASTLE_KING))
        {
            if(whiteToMove)
            {
                ResetBit(WR,IDX_H1);
                SetBit(WR,IDX_F1);
                pieceAt[IDX_H1]=Piece::None;
                pieceAt[IDX_F1]=Piece::WR;
            }
            else
            {
                ResetBit(BR,IDX_H8);
                SetBit(BR,IDX_F8);
                pieceAt[IDX_H8]=Piece::None;
                pieceAt[IDX_F8]=Piece::BR;
            }
        }

        //handeling CastleQueenSide
        else if(hasFlag(move.flag,CASTLE_QUEEN))
        {
            if(whiteToMove)
            {
                ResetBit(WR,IDX_A1);
                SetBit(WR,IDX_D1);
                pieceAt[IDX_A1]=Piece::None;
                pieceAt[IDX_D1]=Piece::WR;
            }
            else
            {
                ResetBit(BR,IDX_A8);
                SetBit(BR,IDX_D8);
                pieceAt[IDX_A8]=Piece::None;
                pieceAt[IDX_D8]=Piece::BR;
            }
        }
    }
    //checking if king in check after the move
    if((whiteToMove && isSquareAttacked(getPiecePosition(WK),0)) || (!whiteToMove && isSquareAttacked(getPiecePosition(BK),1)))
    {
        restoreState(move);
        return false;
    }

    //Updating Castling Rights
    if(whiteToMove)
    {
        if(move.movingPiece==Piece::WK)
        {
            whiteCastleKingSide=0;
            whiteCastleQueenSide=0;
        }
        else
        {
            if(move.movingPiece==Piece::WR)
            {
                if(move.from==IDX_A1)
                    whiteCastleQueenSide=0;
                else if(move.from==IDX_H1)
                    whiteCastleKingSide=0;
            }
            if(move.capturedPiece==Piece::BR)
            {
                if(move.to==IDX_A8)
                    blackCastleQueenSide=0;
                else if(move.to==IDX_H8)
                    blackCastleKingSide=0;
            }
        }
    }
    else
    {
        if(move.movingPiece==Piece::BK)
        {
            blackCastleKingSide=0;
            blackCastleQueenSide=0;
        }
        else
        {
            if(move.movingPiece==Piece::BR)
            {
                if(move.from==IDX_A8)
                    blackCastleQueenSide=0;
                else if(move.from==IDX_H8)
                    blackCastleKingSide=0;
            }
            if(move.capturedPiece==Piece::WR)
            {
                if(move.to==IDX_A1)
                    whiteCastleQueenSide=0;
                else if(move.to==IDX_H1)
                    whiteCastleKingSide=0;
            }
        }
    }

    //setting enPassantSquare if double pawn push
    enPassantSquare=NO_SQUARE;
    if(hasFlag(move.flag,DOUBLE_PAWN))
        enPassantSquare=whiteToMove?move.to-8:move.to+8;

    //Note: below is not needed for perft
    // //marking if checked
    // if(whiteToMove)
    // {
    //     whiteInCheck=0; //clearing own check as once move is made as then own king not in check
    //     if(isSquareAttacked(getPiecePosition(BK),1))
    //         blackInCheck=1;
    //     else
    //         blackInCheck=0;
    // }
    // else
    // {
    //     blackInCheck=0;
    //     if(isSquareAttacked(getPiecePosition(WK),0))
    //         whiteInCheck=1;
    //     else
    //         whiteInCheck=0;
    // }

    //Updating 50 move counter
    if(move.movingPiece == Piece::WP || move.movingPiece==Piece::BP || hasFlag(move.flag,CAPTURE))
        fiftyMoveClock=0;
    else
        fiftyMoveClock++;

    //increasing move count
    if(!whiteToMove)
        moveCount++;

    //updating zorbist hash
    if(trackHistory)
        updateHash(move);

    //Flipping side to move
    whiteToMove=!whiteToMove;

    //adding position to position history
    if(trackHistory)
        positionHistory[zorbistHash]++;

    return true;

}

void Board::printBoard()
{
    char viewBoard[64];
    for(int i=0; i<64; i++)
    {
        if(GetBit(WP,i)) viewBoard[i]='P';
        else if(GetBit(BP,i)) viewBoard[i]='p';
        else if(GetBit(WN,i)) viewBoard[i]='N';
        else if(GetBit(BN,i)) viewBoard[i]='n';
        else if(GetBit(WB,i)) viewBoard[i]='B';
        else if(GetBit(BB,i)) viewBoard[i]='b';
        else if(GetBit(WR,i)) viewBoard[i]='R';
        else if(GetBit(BR,i)) viewBoard[i]='r';
        else if(GetBit(WQ,i)) viewBoard[i]='Q';
        else if(GetBit(BQ,i)) viewBoard[i]='q';
        else if(GetBit(WK,i)) viewBoard[i]='K';
        else if(GetBit(BK,i)) viewBoard[i]='k';
        else viewBoard[i]='.';
    }
    for(int8_t i=7; i>=0; i--)
    {
        for(int j=0; j<8; j++)
        {
            std::cout<<viewBoard[i*8+j]<<' ';
        }
        std::cout<<'\n';
    }
}