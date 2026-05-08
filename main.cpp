#include "board.h"
#include "perft.h"
#include "fenloader.h"
#include <chrono>
#include <iostream>
#include <iomanip>

int main()
{
    Board board;
    board.trackHistory = false;
    // FenLoader loader;
    // std::cout<<"TEMP\n";
    // loader.load(board, "3k4/5ppp/2q5/3p2r1/8/1Q3P2/P4P1P/3R3K w - - 0 1");
    // Perft::divide(board, 4);
    // return 0;

    for (int i = 0; i <= 10; i++)
    {
        const auto start = std::chrono::steady_clock::now();
        const auto nodes = Perft::run(board, i);
        const auto end   = std::chrono::steady_clock::now();

        const std::chrono::duration<double> elapsed = end - start;

        std::cout << "Nodes at depth " << i << " : " << nodes
                  << " | time: " << std::fixed << std::setprecision(6)
                  << elapsed.count() << " s\n";
    }
    return 0;
}