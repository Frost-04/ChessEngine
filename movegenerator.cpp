#include "movegenerator.h"
#include "board.h"
#include "constants.h"

bool MoveGenerator::nextValidMoves(Board& board, MoveList &moves) {

    //getting piece identity
    Piece king = (board.whiteToMove) ? Piece::WK : Piece::BK;
    Piece pawn = (board.whiteToMove) ? Piece::WP : Piece::BP;
    Piece knight = (board.whiteToMove) ? Piece::WN : Piece::BN;
    Piece bishop = (board.whiteToMove) ? Piece::WB : Piece::BB;
    Piece rook = (board.whiteToMove) ? Piece::WR : Piece::BR;
    Piece queen = (board.whiteToMove) ? Piece::WQ : Piece::BQ;

    //caching BB
    const U64& kingBB = board.getPieceBB(king);
    const U64& pawnBB = board.getPieceBB(pawn);
    const U64& knightBB = board.getPieceBB(knight);
    const U64& bishopBB = board.getPieceBB(bishop);
    const U64& rookBB = board.getPieceBB(rook);
    const U64& queenBB = board.getPieceBB(queen);

    U64 allPieces=board.GetAllPieces();
    U64 friendlyPiece=(board.whiteToMove)?board.GetWhitePieces():board.GetBlackPieces();
    U64 opponentPiece=(board.whiteToMove)?board.GetBlackPieces():board.GetWhitePieces();

    //caching castle rules
    bool castleKing=(board.whiteToMove)?board.whiteCastleKingSide:board.blackCastleKingSide;
    bool castleQueen=(board.whiteToMove)?board.whiteCastleQueenSide:board.blackCastleQueenSide;

    //reusable Move struct object
    Move move;

    /*
    King
    */
    move.movingPiece = king;
    move.from = board.getPiecePosition(kingBB);
    move.promotionPiece = Piece::None;
    for(const auto &[dx, dy] : kingOffsets) {
        move.flag = 0;
        int newRank = (move.from/8) + dx;
        int newFile = (move.from%8) + dy;
        if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + dx * 8 + dy;
        else
            continue;
        if(board.GetBit(friendlyPiece,move.to))
            continue;
            
        move.capturedPiece = board.pieceAt[move.to];
        if(move.capturedPiece != Piece::None)
            board.setFlag(move.flag, CAPTURE);

        moves.push_back(move);
    }

    //CASTLING King Side
    if(castleKing)  //might need to put is square attacked here for the middle parts and also if middle part is empty
    {

        //checking castle rules and squares
        U8 intermediateSquare=board.whiteToMove? IDX_F1 : IDX_F8;
        move.flag = 0;
        move.to = board.whiteToMove ? IDX_G1 : IDX_G8;
        if(!board.GetBit(allPieces,intermediateSquare) && !board.GetBit(allPieces,move.to) && !board.isSquareAttacked(move.from, !board.whiteToMove) && !board.isSquareAttacked(intermediateSquare,!board.whiteToMove) && !board.isSquareAttacked(move.to,!board.whiteToMove))
        {
            move.capturedPiece=Piece::None;
            board.setFlag(move.flag, CASTLE_KING);
            moves.push_back(move);
        }
    }

    //CASTLING Queen Side
    if(castleQueen) //might need to put is square attacked here for the middle parts and also if middle part is empty
    {
        U8 intermediateSquare=board.whiteToMove? IDX_D1 : IDX_D8;
        U8 rookIntermediateSquare=board.whiteToMove? IDX_B1:IDX_B8;
        move.flag = 0;
        move.to = board.whiteToMove ? IDX_C1 : IDX_C8;
        if(!board.GetBit(allPieces,intermediateSquare) && !board.GetBit(allPieces,move.to) && !board.GetBit(allPieces,rookIntermediateSquare) && !board.isSquareAttacked(move.from, !board.whiteToMove) &&!board.isSquareAttacked(intermediateSquare,!board.whiteToMove) && !board.isSquareAttacked(move.to,!board.whiteToMove))
        {
            move.capturedPiece=Piece::None;
            board.setFlag(move.flag, CASTLE_QUEEN);
            moves.push_back(move);
        }
    }

    /*
    Pawn
    */
    move.movingPiece = pawn;
    int direction = board.whiteToMove ? 1 : -1;

    U64 pawnBBIterator=pawnBB;
    while (pawnBBIterator)
    {
        int currPosition = __builtin_ctzll(pawnBBIterator); pawnBBIterator &= pawnBBIterator - 1;
        move.from = currPosition;

        // Single forward
        move.to = move.from + 8 * direction;
        if(!board.GetBit(allPieces,move.to))
        {
            move.flag = 0;
            move.promotionPiece = Piece::None;
            move.capturedPiece = Piece::None;
            if((move.to >= IDX_A8 && move.to <= IDX_H8) || (move.to >= IDX_A1 && move.to <= IDX_H1))
            {
                board.setFlag(move.flag, PROMOTION);
                for(Piece p : {knight, rook, bishop, queen}) {
                    move.promotionPiece = p;
                    moves.push_back(move);
                }
            }
        
            else
                moves.push_back(move);
        }

        // Double push
        move.capturedPiece=Piece::None;
        if((board.whiteToMove && move.from/8==1) || (!board.whiteToMove && move.from/8==6))
        {
            move.to = move.from + 16 * direction;
            if(!board.GetBit(allPieces,move.to) && !board.GetBit(allPieces,move.from+8*direction))
            {
                move.flag = 0;
                board.setFlag(move.flag, DOUBLE_PAWN);
                move.promotionPiece = Piece::None;
                move.capturedPiece = Piece::None;
                moves.push_back(move);
            }
        }

        // Captures (left and right)
        for(int xDirection : {1, -1}) {
            int newRank = (move.from/8) + 1*direction;
            int newFile = (move.from%8) + xDirection;
            if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + direction*8 + xDirection;
            else
                continue;
            if(!board.GetBit(friendlyPiece,move.to) && board.pieceAt[move.to]!=Piece::None)
            {
                move.flag = 0;
                move.promotionPiece = Piece::None;
                move.capturedPiece = board.pieceAt[move.to];
                board.setFlag(move.flag, CAPTURE);

                if((move.to >= IDX_A8 && move.to <= IDX_H8) || (move.to >= IDX_A1 && move.to <= IDX_H1))
                {
                    board.setFlag(move.flag, PROMOTION);
                    for(Piece p : {knight, rook, bishop, queen}) {
                        move.promotionPiece = p;
                        moves.push_back(move);
                    }
                }
                else
                    moves.push_back(move);
            }
        }

        //En passant
        if(board.enPassantSquare!=NO_SQUARE)
        {
            move.flag=0;
            board.setFlag(move.flag, EN_PASSANT);
            for(auto xDirection : {1, -1}) 
            {
                int newRank = (move.from/8) + 1*direction;
                int newFile = (move.from%8) + xDirection;
                if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                    move.to = move.from + direction*8 + xDirection;
                else
                    continue;
                if(!board.GetBit(allPieces,move.to) && move.to==board.enPassantSquare)
                {
                    move.capturedPiece=board.whiteToMove?Piece::BP : Piece::WP;
                    moves.push_back(move);
                }
            }
        }
    }

    /*
    Knight
    */
    move.movingPiece = knight;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 knightBBIterator=knightBB;
    while (knightBBIterator)
    {
        int currPosition = __builtin_ctzll(knightBBIterator); knightBBIterator &= knightBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : knightOffsets) {
            move.flag = 0;
            int newRank = rank + dx;
            int newFile = file + dy;
            if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + dx * 8 + dy;
            else
                continue;
            if(!board.GetBit(friendlyPiece,move.to))
            {
                move.capturedPiece = board.pieceAt[move.to];
                if(move.capturedPiece != Piece::None)
                    board.setFlag(move.flag, CAPTURE);
                moves.push_back(move);     
            }
        }
    }

    /*
    Bishop
    */
    move.movingPiece = bishop;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 bishopBBIterator=bishopBB;
    while (bishopBBIterator)
    {
        int currPosition = __builtin_ctzll(bishopBBIterator); bishopBBIterator &= bishopBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : bishopOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);

                    moves.push_back(move);
                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else
                    break;
            }
        }
    }

    /*
    Rook
    */
    move.movingPiece = rook;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 rookBBIterator=rookBB;
    while (rookBBIterator)
    {
        int currPosition = __builtin_ctzll(rookBBIterator); rookBBIterator &= rookBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : rookOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);
                    moves.push_back(move);

                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else   
                    break;
            }
        }
    }

    /*
    Queen
    */
    move.movingPiece = queen;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 queenBBIterator=queenBB;
    while (queenBBIterator)
    {
        int currPosition = __builtin_ctzll(queenBBIterator); queenBBIterator &= queenBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : queenOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);
                    moves.push_back(move);

                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else
                    break;
            }
        }
    }

    if(moves.empty())
        return false;
    return true;
}   

