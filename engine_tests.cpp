#include "board.h"
#include "evaluator.h"
#include "fenLoader.h"
#include "movegenerator.h"
#include "perft.h"
#include "search.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

struct TestStats {
    int passed = 0;
    int failed = 0;
    int skipped = 0;

    void expect(bool condition, const std::string& name) {
        if (condition) {
            ++passed;
            std::cout << "[PASS] " << name << "\n";
        } else {
            ++failed;
            std::cout << "[FAIL] " << name << "\n";
        }
    }

    template <typename T>
    void expectEq(const T& actual, const T& expected, const std::string& name) {
        if (actual == expected) {
            ++passed;
            std::cout << "[PASS] " << name << " (" << actual << ")\n";
        } else {
            ++failed;
            std::cout << "[FAIL] " << name << " (got " << actual
                      << ", expected " << expected << ")\n";
        }
    }

    void skip(const std::string& name) {
        ++skipped;
        std::cout << "[SKIP] " << name << "\n";
    }
};

struct PerftExpected {
    int depth;
    const char* nodes;
};

struct PerftTest {
    const char* name;
    const char* fen;
    std::vector<PerftExpected> expected;
};

const std::vector<PerftTest> kPerftTests = {
    {
        "Initial position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {
            {0, "1"},
            {1, "20"},
            {2, "400"},
            {3, "8902"},
            {4, "197281"},
            {5, "4865609"},
            {6, "119060324"},
            {7, "3195901860"},
            {8, "84998978956"},
            {9, "2439530234167"},
            {10, "69352859712417"},
            {11, "2097651003696806"},
            {12, "62854969236701747"},
            {13, "1981066775000396239"},
            {14, "61885021521585529237"},
            {15, "2015099950053364471960"},
        },
    },
    {
        "Position 2 (Kiwipete)",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {
            {1, "48"},
            {2, "2039"},
            {3, "97862"},
            {4, "4085603"},
            {5, "193690690"},
            {6, "8031647685"},
        },
    },
    {
        "Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {
            {1, "14"},
            {2, "191"},
            {3, "2812"},
            {4, "43238"},
            {5, "674624"},
            {6, "11030083"},
            {7, "178633661"},
            {8, "3009794393"},
        },
    },
    {
        "Position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {
            {1, "6"},
            {2, "264"},
            {3, "9467"},
            {4, "422333"},
            {5, "15833292"},
            {6, "706045033"},
        },
    },
    {
        "Position 4 (mirrored)",
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        {
            {1, "6"},
            {2, "264"},
            {3, "9467"},
            {4, "422333"},
            {5, "15833292"},
            {6, "706045033"},
        },
    },
    {
        "Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {
            {1, "44"},
            {2, "1486"},
            {3, "62379"},
            {4, "2103487"},
            {5, "89941194"},
        },
    },
    {
        "Position 6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        {
            {0, "1"},
            {1, "46"},
            {2, "2079"},
            {3, "89890"},
            {4, "3894594"},
            {5, "164075551"},
            {6, "6923051137"},
            {7, "287188994746"},
            {8, "11923589843526"},
            {9, "490154852788714"},
        },
    },
};

bool parseU64(const char* text, unsigned long long& out) {
    if (!text || *text == '\0') {
        return false;
    }
    unsigned long long value = 0;
    for (const char* p = text; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
        unsigned digit = static_cast<unsigned>(*p - '0');
        if (value > (std::numeric_limits<unsigned long long>::max() - digit) / 10ULL) {
            return false;
        }
        value = value * 10ULL + digit;
    }
    out = value;
    return true;
}

bool loadFen(Board& board, const char* fen) {
    FenLoader loader;
    std::string error;
    if (!loader.load(board, fen, &error)) {
        std::cout << "FEN load failed: " << error << "\n";
        return false;
    }
    return true;
}

bool hasAnyLegalMove(Board& board) {
    MoveList moves;
    MoveGenerator::nextValidMoves(board, moves);
    for (const auto& move : moves) {
        if (board.makeMove(move)) {
            board.undoMove(move);
            return true;
        }
    }
    return false;
}

bool sideToMoveInCheck(Board& board) {
    int kingSq = board.getPiecePosition(board.whiteToMove ? board.WK : board.BK);
    if (kingSq < 0) {
        return false;
    }
    return board.isSquareAttacked(kingSq, !board.whiteToMove);
}

