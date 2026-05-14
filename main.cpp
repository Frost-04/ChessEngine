#include "board.h"
#include "search.h"
#include "algebraic_parser.h"
#include "movegenerator.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

//using anon namespace so that these functions are localized to this main.cpp only 
namespace {

char pieceSanLetter(Piece piece) {
    switch (piece) {
        case Piece::WK:
        case Piece::BK:
            return 'K';
        case Piece::WQ:
        case Piece::BQ:
            return 'Q';
        case Piece::WR:
        case Piece::BR:
            return 'R';
        case Piece::WB:
        case Piece::BB:
            return 'B';
        case Piece::WN:
        case Piece::BN:
            return 'N';
        default:
            return '\0';
    }
}

char fileChar(int sq) { return static_cast<char>('a' + (sq % 8)); }
char rankChar(int sq) { return static_cast<char>('1' + (sq / 8)); }

bool isLegalMove(Board& board, const Move& move) {
    if (!board.makeMove(move)) {
        return false;
    }
    board.undoMove(move);
    return true;
}

void appendSquare(std::string& out, int sq) {
    out.push_back(fileChar(sq));
    out.push_back(rankChar(sq));
}

std::string moveToSan(Board& board, const Move& move) {
    if (move.flag & CASTLE_KING) {
        return "O-O";
    }
    if (move.flag & CASTLE_QUEEN) {
        return "O-O-O";
    }

    bool isPawn = (move.movingPiece == Piece::WP || move.movingPiece == Piece::BP);
    bool isCapture = (move.flag & CAPTURE) != 0 || (move.flag & EN_PASSANT) != 0 || move.capturedPiece != Piece::None;

    std::string san;
    san.reserve(6);

    if (!isPawn) {
        san.push_back(pieceSanLetter(move.movingPiece));

        MoveList moves;
        MoveGenerator::nextValidMoves(board, moves);

        bool hasSameFile = false;
        bool hasSameRank = false;
        int sameDestCount = 0;

        for (const Move& candidate : moves) {
            if (candidate.movingPiece != move.movingPiece || candidate.to != move.to) {
                continue;
            }
            if (!isLegalMove(board, candidate)) {
                continue;
            }
            sameDestCount++;
            if (candidate.from == move.from) {
                continue;
            }
            if ((candidate.from % 8) == (move.from % 8)) {
                hasSameFile = true;
            }
            if ((candidate.from / 8) == (move.from / 8)) {
                hasSameRank = true;
            }
        }

        if (sameDestCount > 1) {
            if (!hasSameFile) {
                san.push_back(fileChar(move.from));
            } else if (!hasSameRank) {
                san.push_back(rankChar(move.from));
            } else {
                san.push_back(fileChar(move.from));
                san.push_back(rankChar(move.from));
            }
        }
    } else if (isCapture) {
        san.push_back(fileChar(move.from));
    }

    if (isCapture) {
        san.push_back('x');
    }

    appendSquare(san, move.to);

    if (move.flag & PROMOTION) {
        san.push_back('=');
        san.push_back(pieceSanLetter(move.promotionPiece));
    }

    bool givesCheck = false;
    bool givesMate = false;
    if (board.makeMove(move)) {
        int kingSq = board.getPiecePosition(board.whiteToMove ? board.WK : board.BK);
        bool inCheck = board.isSquareAttacked(kingSq, !board.whiteToMove);
        if (inCheck) {
            MoveList replies;
            MoveGenerator::nextValidMoves(board, replies);
            bool anyLegal = false;
            for (const Move& reply : replies) {
                if (board.makeMove(reply)) {
                    board.undoMove(reply);
                    anyLegal = true;
                    break;
                }
            }
            givesCheck = true;
            givesMate = !anyLegal;
        }
        board.undoMove(move);
    }

    if (givesMate) {
        san.push_back('#');
    } else if (givesCheck) {
        san.push_back('+');
    }

    return san;
}

bool readDepth(int& depth) {
    std::string line;
    while (true) {
        std::cout << "Enter search depth: ";
        if (!std::getline(std::cin, line)) {
            return false;
        }
        try {
            depth = std::stoi(line);
        } catch (...) {
            std::cout << "Please enter a valid integer.\n";
            continue;
        }
        if (depth <= 0) {
            std::cout << "Depth must be positive.\n";
            continue;
        }
        return true;
    }
}

bool isQuitCommand(const std::string& text) {
    if (text == "quit" || text == "exit") {
        return true;
    }
    if (text.size() == 1 && (text[0] == 'q' || text[0] == 'Q')) {
        return true;
    }
    return false;
}

bool readSide(bool& playAsWhite) {
    std::string line;
    while (true) {
        std::cout << "Play as (w)hite or (b)lack? ";
        if (!std::getline(std::cin, line)) return false;
        if (line == "w" || line == "W" || line == "white" || line == "White") {
            playAsWhite = true;
            return true;
        }
        if (line == "b" || line == "B" || line == "black" || line == "Black") {
            playAsWhite = false;
            return true;
        }
        if (line == "q" || line == "quit") return false;
        std::cout << "Please enter 'w' or 'b'.\n";
    }
}


} // namespace

