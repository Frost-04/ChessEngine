#pragma once

#include <cstdint>

using U64 = uint64_t;
using U8 = uint8_t;

// Move flag
constexpr U8 CAPTURE      = 1ULL << 0; //can be synergized with PROMOTION
constexpr U8 EN_PASSANT   = 1ULL << 1; //this is to be implemented with special rule that EN_PASSANT flag dont have CAPTURE flag
constexpr U8 CASTLE_KING  = 1ULL << 2;
constexpr U8 CASTLE_QUEEN = 1ULL << 3;
constexpr U8 DOUBLE_PAWN  = 1ULL << 4;
constexpr U8 PROMOTION    = 1ULL << 5; //can be synergized with CAPTURE

//Movement offsets
constexpr int knightOffsets[8][2]= {{2,1}, {2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2}, {-2,1}, {-2,-1}};
constexpr int rookOffsets[4][2]= {{0,1}, {0,-1}, {1,0}, {-1,0}};
constexpr int bishopOffsets[4][2]= {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
constexpr int queenOffsets[8][2]= {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};
constexpr int kingOffsets[8][2]= {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

constexpr U8 NO_SQUARE = 128; //big number

// Bitboard constants for each square (LSB = A1, MSB = H8)
constexpr U64 A1 = 0x0000000000000001ULL;
constexpr U64 B1 = 0x0000000000000002ULL;
constexpr U64 C1 = 0x0000000000000004ULL;
constexpr U64 D1 = 0x0000000000000008ULL;
constexpr U64 E1 = 0x0000000000000010ULL;
constexpr U64 F1 = 0x0000000000000020ULL;
constexpr U64 G1 = 0x0000000000000040ULL;
constexpr U64 H1 = 0x0000000000000080ULL;

constexpr U64 A2 = 0x0000000000000100ULL;
constexpr U64 B2 = 0x0000000000000200ULL;
constexpr U64 C2 = 0x0000000000000400ULL;
constexpr U64 D2 = 0x0000000000000800ULL;
constexpr U64 E2 = 0x0000000000001000ULL;
constexpr U64 F2 = 0x0000000000002000ULL;
constexpr U64 G2 = 0x0000000000004000ULL;
constexpr U64 H2 = 0x0000000000008000ULL;

constexpr U64 A3 = 0x0000000000010000ULL;
constexpr U64 B3 = 0x0000000000020000ULL;
constexpr U64 C3 = 0x0000000000040000ULL;
constexpr U64 D3 = 0x0000000000080000ULL;
constexpr U64 E3 = 0x0000000000100000ULL;
constexpr U64 F3 = 0x0000000000200000ULL;
constexpr U64 G3 = 0x0000000000400000ULL;
constexpr U64 H3 = 0x0000000000800000ULL;

constexpr U64 A4 = 0x0000000001000000ULL;
constexpr U64 B4 = 0x0000000002000000ULL;
constexpr U64 C4 = 0x0000000004000000ULL;
constexpr U64 D4 = 0x0000000008000000ULL;
constexpr U64 E4 = 0x0000000010000000ULL;
constexpr U64 F4 = 0x0000000020000000ULL;
constexpr U64 G4 = 0x0000000040000000ULL;
constexpr U64 H4 = 0x0000000080000000ULL;

constexpr U64 A5 = 0x0000000100000000ULL;
constexpr U64 B5 = 0x0000000200000000ULL;
constexpr U64 C5 = 0x0000000400000000ULL;
constexpr U64 D5 = 0x0000000800000000ULL;
constexpr U64 E5 = 0x0000001000000000ULL;
constexpr U64 F5 = 0x0000002000000000ULL;
constexpr U64 G5 = 0x0000004000000000ULL;
constexpr U64 H5 = 0x0000008000000000ULL;

constexpr U64 A6 = 0x0000010000000000ULL;
constexpr U64 B6 = 0x0000020000000000ULL;
constexpr U64 C6 = 0x0000040000000000ULL;
constexpr U64 D6 = 0x0000080000000000ULL;
constexpr U64 E6 = 0x0000100000000000ULL;
constexpr U64 F6 = 0x0000200000000000ULL;
constexpr U64 G6 = 0x0000400000000000ULL;
constexpr U64 H6 = 0x0000800000000000ULL;

constexpr U64 A7 = 0x0001000000000000ULL;
constexpr U64 B7 = 0x0002000000000000ULL;
constexpr U64 C7 = 0x0004000000000000ULL;
constexpr U64 D7 = 0x0008000000000000ULL;
constexpr U64 E7 = 0x0010000000000000ULL;
constexpr U64 F7 = 0x0020000000000000ULL;
constexpr U64 G7 = 0x0040000000000000ULL;
constexpr U64 H7 = 0x0080000000000000ULL;

constexpr U64 A8 = 0x0100000000000000ULL;
constexpr U64 B8 = 0x0200000000000000ULL;
constexpr U64 C8 = 0x0400000000000000ULL;
constexpr U64 D8 = 0x0800000000000000ULL;
constexpr U64 E8 = 0x1000000000000000ULL;
constexpr U64 F8 = 0x2000000000000000ULL;
constexpr U64 G8 = 0x4000000000000000ULL;
constexpr U64 H8 = 0x8000000000000000ULL;

constexpr U8 IDX_A1 = 0;
constexpr U8 IDX_B1 = 1;
constexpr U8 IDX_C1 = 2;
constexpr U8 IDX_D1 = 3;
constexpr U8 IDX_E1 = 4;
constexpr U8 IDX_F1 = 5;
constexpr U8 IDX_G1 = 6;
constexpr U8 IDX_H1 = 7;

constexpr U8 IDX_A2 = 8;
constexpr U8 IDX_B2 = 9;
constexpr U8 IDX_C2 = 10;
constexpr U8 IDX_D2 = 11;
constexpr U8 IDX_E2 = 12;
constexpr U8 IDX_F2 = 13;
constexpr U8 IDX_G2 = 14;
constexpr U8 IDX_H2 = 15;

constexpr U8 IDX_A3 = 16;
constexpr U8 IDX_B3 = 17;
constexpr U8 IDX_C3 = 18;
constexpr U8 IDX_D3 = 19;
constexpr U8 IDX_E3 = 20;
constexpr U8 IDX_F3 = 21;
constexpr U8 IDX_G3 = 22;
constexpr U8 IDX_H3 = 23;

constexpr U8 IDX_A4 = 24;
constexpr U8 IDX_B4 = 25;
constexpr U8 IDX_C4 = 26;
constexpr U8 IDX_D4 = 27;
constexpr U8 IDX_E4 = 28;
constexpr U8 IDX_F4 = 29;
constexpr U8 IDX_G4 = 30;
constexpr U8 IDX_H4 = 31;

constexpr U8 IDX_A5 = 32;
constexpr U8 IDX_B5 = 33;
constexpr U8 IDX_C5 = 34;
constexpr U8 IDX_D5 = 35;
constexpr U8 IDX_E5 = 36;
constexpr U8 IDX_F5 = 37;
constexpr U8 IDX_G5 = 38;
constexpr U8 IDX_H5 = 39;

constexpr U8 IDX_A6 = 40;
constexpr U8 IDX_B6 = 41;
constexpr U8 IDX_C6 = 42;
constexpr U8 IDX_D6 = 43;
constexpr U8 IDX_E6 = 44;
constexpr U8 IDX_F6 = 45;
constexpr U8 IDX_G6 = 46;
constexpr U8 IDX_H6 = 47;

constexpr U8 IDX_A7 = 48;
constexpr U8 IDX_B7 = 49;
constexpr U8 IDX_C7 = 50;
constexpr U8 IDX_D7 = 51;
constexpr U8 IDX_E7 = 52;
constexpr U8 IDX_F7 = 53;
constexpr U8 IDX_G7 = 54;
constexpr U8 IDX_H7 = 55;

constexpr U8 IDX_A8 = 56;
constexpr U8 IDX_B8 = 57;
constexpr U8 IDX_C8 = 58;
constexpr U8 IDX_D8 = 59;
constexpr U8 IDX_E8 = 60;
constexpr U8 IDX_F8 = 61;
constexpr U8 IDX_G8 = 62;
constexpr U8 IDX_H8 = 63;