bool isCheckmate(Board& board) {
    return sideToMoveInCheck(board) && !hasAnyLegalMove(board);
}

bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, out));
}

bool tryParseInt(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    long value = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

bool readInt(const std::string& prompt, int minValue, int maxValue, int& out) {
    std::string line;
    while (true) {
        if (!readLine(prompt, line)) {
            return false;
        }
        if (line == "q" || line == "quit" || line == "exit") {
            return false;
        }
        int value = 0;
        if (!tryParseInt(line, value)) {
            std::cout << "Please enter a valid integer.\n";
            continue;
        }
        if (value < minValue || value > maxValue) {
            std::cout << "Value must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        out = value;
        return true;
    }
}

void printSummary(const TestStats& stats) {
    std::cout << "\nSummary: " << stats.passed << " passed, " << stats.failed
              << " failed, " << stats.skipped << " skipped\n";
}

void runEvaluationTests(TestStats& stats) {
    {
        Board board;
        if (loadFen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
            int score = Evaluator::evaluate(board);
            stats.expectEq(score, 0, "Eval start position is 0");
        } else {
            stats.expect(false, "Eval start position setup");
        }
    }
    {
        Board board;
        if (loadFen(board, "6k1/8/8/8/8/8/8/6KQ w - - 0 1")) {
            int score = Evaluator::evaluate(board);
            stats.expect(score > 500, "Eval white up queen is positive");
        } else {
            stats.expect(false, "Eval queen-up (white) setup");
        }
    }
    {
        Board board;
        if (loadFen(board, "6k1/8/8/8/8/8/8/6KQ b - - 0 1")) {
            int score = Evaluator::evaluate(board);
            stats.expect(score < -500, "Eval black to move is negative");
        } else {
            stats.expect(false, "Eval queen-up (black) setup");
        }
    }
}

void runSearchTests(TestStats& stats, int depth) {
    Board board;
    if (!loadFen(board, "7k/5K2/6Q1/8/8/8/8/8 w - - 0 1")) {
        stats.expect(false, "Search mate-in-1 setup");
        return;
    }
    board.trackHistory = true;

    Search search;
    SearchResult result = search.bestMove(board, depth);
    bool legal = board.makeMove(result.bestMove);
    stats.expect(legal, "Search returns legal move");
    if (legal) {
        stats.expect(isCheckmate(board), "Search finds mate in 1");
        board.undoMove(result.bestMove);
    }
}

bool runPerftTest(const PerftTest& test, int maxDepth, TestStats& stats, double& elapsedSeconds) {
    Board board;
    FenLoader loader;
    std::string error;

    if (!loader.load(board, test.fen, &error)) {
        stats.expect(false, std::string("Perft load: ") + test.name);
        return false;
    }

    board.trackHistory = false;

    bool ok = true;
    std::cout << "\n== " << test.name << " ==\n";
    const auto testStart = std::chrono::steady_clock::now();

    for (const auto& entry : test.expected) {
        if (maxDepth >= 0 && entry.depth > maxDepth) {
            continue;
        }

        unsigned long long expected = 0;
        if (!parseU64(entry.nodes, expected)) {
            stats.skip(std::string(test.name) + " depth " + std::to_string(entry.depth));
            continue;
        }

        const auto depthStart = std::chrono::steady_clock::now();
        unsigned long long actual = static_cast<unsigned long long>(Perft::run(board, entry.depth));
        const auto depthEnd = std::chrono::steady_clock::now();
        const std::chrono::duration<double> depthElapsed = depthEnd - depthStart;

        std::string label = std::string(test.name) + " depth " + std::to_string(entry.depth);
        if (actual == expected) {
            stats.expect(true, label + " (" + std::to_string(actual) + ")");
        } else {
            stats.expect(false, label + " (got " + std::to_string(actual) +
                             ", expected " + std::to_string(expected) + ")");
            ok = false;
        }

        std::cout << "time: " << std::fixed << std::setprecision(6)
                  << depthElapsed.count() << " s\n";
    }

    const auto testEnd = std::chrono::steady_clock::now();
    const std::chrono::duration<double> testElapsed = testEnd - testStart;
    elapsedSeconds = testElapsed.count();
    std::cout << "Position time: " << std::fixed << std::setprecision(6)
              << elapsedSeconds << " s\n";
    return ok;
}

void runPerftSuite(TestStats& stats, int maxDepth, int selection) {
    double totalSeconds = 0.0;
    int failed = 0;
    int startIndex = 0;
    int endIndex = static_cast<int>(kPerftTests.size());

    if (selection >= 0) {
        startIndex = selection;
        endIndex = selection + 1;
    }

    for (int i = startIndex; i < endIndex; ++i) {
        double elapsedSeconds = 0.0;
        if (!runPerftTest(kPerftTests[i], maxDepth, stats, elapsedSeconds)) {
            failed++;
        }
        totalSeconds += elapsedSeconds;
    }

    std::cout << "\nPerft summary: " << (endIndex - startIndex - failed)
              << " passed, " << failed << " failed (max depth " << maxDepth << ")\n";
    std::cout << "Perft total time: " << std::fixed << std::setprecision(6)
              << totalSeconds << " s\n";
}

void runPerftCustom() {
    std::string fen;
    if (!readLine("Enter FEN (or 'q' to cancel): ", fen)) {
        return;
    }
    if (fen == "q" || fen == "quit" || fen == "exit") {
        return;
    }
    int depth = 0;
    if (!readInt("Enter depth: ", 0, 20, depth)) {
        return;
    }

    Board board;
    if (!loadFen(board, fen.c_str())) {
        return;
    }

    board.trackHistory = false;

    const auto start = std::chrono::steady_clock::now();
    unsigned long long nodes = static_cast<unsigned long long>(Perft::run(board, depth));
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end - start;

    std::cout << "Perft nodes: " << nodes << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(6)
              << elapsed.count() << " s\n";
}

int choosePerftPosition() {
    std::cout << "\nPerft positions:\n";
    std::cout << "  0) All positions\n";
    for (size_t i = 0; i < kPerftTests.size(); ++i) {
        std::cout << "  " << (i + 1) << ") " << kPerftTests[i].name << "\n";
    }
    int choice = 0;
    if (!readInt("Choose position: ", 0, static_cast<int>(kPerftTests.size()), choice)) {
        return -2;
    }
    if (choice == 0) {
        return -1;
    }
    return choice - 1;
}

void printMenu() {
    std::cout << "\n==== Engine Tests ===="
              << "\n1) Run all tests"
              << "\n2) Perft suite (built-in positions)"
              << "\n3) Perft custom FEN"
              << "\n4) Evaluation sanity tests"
              << "\n5) Search sanity tests"
              << "\n0) Exit\n";
}

} // namespace

int main() {
    while (true) {
        printMenu();
        int choice = 0;
        if (!readInt("Select option: ", 0, 5, choice)) {
            break;
        }

        if (choice == 0) {
            break;
        }

        if (choice == 1) {
            int perftDepth = 0;
            if (!readInt("Enter perft max depth: ", 0, 20, perftDepth)) {
                continue;
            }
            int searchDepth = 1;
            if (!readInt("Enter search depth: ", 1, 20, searchDepth)) {
                continue;
            }
            TestStats stats;
            runEvaluationTests(stats);
            runPerftSuite(stats, perftDepth, -1);
            runSearchTests(stats, searchDepth);
            printSummary(stats);
            continue;
        }

        if (choice == 2) {
            int selection = choosePerftPosition();
            if (selection == -2) {
                continue;
            }
            int perftDepth = 0;
            if (!readInt("Enter perft max depth: ", 0, 20, perftDepth)) {
                continue;
            }
            TestStats stats;
            runPerftSuite(stats, perftDepth, selection);
            printSummary(stats);
            continue;
        }

        if (choice == 3) {
            runPerftCustom();
            continue;
        }

        if (choice == 4) {
            TestStats stats;
            runEvaluationTests(stats);
            printSummary(stats);
            continue;
        }

        if (choice == 5) {
            int searchDepth = 1;
            if (!readInt("Enter search depth: ", 1, 20, searchDepth)) {
                continue;
            }
            TestStats stats;
            runSearchTests(stats, searchDepth);
            printSummary(stats);
            continue;
        }
    }

    return 0;
}
