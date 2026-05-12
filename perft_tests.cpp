#include "board.h"
#include "fenLoader.h"
#include "perft.h"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct PerftExpected {
    int depth;
    const char* nodes;
};

struct PerftTest {
    const char* name;
    const char* fen;
    std::vector<PerftExpected> expected;
};

static bool parseU64(const char* text, unsigned long long& out) {
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

static int parseMaxDepth(int argc, char** argv) {
    if (argc < 2) {
        return 6;
    }
    char* end = nullptr;
    long value = std::strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0') {
        return 6;
    }
    return static_cast<int>(value);
}

static bool runPerftTest(const PerftTest& test, int maxDepth, double& elapsedSeconds) {
    Board board;
    board.trackHistory=0;
    FenLoader loader;
    std::string error;

    if (!loader.load(board, test.fen, &error)) {
        std::cout << "[FAIL] " << test.name << " - FEN load error: " << error << "\n";
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
            std::cout << "depth " << entry.depth << ": skip (expected too large for 64-bit: "
                      << entry.nodes << ")\n";
            continue;
        }

        const auto depthStart = std::chrono::steady_clock::now();
        unsigned long long actual = static_cast<unsigned long long>(Perft::run(board, entry.depth));
        const auto depthEnd = std::chrono::steady_clock::now();
        const std::chrono::duration<double> depthElapsed = depthEnd - depthStart;
        if (actual == expected) {
            std::cout << "depth " << entry.depth << ": ok (" << actual << ")"
                      << " | time: " << std::fixed << std::setprecision(6)
                      << depthElapsed.count() << " s\n";
        } else {
            std::cout << "depth " << entry.depth << ": FAIL (got " << actual
                      << ", expected " << expected << ")"
                      << " | time: " << std::fixed << std::setprecision(6)
                      << depthElapsed.count() << " s\n";
            ok = false;
        }
    }

    const auto testEnd = std::chrono::steady_clock::now();
    const std::chrono::duration<double> testElapsed = testEnd - testStart;
    elapsedSeconds = testElapsed.count();
    std::cout << "Position time: " << std::fixed << std::setprecision(6)
              << elapsedSeconds << " s\n";
    return ok;
}

int main(int argc, char** argv) {
    const int maxDepth = parseMaxDepth(argc, argv);

    const std::vector<PerftTest> tests = {
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

    int failed = 0;
    double totalSeconds = 0.0;
    std::vector<double> perTestSeconds;
    perTestSeconds.reserve(tests.size());
    for (const auto& test : tests) {
        double elapsedSeconds = 0.0;
        if (!runPerftTest(test, maxDepth, elapsedSeconds)) {
            failed++;
        }
        perTestSeconds.push_back(elapsedSeconds);
        totalSeconds += elapsedSeconds;
    }

    std::cout << "\nSummary: " << (tests.size() - failed) << " passed, "
              << failed << " failed (max depth " << maxDepth << ")\n";
    std::cout << "Timing breakdown:\n";
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "  " << tests[i].name << ": " << std::fixed
                  << std::setprecision(6) << perTestSeconds[i] << " s\n";
    }
    std::cout << "  Total: " << std::fixed << std::setprecision(6)
              << totalSeconds << " s\n";

    return failed == 0 ? 0 : 1;
}