int main()
{
    Board board;
    Search search;

    double totalMoveTimeMs = 0.0;
    int timedMoveCount = 0;

    int depth = 0;
    if (!readDepth(depth)) {
        return 0;
    }

    bool playerIsWhite = true;
    if (!readSide(playerIsWhite)) {
        return 0;
    }

    // If player chose black, computer (white) moves first.
    // The loop will handle that automatically because board starts with whiteToMove.

    while (true) {
        GameResult result = board.hasGameEnded();
        if (result != GameResult::Ongoing) {
            board.printBoard();
            std::cout << "Game over: " << static_cast<int>(result) << "\n";
            if (timedMoveCount > 0) {
                double average = totalMoveTimeMs / timedMoveCount;
                std::cout << std::fixed << std::setprecision(2);
                std::cout << "Total timed moves: " << timedMoveCount << "\n";
                std::cout << "Average move time: " << average << " ms\n";
            }
            break;
        }

        board.printBoard();

        if (board.whiteToMove == playerIsWhite) {
            // ---------- Player's turn ----------
            std::string input;
            auto moveStart = std::chrono::high_resolution_clock::now();

            std::cout << "Your move (e4, Nf3, O-O). Type 'quit' to exit: ";
            if (!std::getline(std::cin, input)) break;
            if (input.empty()) continue;
            if (isQuitCommand(input)) break;

            Move userMove{};
            std::string error;
            if (!AlgebraicParser::parse(board, input, userMove, &error)) {
                std::cout << "Invalid move: " << error << "\n";
                continue;
            }
            if (!board.makeMove(userMove)) {
                std::cout << "Illegal move.\n";
                continue;
            }

            auto moveEnd = std::chrono::high_resolution_clock::now();
            double moveTime = std::chrono::duration<double, std::milli>(moveEnd - moveStart).count();
            totalMoveTimeMs += moveTime;
            timedMoveCount++;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Time taken: " << moveTime << " ms\n";
        } else {
            // ---------- Engine's turn ----------
            auto moveStart = std::chrono::high_resolution_clock::now();

            auto res = search.bestMove(board, depth);

            auto moveEnd = std::chrono::high_resolution_clock::now();
            double moveTime = std::chrono::duration<double, std::milli>(moveEnd - moveStart).count();
            totalMoveTimeMs += moveTime;
            timedMoveCount++;

            std::string engineMove = moveToSan(board, res.bestMove);
            int scoreForWhite = board.whiteToMove ? res.score : -res.score;
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "\x1b[32m"
                      << "Engine: " << engineMove
                      << "  score: " << scoreForWhite
                      << "  time: " << moveTime << " ms"
                      << "\x1b[0m\n";

            if (!board.makeMove(res.bestMove)) {
                std::cout << "Engine move failed.\n";
                break;
            }
        }
    }

    board.printBoard();
    return 0;
}