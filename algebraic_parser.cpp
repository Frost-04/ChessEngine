#include "algebraic_parser.h"
#include "board.h"
#include "movegenerator.h"
#include "constants.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct ParsedSan {
    Piece movingPiece = Piece::None;
    int to = -1;
    bool isCapture = false;
    bool isPromotion = false;
    Piece promotionPiece = Piece::None;
    bool castleKing = false;
    bool castleQueen = false;
    char disambigFile = '\0';
    char disambigRank = '\0';
};

static void setError(std::string* errorMessage, const std::string& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

static bool isFileChar(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    char lc = static_cast<char>(std::tolower(uc));
    return lc >= 'a' && lc <= 'h';
}

static bool isRankChar(char c) {
    return c >= '1' && c <= '8';
}

static bool parseSquareIndex(char fileChar, char rankChar, int& outIndex) {
    unsigned char uc = static_cast<unsigned char>(fileChar);
    char lc = static_cast<char>(std::tolower(uc));
    if (!isFileChar(lc) || !isRankChar(rankChar)) {
        return false;
    }
    int file = lc - 'a';
    int rank = rankChar - '1';
    outIndex = rank * 8 + file;
    return true;
}

static bool pieceFromChar(bool whiteToMove, char c, Piece& outPiece) {
    unsigned char uc = static_cast<unsigned char>(c);
    char up = static_cast<char>(std::toupper(uc));
    switch (up) {
        case 'K': outPiece = whiteToMove ? Piece::WK : Piece::BK; return true;
        case 'Q': outPiece = whiteToMove ? Piece::WQ : Piece::BQ; return true;
        case 'R': outPiece = whiteToMove ? Piece::WR : Piece::BR; return true;
        case 'B': outPiece = whiteToMove ? Piece::WB : Piece::BB; return true;
        case 'N': outPiece = whiteToMove ? Piece::WN : Piece::BN; return true;
        default: return false;
    }
}

static bool promotionFromChar(bool whiteToMove, char c, Piece& outPiece) {
    if (!pieceFromChar(whiteToMove, c, outPiece)) {
        return false;
    }
    return outPiece != Piece::WK && outPiece != Piece::BK;
}

static bool isPromotionChar(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    char up = static_cast<char>(std::toupper(uc));
    return up == 'Q' || up == 'R' || up == 'B' || up == 'N';
}

static bool endsWithCaseInsensitive(const std::string& text, const std::string& suffix) {
    if (suffix.size() > text.size()) {
        return false;
    }
    size_t offset = text.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); ++i) {
        unsigned char tc = static_cast<unsigned char>(text[offset + i]);
        unsigned char sc = static_cast<unsigned char>(suffix[i]);
        if (std::toupper(tc) != std::toupper(sc)) {
            return false;
        }
    }
    return true;
}

static void stripTrailingAnnotations(std::string& text) {
    while (!text.empty()) {
        char c = text.back();
        if (c == '+' || c == '#' || c == '!' || c == '?') {
            text.pop_back();
        } else {
            break;
        }
    }

    if (endsWithCaseInsensitive(text, "e.p.")) {
        text.resize(text.size() - 4);
    } else if (endsWithCaseInsensitive(text, "ep")) {
        text.resize(text.size() - 2);
    }

    while (!text.empty()) {
        char c = text.back();
        if (c == '.' || c == '+' || c == '#' || c == '!' || c == '?') {
            text.pop_back();
        } else {
            break;
        }
    }
}

static bool parseDisambiguation(const std::string& text, char& outFile, char& outRank) {
    if (text.empty()) {
        return true;
    }
    if (text.size() > 2) {
        return false;
    }

    char c1 = text[0];
    char c2 = text.size() == 2 ? text[1] : '\0';

    if (text.size() == 1) {
        if (isFileChar(c1)) {
            unsigned char uc = static_cast<unsigned char>(c1);
            outFile = static_cast<char>(std::tolower(uc));
            return true;
        }
        if (isRankChar(c1)) {
            outRank = c1;
            return true;
        }
        return false;
    }

    bool c1File = isFileChar(c1);
    bool c1Rank = isRankChar(c1);
    bool c2File = isFileChar(c2);
    bool c2Rank = isRankChar(c2);

    if (c1File && c2Rank) {
        unsigned char uc = static_cast<unsigned char>(c1);
        outFile = static_cast<char>(std::tolower(uc));
        outRank = c2;
        return true;
    }

    return false;
}

static bool matchesParsedMove(Board& board, const Move& move, const ParsedSan& parsed) {
    if (parsed.castleKing) {
        return (move.flag & CASTLE_KING) != 0;
    }
    if (parsed.castleQueen) {
        return (move.flag & CASTLE_QUEEN) != 0;
    }

    if (move.movingPiece != parsed.movingPiece) {
        return false;
    }
    if (move.to != parsed.to) {
        return false;
    }

    bool moveIsCapture = (move.flag & CAPTURE) != 0 || (move.flag & EN_PASSANT) != 0 || move.capturedPiece != Piece::None;
    if (parsed.isCapture != moveIsCapture) {
        return false;
    }

    if (parsed.isPromotion) {
        if ((move.flag & PROMOTION) == 0) {
            return false;
        }
        if (move.promotionPiece != parsed.promotionPiece) {
            return false;
        }
    } else if ((move.flag & PROMOTION) != 0) {
        return false;
    }

    if (parsed.disambigFile != '\0') {
        int fromFile = move.from % 8;
        if (fromFile != (parsed.disambigFile - 'a')) {
            return false;
        }
    }

    if (parsed.disambigRank != '\0') {
        int fromRank = move.from / 8;
        if (fromRank != (parsed.disambigRank - '1')) {
            return false;
        }
    }

    return true;
}

