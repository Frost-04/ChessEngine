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
    FenLoader loader;
    loader.load(board, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
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