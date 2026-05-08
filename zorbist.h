#pragma once

#include <random>
#include <cstdint>
#include "constants.h"

struct zorbistTable{
    U64 pieces[12][64];
    U64 sideToMove;
    U64 castling[4];    //0=White King 1=white queen 2=black king 3=white queen
    U64 enPassantFile[8];

    zorbistTable()
    {
        std::mt19937_64 gen{1070372ull};
        for(auto &piece:pieces)
            for(auto &sq:piece)
                sq=gen();
        
        sideToMove=gen();

        for(auto &it:castling)
            it=gen();
        for(auto &it:enPassantFile)
            it=gen();
    }
};
inline zorbistTable ZORBIST;