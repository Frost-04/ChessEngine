#include "fenLoader.h"
#include "board.h"
#include "constants.h"
#include <limits>
#include <vector>

bool FenLoader::isValid(std::string_view fen, std::string* errorMessage) {
    ParsedFen parsed;
    return parse(fen, parsed, errorMessage);
}

bool FenLoader::load(Board& board, std::string_view fen, std::string* errorMessage) {
    ParsedFen parsed;
    if (!parse(fen, parsed, errorMessage)) {
        return false;
    }
    applyToBoard(board, parsed);
    return true;
}

bool FenLoader::parse(std::string_view fen, ParsedFen& out, std::string* errorMessage) {
    // Split by spaces into 6 fields.
    std::vector<std::string_view> parts;
    parts.reserve(6);
    std::size_t start = 0;
    while (start < fen.size()) {
        std::size_t end = fen.find(' ', start);
        if (end == std::string_view::npos) {
            end = fen.size();
        }
        if (end > start) {
            parts.push_back(fen.substr(start, end - start));
        }
        start = end + 1;
    }
    if (parts.size() != 6) {
        setError(errorMessage, "FEN must have 6 space-separated fields");
        return false;
    }

    // 1) Piece placement
    int rank = 7;
    int file = 0;
    for (char ch : parts[0]) {
        if (ch == '/') {
            if (file != 8) {
                setError(errorMessage, "Each rank must have exactly 8 files");
                return false;
            }
            rank--;
            file = 0;
            if (rank < 0) {
                setError(errorMessage, "Too many ranks in piece placement");
                return false;
            }
            continue;
        }
        if (ch >= '1' && ch <= '8') {
            file += (ch - '0');
            if (file > 8) {
                setError(errorMessage, "Rank has more than 8 files");
                return false;
            }
            continue;
        }
        if (file >= 8) {
            setError(errorMessage, "Rank has more than 8 files");
            return false;
        }
        int squareIndex = rank * 8 + file;
        if (!addPieceBit(out, ch, squareIndex)) {
            setError(errorMessage, "Invalid piece character in placement field");
            return false;
        }
        file++;
    }
    if (rank != 0 || file != 8) {
        setError(errorMessage, "Piece placement must have 8 ranks of 8 files");
        return false;
    }

    // Basic king sanity: exactly one each
    if (__builtin_popcountll(out.WK) != 1 || __builtin_popcountll(out.BK) != 1) {
        setError(errorMessage, "FEN must contain exactly one white king and one black king");
        return false;
    }

    // 2) Active color
    if (parts[1] == "w") {
        out.whiteToMove = true;
    } else if (parts[1] == "b") {
        out.whiteToMove = false;
    } else {
        setError(errorMessage, "Active color must be 'w' or 'b'");
        return false;
    }

    // 3) Castling availability
    out.whiteCastleKingSide = false;
    out.whiteCastleQueenSide = false;
    out.blackCastleKingSide = false;
    out.blackCastleQueenSide = false;
    if (parts[2] != "-") {
        for (char ch : parts[2]) {
            switch (ch) {
                case 'K': out.whiteCastleKingSide = true; break;
                case 'Q': out.whiteCastleQueenSide = true; break;
                case 'k': out.blackCastleKingSide = true; break;
                case 'q': out.blackCastleQueenSide = true; break;
                default:
                    setError(errorMessage, "Invalid castling rights field");
                    return false;
            }
        }
    }

    // 4) En passant target square
    if (parts[3] == "-") {
        out.enPassantSquare = NO_SQUARE;
    } else if (parts[3].size() == 2) {
        char fileChar = parts[3][0];
        char rankChar = parts[3][1];
        if (fileChar < 'a' || fileChar > 'h' || rankChar < '1' || rankChar > '8') {
            setError(errorMessage, "Invalid en passant square");
            return false;
        }
        int fileIndex = fileChar - 'a';
        int rankIndex = rankChar - '1';
        out.enPassantSquare = static_cast<std::uint8_t>(rankIndex * 8 + fileIndex);
    } else {
        setError(errorMessage, "Invalid en passant field");
        return false;
    }

    // 5) Halfmove clock
    int halfmove = 0;
    if (!parseUnsignedInt(parts[4], halfmove) || halfmove < 0 || halfmove > 255) {
        setError(errorMessage, "Invalid halfmove clock");
        return false;
    }
    out.fiftyMoveClock = static_cast<std::uint8_t>(halfmove);

    // 6) Fullmove number
    int fullmove = 0;
    if (!parseUnsignedInt(parts[5], fullmove) || fullmove <= 0) {
        setError(errorMessage, "Invalid fullmove number");
        return false;
    }
    out.moveCount = fullmove;

    return true;
}

bool FenLoader::parseUnsignedInt(std::string_view text, int& out) {
    if (text.empty()) {
        return false;
    }
    int value = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        int digit = ch - '0';
        if (value > (std::numeric_limits<int>::max() - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    out = value;
    return true;
}

void FenLoader::setError(std::string* errorMessage, const std::string& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

bool FenLoader::addPieceBit(ParsedFen& out, char piece, int squareIndex) {
    std::uint64_t bit = 1ULL << squareIndex;
    switch (piece) {
        case 'P': out.WP |= bit; return true;
        case 'N': out.WN |= bit; return true;
        case 'B': out.WB |= bit; return true;
        case 'R': out.WR |= bit; return true;
        case 'Q': out.WQ |= bit; return true;
        case 'K': out.WK |= bit; return true;
        case 'p': out.BP |= bit; return true;
        case 'n': out.BN |= bit; return true;
        case 'b': out.BB |= bit; return true;
        case 'r': out.BR |= bit; return true;
        case 'q': out.BQ |= bit; return true;
        case 'k': out.BK |= bit; return true;
        default:  return false;
    }
}

void FenLoader::applyToBoard(Board& board, const ParsedFen& parsed) {
    board.WP = parsed.WP;
    board.WN = parsed.WN;
    board.WB = parsed.WB;
    board.WR = parsed.WR;
    board.WQ = parsed.WQ;
    board.WK = parsed.WK;

    board.BP = parsed.BP;
    board.BN = parsed.BN;
    board.BB = parsed.BB;
    board.BR = parsed.BR;
    board.BQ = parsed.BQ;
    board.BK = parsed.BK;

    board.NULL_PIECE = 0;
    board.enPassantSquare = parsed.enPassantSquare;

    board.whiteToMove = parsed.whiteToMove;
    board.whiteCastleKingSide = parsed.whiteCastleKingSide;
    board.whiteCastleQueenSide = parsed.whiteCastleQueenSide;
    board.blackCastleKingSide = parsed.blackCastleKingSide;
    board.blackCastleQueenSide = parsed.blackCastleQueenSide;

    board.blackInCheck = false;
    board.whiteInCheck = false;
    board.fiftyMoveClock = parsed.fiftyMoveClock;
    board.moveCount = parsed.moveCount;
    board.isInsufficientMaterial = false;

    board.undoStack = std::stack<Undo>();
    board.validMoves.clear();

    // Refresh check state after loading.
    int whiteKingSq = board.getPiecePosition(board.WK);
    int blackKingSq = board.getPiecePosition(board.BK);
    if (whiteKingSq >= 0) {
        board.whiteInCheck = board.isSquareAttacked(whiteKingSq, false);
    }
    if (blackKingSq >= 0) {
        board.blackInCheck = board.isSquareAttacked(blackKingSq, true);
    }
    //adding zorbist things
    board.zorbistHash = board.computeInitialHash();
    board.positionHistory.clear();
    board.positionHistory[board.zorbistHash]++;
}