static bool isLegalMove(Board& board, const Move& move) {
    if (!board.makeMove(move)) {
        return false;
    }
    board.undoMove(move);
    return true;
}

static bool parseSanText(Board& board, std::string_view text, ParsedSan& outParsed, std::string* errorMessage) {
    std::string s(text);
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c) != 0; }), s.end());

    if (s.empty()) {
        setError(errorMessage, "Empty move string");
        return false;
    }

    stripTrailingAnnotations(s);
    if (s.empty()) {
        setError(errorMessage, "Move string is empty after stripping annotations");
        return false;
    }

    std::string castleCheck = s;
    for (char& c : castleCheck) {
        if (c == '0') {
            c = 'O';
        }
        unsigned char uc = static_cast<unsigned char>(c);
        c = static_cast<char>(std::toupper(uc));
    }

    if (castleCheck == "O-O") {
        outParsed.castleKing = true;
        return true;
    }
    if (castleCheck == "O-O-O") {
        outParsed.castleQueen = true;
        return true;
    }

    outParsed.movingPiece = board.whiteToMove ? Piece::WP : Piece::BP;

    size_t eqPos = s.find('=');
    if (eqPos != std::string::npos) {
        if (eqPos + 2 != s.size()) {
            setError(errorMessage, "Invalid promotion notation");
            return false;
        }
        if (!promotionFromChar(board.whiteToMove, s[eqPos + 1], outParsed.promotionPiece)) {
            setError(errorMessage, "Invalid promotion piece");
            return false;
        }
        outParsed.isPromotion = true;
        s.erase(eqPos);
    } else if (s.size() >= 3) {
        char promoChar = s.back();
        char fileChar = s[s.size() - 3];
        char rankChar = s[s.size() - 2];
        if (isPromotionChar(promoChar) && isFileChar(fileChar) && isRankChar(rankChar)) {
            if (!promotionFromChar(board.whiteToMove, promoChar, outParsed.promotionPiece)) {
                setError(errorMessage, "Invalid promotion piece");
                return false;
            }
            outParsed.isPromotion = true;
            s.pop_back();
        }
    }

    if (s.size() < 2) {
        setError(errorMessage, "Missing destination square");
        return false;
    }

    char toFile = s[s.size() - 2];
    char toRank = s[s.size() - 1];
    if (!parseSquareIndex(toFile, toRank, outParsed.to)) {
        setError(errorMessage, "Invalid destination square");
        return false;
    }
    s.erase(s.size() - 2);

    if (!s.empty()) {
        Piece pieceCandidate = Piece::None;
        char lead = s[0];
        if (lead == 'K' || lead == 'Q' || lead == 'R' || lead == 'B' || lead == 'N') {
            if (pieceFromChar(board.whiteToMove, lead, pieceCandidate)) {
                outParsed.movingPiece = pieceCandidate;
                s.erase(0, 1);
            }
        }
    }

    size_t xPos = s.find('x');
    if (xPos == std::string::npos) {
        xPos = s.find('X');
    }
    if (xPos != std::string::npos) {
        outParsed.isCapture = true;
        s.erase(xPos, 1);
    }

    if (!parseDisambiguation(s, outParsed.disambigFile, outParsed.disambigRank)) {
        setError(errorMessage, "Invalid disambiguation");
        return false;
    }

    if (outParsed.movingPiece == Piece::WP || outParsed.movingPiece == Piece::BP) {
        if (outParsed.isCapture) {
            if (outParsed.disambigFile == '\0' || outParsed.disambigRank != '\0') {
                setError(errorMessage, "Invalid pawn capture notation");
                return false;
            }
        } else {
            if (outParsed.disambigFile != '\0' || outParsed.disambigRank != '\0') {
                setError(errorMessage, "Unexpected disambiguation for pawn move");
                return false;
            }
        }
    }

    return true;
}

} // namespace

bool AlgebraicParser::parse(Board& board, std::string_view text, Move& outMove, std::string* errorMessage) {
    ParsedSan parsed;
    if (!parseSanText(board, text, parsed, errorMessage)) {
        return false;
    }

    MoveList moves;
    if (!MoveGenerator::nextValidMoves(board, moves)) {
        setError(errorMessage, "No moves available in position");
        return false;
    }

    std::vector<Move> matches;
    matches.reserve(moves.count);

    for (const Move& move : moves) {
        if (!matchesParsedMove(board, move, parsed)) {
            continue;
        }
        if (!isLegalMove(board, move)) {
            continue;
        }
        matches.push_back(move);
    }

    if (matches.empty()) {
        setError(errorMessage, "No legal move matches the input");
        return false;
    }

    if (matches.size() > 1) {
        setError(errorMessage, "Ambiguous move input");
        return false;
    }

    outMove = matches.front();
    return true;
}