bool MoveGenerator::hasValidMoves(Board& board) {
     //getting piece identity
    Piece king = (board.whiteToMove) ? Piece::WK : Piece::BK;
    Piece pawn = (board.whiteToMove) ? Piece::WP : Piece::BP;
    Piece knight = (board.whiteToMove) ? Piece::WN : Piece::BN;
    Piece bishop = (board.whiteToMove) ? Piece::WB : Piece::BB;
    Piece rook = (board.whiteToMove) ? Piece::WR : Piece::BR;
    Piece queen = (board.whiteToMove) ? Piece::WQ : Piece::BQ;

    //caching BB
    const U64& kingBB = board.getPieceBB(king);
    const U64& pawnBB = board.getPieceBB(pawn);
    const U64& knightBB = board.getPieceBB(knight);
    const U64& bishopBB = board.getPieceBB(bishop);
    const U64& rookBB = board.getPieceBB(rook);
    const U64& queenBB = board.getPieceBB(queen);

    U64 allPieces=board.GetAllPieces();
    U64 friendlyPiece=(board.whiteToMove)?board.GetWhitePieces():board.GetBlackPieces();
    U64 opponentPiece=(board.whiteToMove)?board.GetBlackPieces():board.GetWhitePieces();

    //caching castle rules
    bool castleKing=(board.whiteToMove)?board.whiteCastleKingSide:board.blackCastleKingSide;
    bool castleQueen=(board.whiteToMove)?board.whiteCastleQueenSide:board.blackCastleQueenSide;

    //reusable Move struct object
    Move move;

    /*
    King
    */
    move.movingPiece = king;
    move.from = board.getPiecePosition(kingBB);
    move.promotionPiece = Piece::None;
    for(const auto &[dx, dy] : kingOffsets) {
        move.flag = 0;
        int newRank = (move.from/8) + dx;
        int newFile = (move.from%8) + dy;
        if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + dx * 8 + dy;
        else
            continue;
        if(board.GetBit(friendlyPiece,move.to))
            continue;
            
        move.capturedPiece = board.pieceAt[move.to];
        if(move.capturedPiece != Piece::None)
            board.setFlag(move.flag, CAPTURE);
        return true;
    }

    //CASTLING King Side
    if(castleKing)  //might need to put is square attacked here for the middle parts and also if middle part is empty
    {

        //checking castle rules and squares
        U8 intermediateSquare=board.whiteToMove? IDX_F1 : IDX_F8;
        move.flag = 0;
        move.to = board.whiteToMove ? IDX_G1 : IDX_G8;
        if(!board.GetBit(allPieces,intermediateSquare) && !board.GetBit(allPieces,move.to) && !board.isSquareAttacked(move.from, !board.whiteToMove) && !board.isSquareAttacked(intermediateSquare,!board.whiteToMove) && !board.isSquareAttacked(move.to,!board.whiteToMove))
        {
            move.capturedPiece=Piece::None;
            board.setFlag(move.flag, CASTLE_KING);
            return true;
        }
    }

    //CASTLING Queen Side
    if(castleQueen) //might need to put is square attacked here for the middle parts and also if middle part is empty
    {
        U8 intermediateSquare=board.whiteToMove? IDX_D1 : IDX_D8;
        U8 rookIntermediateSquare=board.whiteToMove? IDX_B1:IDX_B8;
        move.flag = 0;
        move.to = board.whiteToMove ? IDX_C1 : IDX_C8;
        if(!board.GetBit(allPieces,intermediateSquare) && !board.GetBit(allPieces,move.to) && !board.GetBit(allPieces,rookIntermediateSquare) && !board.isSquareAttacked(move.from, !board.whiteToMove) &&!board.isSquareAttacked(intermediateSquare,!board.whiteToMove) && !board.isSquareAttacked(move.to,!board.whiteToMove))
        {
            move.capturedPiece=Piece::None;
            board.setFlag(move.flag, CASTLE_QUEEN);
            return true;
        }
    }

    /*
    Pawn
    */
    move.movingPiece = pawn;
    int direction = board.whiteToMove ? 1 : -1;

    U64 pawnBBIterator=pawnBB;
    while (pawnBBIterator)
    {
        int currPosition = __builtin_ctzll(pawnBBIterator); pawnBBIterator &= pawnBBIterator - 1;
        move.from = currPosition;

        // Single forward
        move.to = move.from + 8 * direction;
        if(!board.GetBit(allPieces,move.to))
        {
            move.flag = 0;
            move.promotionPiece = Piece::None;
            move.capturedPiece = Piece::None;
            if((move.to >= IDX_A8 && move.to <= IDX_H8) || (move.to >= IDX_A1 && move.to <= IDX_H1))
            {
                board.setFlag(move.flag, PROMOTION);
                for(Piece p : {knight, rook, bishop, queen}) {
                    move.promotionPiece = p;
                    return true;
                }
            }
        
            else
            {
                return true;
            }
        }

        // Double push
        move.capturedPiece=Piece::None;
        if((board.whiteToMove && move.from/8==1) || (!board.whiteToMove && move.from/8==6))
        {
            move.to = move.from + 16 * direction;
            if(!board.GetBit(allPieces,move.to) && !board.GetBit(allPieces,move.from+8*direction))
            {
                move.flag = 0;
                board.setFlag(move.flag, DOUBLE_PAWN);
                move.promotionPiece = Piece::None;
                move.capturedPiece = Piece::None;
                return true;
            }
        }

        // Captures (left and right)
        for(int xDirection : {1, -1}) {
            int newRank = (move.from/8) + 1*direction;
            int newFile = (move.from%8) + xDirection;
            if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + direction*8 + xDirection;
            else
                continue;
            if(!board.GetBit(friendlyPiece,move.to) && board.pieceAt[move.to]!=Piece::None)
            {
                move.flag = 0;
                move.promotionPiece = Piece::None;
                move.capturedPiece = board.pieceAt[move.to];
                board.setFlag(move.flag, CAPTURE);

                if((move.to >= IDX_A8 && move.to <= IDX_H8) || (move.to >= IDX_A1 && move.to <= IDX_H1))
                {
                    board.setFlag(move.flag, PROMOTION);
                    for(Piece p : {knight, rook, bishop, queen}) {
                        move.promotionPiece = p;
                        return true;
                    }
                }
                else
                {
                    return true;
                }
            }
        }

        //En passant
        if(board.enPassantSquare!=NO_SQUARE)
        {
            move.flag=0;
            board.setFlag(move.flag, EN_PASSANT);
            for(auto xDirection : {1, -1}) 
            {
                int newRank = (move.from/8) + 1*direction;
                int newFile = (move.from%8) + xDirection;
                if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                    move.to = move.from + direction*8 + xDirection;
                else
                    continue;
                if(!board.GetBit(allPieces,move.to) && move.to==board.enPassantSquare)
                {
                    move.capturedPiece=board.whiteToMove?Piece::BP : Piece::WP;
                    return true;
                }
            }
        }
    }

    /*
    Knight
    */
    move.movingPiece = knight;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 knightBBIterator=knightBB;
    while (knightBBIterator)
    {
        int currPosition = __builtin_ctzll(knightBBIterator); knightBBIterator &= knightBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : knightOffsets) {
            move.flag = 0;
            int newRank = rank + dx;
            int newFile = file + dy;
            if(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7)
                move.to = move.from + dx * 8 + dy;
            else
                continue;
            if(!board.GetBit(friendlyPiece,move.to))
            {
                move.capturedPiece = board.pieceAt[move.to];
                if(move.capturedPiece != Piece::None)
                    board.setFlag(move.flag, CAPTURE);
                return true;     
            }
        }
    }

    /*
    Bishop
    */
    move.movingPiece = bishop;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 bishopBBIterator=bishopBB;
    while (bishopBBIterator)
    {
        int currPosition = __builtin_ctzll(bishopBBIterator); bishopBBIterator &= bishopBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : bishopOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);

                    return true;
                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else
                    break;
            }
        }
    }

    /*
    Rook
    */
    move.movingPiece = rook;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 rookBBIterator=rookBB;
    while (rookBBIterator)
    {
        int currPosition = __builtin_ctzll(rookBBIterator); rookBBIterator &= rookBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : rookOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);
                    return true;

                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else   
                    break;
            }
        }
    }

    /*
    Queen
    */
    move.movingPiece = queen;
    move.flag=0;
    move.promotionPiece = Piece::None;
    U64 queenBBIterator=queenBB;
    while (queenBBIterator)
    {
        int currPosition = __builtin_ctzll(queenBBIterator); queenBBIterator &= queenBBIterator - 1;
        move.from = currPosition;
        int rank = move.from / 8;
        int file = move.from % 8;
        for(const auto &[dx, dy] : queenOffsets) {
            int newRank = rank + dx;
            int newFile = file + dy;
            while(newRank >= 0 && newRank <= 7 && newFile >= 0 && newFile <= 7) {
                move.flag = 0;
                move.to = newRank * 8 + newFile;
                if(!board.GetBit(friendlyPiece,move.to))
                {
                    move.capturedPiece = board.pieceAt[move.to];
                    if(move.capturedPiece != Piece::None)
                        board.setFlag(move.flag, CAPTURE);

                    return true;

                    //stop sliding only when some piece is encounterd in the way
                    if(move.capturedPiece != Piece::None)
                        break;
                    newRank += dx;
                    newFile += dy;
                }
                else
                    break;
            }
        }
    }

    return false;
}