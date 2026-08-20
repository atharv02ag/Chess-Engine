#include "../include/engine.h"
#include "../include/game.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct test_case {
    std::string fen;
    std::string expected_move;
};

const std::vector<test_case> positions = {
    {"r5k1/1p3ppp/6q1/pPPp1b2/3Nr3/4P2P/P1Q2PPK/1R3R2 b - - 4 20", "e4d4"},
    {"5r1k/4r2p/2qpB3/B1p1bPP1/P3R1QP/1P6/6K1/8 b - - 0 39", "e7e6"},
    {"4r1k1/1p3pp1/p2Qp3/3p1qBp/3P3P/PP6/2n2PP1/2R3K1 w - - 4 28", "d6c5"},
    {"r3k2r/ppp2ppp/3p4/2b5/4P2q/2N2b1P/PPP1BP2/R1B1QRK1 b kq - 1 13", "h4g3"},
};

std::string move_text(const move &mv) {
    if (mv.castle_kside())
        return "O-O";
    if (mv.castle_qside())
        return "O-O-O";
    if (!mv.has_coordinates())
        return "(none)";
    const int start = mv.start_square();
    const int end = mv.end_square();
    std::string result;
    result += static_cast<char>('a' + start % 8);
    result += static_cast<char>('1' + start / 8);
    result += static_cast<char>('a' + end % 8);
    result += static_cast<char>('1' + end / 8);
    switch (mv.promotion_piece()) {
    case PIECE::QUEEN:
        result += 'q';
        break;
    case PIECE::ROOK:
        result += 'r';
        break;
    case PIECE::BISHOP:
        result += 'b';
        break;
    case PIECE::KNIGHT:
        result += 'n';
        break;
    default:
        break;
    }
    return result;
}

} // namespace

int main() {
    constexpr int depth = 8;
    int passed = 0;
    const auto suite_started = std::chrono::steady_clock::now();

    std::cout << "=== Tactical best-move tests ===\n"
              << "Cases: " << positions.size() << " | Search depth: " << depth << "\n\n";

    for (std::size_t index = 0; index < positions.size(); ++index) {
        const test_case &test = positions[index];
        game<std_rules> position(test.fen);
        const move expected = position.parse_move(test.expected_move);
        engine<std_rules> search;

        const auto started = std::chrono::steady_clock::now();
        const auto [actual, evaluation] = search.minimax_tt_wrap(position.chess_board, position.turn, depth);
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        const bool success = actual == expected;
        passed += success ? 1 : 0;

        std::cout << '[' << (success ? "PASS" : "FAIL") << "] Test " << index + 1 << '/' << positions.size() << '\n'
                  << "  FEN:      " << test.fen << '\n'
                  << "  Expected: " << test.expected_move << '\n'
                  << "  Actual:   " << move_text(actual) << '\n'
                  << "  Eval:     " << std::fixed << std::setprecision(2) << evaluation << '\n'
                  << "  Search:   " << std::setprecision(3) << seconds << " s | " << search.nodes_seen << " nodes | "
                  << search.tt_hits << " TT hits";
        if (seconds > 0.0)
            std::cout << " | " << static_cast<unsigned long long>(search.nodes_seen / seconds) << " nps";
        std::cout << "\n\n" << std::flush;
    }

    const double total_seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - suite_started).count();
    const int failed = static_cast<int>(positions.size()) - passed;
    std::cout << "=== Summary ===\n"
              << "Passed: " << passed << " | Failed: " << failed << " | Total: " << positions.size()
              << " | Elapsed: " << std::fixed << std::setprecision(3) << total_seconds << " s\n";
    return failed == 0 ? 0 : 1;
}
