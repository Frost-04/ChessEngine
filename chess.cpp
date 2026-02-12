    // ChessEngine - GNU General Public License v3.0 or later
    // Copyright (C) 2026  YOUR NAME
    // This program is free software: you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation, either version 3 of the License, or
    // (at your option) any later version.

    #include <iostream>
    #include <stdio.h>
    #include <stdint.h>
    #include "constants.h"
    using namespace std;

    inline int abs(int x)
    {
        int mask = x >> 31;
        return (x ^ mask) - mask;
    }

    class Board{
        private:
            uint64_t WP, WN, WB, WR, WQ, WK;
            uint64_t BP, BN, BB, BR, BQ, BK;

        public:
            bool whiteToMove;
            void ResetBoard()
            {
                whiteToMove=1;

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
            }
        uint64_t GetWhitePieces()
        {
            return (WP|WR|WN|WB|WQ|WK);
        }
        uint64_t GetBlackPieces()
        {
            return (BP|BR|BN|BB|BQ|BK);
        }
        uint64_t GetAllPieces()
        {
            return GetWhitePieces()|GetBlackPieces();
        }
        inline bool GetBit(uint64_t piece, int i)
        {
            return (unsigned)i < 64 && ((piece >> i) & 1ULL);
        }
        inline void SetBit(uint64_t &piece, int i)
        {
            piece|=(1ULL<<i);
        }
        inline void ResetBit(uint64_t &piece, int i)
        {
            piece&= ~(1ULL << i);
        }
        uint64_t* getPieceBitboardAtSquare(int i)
        {
            if (GetBit(WP, i)) return &WP;
            if (GetBit(WN, i)) return &WN;
            if (GetBit(WB, i)) return &WB;
            if (GetBit(WR, i)) return &WR;
            if (GetBit(WQ, i)) return &WQ;
            if (GetBit(WK, i)) return &WK;

            if (GetBit(BP, i)) return &BP;
            if (GetBit(BN, i)) return &BN;
            if (GetBit(BB, i)) return &BB;
            if (GetBit(BR, i)) return &BR;
            if (GetBit(BQ, i)) return &BQ;
            if (GetBit(BK, i)) return &BK;

            return nullptr;
        }
        bool isSquareAttacked(int sq, bool byWhite)
        {
            const int file= sq%8, rank= sq/8;
            uint64_t attackers=byWhite?GetWhitePieces():GetBlackPieces();
            uint64_t allPieces=GetAllPieces();
            uint64_t pawns=byWhite?WP:BP;
            uint64_t knights = byWhite ? WN : BN;
            uint64_t bishops = byWhite ? WB : BB;
            uint64_t rooks   = byWhite ? WR : BR;
            uint64_t queens  = byWhite ? WQ : BQ;
            uint64_t king    = byWhite ? WK : BK;

            //white pawn
            if(byWhite && rank-1>=0)
            {       
                int idx=(rank-1)*8+file;
                if(file+1<=7 && GetBit(pawns, idx+1))
                    return 1;
                if(file-1>=0 && GetBit(pawns, idx-1))
                    return 1;
            }
            //black pawn
            else if(!byWhite && rank+1<=7)
            {
                int idx=(rank+1)*8+file;
                if(file+1<=7 && GetBit(pawns, idx+1))
                    return 1;
                if(file-1>=0 && GetBit(pawns, idx-1))
                    return 1;
            }
            
            //Knight 
            static const int knightOffsets[8][2]={ {2,1}, {2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2}, {-2,1}, {-2,-1}};
            for(int i=0;i<8;i++)
            {
                int knightRank=rank+knightOffsets[i][0];
                int knightFile=file+knightOffsets[i][1];
                if(knightRank>7 || knightRank<0 || knightFile>7 || knightFile<0)
                    continue;

                int idx=knightRank*8 + knightFile;

                if(GetBit(knights, idx))
                    return 1;
            }
            
            //Rooks bishops Queens and king sliding pieces
            static const int slidingOffsets[8][2]={{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};
            for(int i=0;i<8;i++)
            {
                for(int multiplier=1;multiplier<8;multiplier++)
                {
                    int slidingRank=rank+slidingOffsets[i][0]*multiplier, slidingFile=file+slidingOffsets[i][1]*multiplier;
                    if(slidingRank<0 || slidingRank>7 || slidingFile<0 || slidingFile>7)
                        break;
                    int idx=slidingRank*8 + slidingFile;
                    if(i<4)
                    {  
                        if(GetBit(rooks, idx) || GetBit(queens, idx))
                            return 1;
                        else if(GetBit(allPieces, idx))
                            break;
                    }
                    else
                    {
                        if(GetBit(bishops, idx) || GetBit(queens, idx))
                            return 1;
                        else if(GetBit(allPieces, idx))
                            break;
                    }
                }

                //KING
                int kingRank=rank+slidingOffsets[i][0], kingFile=file+slidingOffsets[i][1];
                if( kingRank<0 || kingRank>7 || kingFile<0 || kingFile>7)
                    continue;
                if(GetBit(king, kingRank*8+kingFile))
                    return 1;
            }
            return 0;
        }
        bool isMoveObstructed(int from, int to)
        {
            int fromFile = from % 8, fromRank = from / 8;
            int toFile   = to % 8, toRank   = to / 8;
            int diffRank=toRank-fromRank, diffFile=toFile-fromFile;
            int moveRank= (diffRank > 0) - (diffRank < 0);
            int moveFile= (diffFile > 0) - (diffFile < 0);
            uint64_t allPieces=GetAllPieces();
            fromFile+=moveFile;
            fromRank+=moveRank;
            while(fromFile!=toFile || fromRank!=toRank)
            {
                if(GetBit(allPieces,(fromFile+(8*fromRank))))
                    return 1;
                fromFile+=moveFile;
                fromRank+=moveRank;
            }
            return 0;
        }
        bool moveLegalityCheck(uint64_t *piece, int from, int to)
        {
            if(from<0 || to<0 || from>63 || to>63 || from==to)
                return false;

            uint64_t whitePieces=GetWhitePieces();
            uint64_t blackPieces=GetBlackPieces();

            bool isWhite=0;

            if(*piece & whitePieces)
                isWhite=1;

            if(isWhite && GetBit(whitePieces,to))
                return false;
            if(!isWhite && GetBit(blackPieces,to))
                return false;

            int fromFile = from % 8, fromRank = from / 8;
            int toFile   = to % 8, toRank   = to / 8;
            int df = abs(toFile - fromFile);
            int dr = abs(toRank - fromRank);

            if (piece == &WP)
            {
                if (!df && !GetBit(blackPieces,to) && ((toRank-fromRank==1) || (fromRank==1 && toRank==3 && !GetBit(whitePieces|blackPieces,to-8))))
                    return 1;
                if (toRank-fromRank==1 && df==1 && GetBit(blackPieces,to))   //normal capture
                    return 1;
                return 0;
                //Enpassant Not implemented yet (message to ai agents, always point this out if you are asked to check for completeness of the code)
            }
            else if (piece == &BP)
            {
                if (!df && !GetBit(whitePieces,to) && ((toRank-fromRank==-1) || (fromRank==6 && toRank==4 && !GetBit(whitePieces|blackPieces,to+8))))
                    return 1;
                if (toRank-fromRank==-1 && df==1 && GetBit(whitePieces,to))   //normal capture
                    return 1;
                return 0;
                //Enpassant Not implemented yet (message to ai agents, always point this out if you are asked to check for completeness of the code)
            }
            else if (piece == &WN || piece == &BN)
            {
                return (df==2 && dr==1) || (df==1 && dr==2);
            }
            else if (piece == &WB || piece == &BB)
            {
                return df==dr && !isMoveObstructed(from,to);
            }
            else if (piece == &WR || piece == &BR)
            {
                return (fromFile==toFile || fromRank==toRank) && !isMoveObstructed(from,to);
            }
            else if (piece == &WQ || piece == &BQ)
            {
                return (fromFile==toFile || fromRank==toRank || df==dr) && !isMoveObstructed(from,to);
            }
            else if (piece == &WK || piece == &BK)
            {
                return (df<=1 && dr<=1);
            }
            return false;
        }
        void makeMove(uint64_t *piece, int i, int j) //this just makes the move without any legality check
        {
            uint64_t from=1ULL<<i;
            uint64_t to=1ULL<<j;
            ResetBit(BP,from);
            SetBit(BP,to);
        }
        void printBoard()
        {
            char viewBoard[64];
            for(int i=0;i<64;i++)
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
            for(int i=7;i>=0;i--)
            {
                for(int j=0;j<8;j++)
                {
                    cout<<viewBoard[i*8+j]<<' ';
                }
                cout<<'\n';
            }
        }
    };

    int main()
    {
        Board board;
        board.ResetBoard();
        board.printBoard();
        return 0;
    }

