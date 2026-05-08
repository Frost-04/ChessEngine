#include "perft.h"
#include <iostream>

long long Perft::run(Board &board,int depth)
{
    if(depth==0)
        return 1;
    int nodes=0;
    MoveGenerator::nextValidMoves(board);
    if(depth == 1)
        return (long long)board.validMoves.size();
    std::vector<Move> moves=board.validMoves;
    for(auto &move: moves)
    {
        board.makeMove(move);
        nodes+=run(board,depth-1);
        board.undoMove();
    }
    return nodes;
}
void Perft::divide(Board &board, int depth)
{
    MoveGenerator::nextValidMoves(board);
    std::vector<Move> moves = board.validMoves;
    
    long long total = 0;
    for(auto &move : moves)
    {
        board.makeMove(move);
        long long count = run(board, depth - 1);
        board.undoMove();

        // print move
        char from_file = 'a' + move.from % 8;
        char from_rank = '1' + move.from / 8;
        char to_file   = 'a' + move.to % 8;
        char to_rank   = '1' + move.to / 8;
        std::cout << from_file << from_rank << to_file << to_rank;

        if(board.hasFlag(move.flag, PROMOTION))
        {
            switch(move.promotionPiece)
            {
                case Piece::WQ: case Piece::BQ: std::cout << "q"; break;
                case Piece::WR: case Piece::BR: std::cout << "r"; break;
                case Piece::WB: case Piece::BB: std::cout << "b"; break;
                case Piece::WN: case Piece::BN: std::cout << "n"; break;
                default: break;
            }
        }
        std::cout << ": " << count << "\n";
        total += count;
    }
    std::cout << "\nTotal: " << total << "\n";
}