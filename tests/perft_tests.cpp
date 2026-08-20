#include "../include/engine.h"
#include "../include/game.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

using node_count = std::uint64_t;

int failures = 0;

COLOUR opposite(COLOUR colour) {
    return colour == COLOUR::WHITE ? COLOUR::BLACK : COLOUR::WHITE;
}

void apply_move(board &position, const move &mv, std_rules &rules) {
    const COLOUR next_turn = opposite(mv.colour());
    position.zhash ^= zobrist::rv_colour;
    position.turn = next_turn;

    if (mv.castle_kside())
        rules.castle_kside(position, mv.colour());
    else if (mv.castle_qside())
        rules.castle_qside(position, mv.colour());
    else
        position.apply_move(mv);

    rules.update_flags(position, mv);
}

bool hash_is_consistent(const board &position) {
    board recomputed = position;
    recomputed.init_hash();
    return recomputed.zhash == position.zhash;
}

node_count perft(const board &position, COLOUR turn, int depth, std_rules &rules, bool *hashes_valid = nullptr) {
    if (hashes_valid && !hash_is_consistent(position))
        *hashes_valid = false;
    if (depth == 0)
        return 1;

    const std::vector<move> moves = rules.get_all_legal_moves(position, turn);
    node_count nodes = 0;
    for (const move &mv : moves) {
        board next = position;
        apply_move(next, mv, rules);
        nodes += perft(next, opposite(turn), depth - 1, rules, hashes_valid);
    }
    return nodes;
}

template <typename actual_type, typename expected_type>
void expect_equal(const std::string &name, const actual_type &actual, const expected_type &expected) {
    if (actual == expected) {
        std::cout << "[PASS] " << name << " = " << actual << '\n';
        return;
    }
    ++failures;
    std::cerr << "[FAIL] " << name << ": expected " << expected << ", got " << actual << '\n';
}

void expect_true(const std::string &name, bool condition) {
    expect_equal(name, condition, true);
}

void expect_near(const std::string &name, float actual, float expected, float tolerance = 0.0001f) {
    if (std::fabs(actual - expected) <= tolerance) {
        std::cout << "[PASS] " << name << " = " << actual << '\n';
        return;
    }
    ++failures;
    std::cerr << "[FAIL] " << name << ": expected " << expected << ", got " << actual << '\n';
}

void run_move_packing_tests() {
    expect_equal("packed move size", sizeof(move), std::size_t{4});

    // White pawn e7xd8=Q: from=52, to=59, piece=1, colour=0,
    // flags=CAPTURE|PROMOTE_QUEEN, coordinates-present=1.
    const move promotion_capture(PIECE::PAWN, COLOUR::WHITE, 1ULL << 52, 1ULL << 59, true, PIECE::QUEEN);
    const u32 expected = 52U | (59U << 6) | (1U << 12) |
                         ((static_cast<u32>(MOVE_FLAGS::CAPTURE) |
                           static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN))
                          << 17) |
                         (1U << 25);
    expect_equal("packed move bit layout", promotion_capture.packed(), expected);
    expect_equal("packed source square", promotion_capture.start_square(), 52);
    expect_equal("packed destination square", promotion_capture.end_square(), 59);
    expect_true("packed piece", promotion_capture.piece() == PIECE::PAWN);
    expect_true("packed colour", promotion_capture.colour() == COLOUR::WHITE);
    expect_true("packed capture flag", promotion_capture.piece_captured());
    expect_true("packed promotion flag", promotion_capture.promotion_piece() == PIECE::QUEEN);

    const move unpacked = move::unpack(promotion_capture.packed());
    expect_true("packed move round trip", unpacked == promotion_capture);
    expect_equal("packed source bitboard round trip", unpacked.start_location(), 1ULL << 52);
    expect_equal("packed destination bitboard round trip", unpacked.end_location(), 1ULL << 59);

    const move black_queenside_castle(PIECE::KING, COLOUR::BLACK, false, true);
    expect_true("packed castle flag", black_queenside_castle.castle_qside());
    expect_equal("packed castle source", black_queenside_castle.start_square(), 60);
    expect_equal("packed castle destination", black_queenside_castle.end_square(), 58);
    expect_true("packed castle round trip",
                move::unpack(black_queenside_castle.packed()) == black_queenside_castle);

    const move empty;
    expect_true("empty packed move piece", empty.piece() == PIECE::EMPTY);
    expect_true("empty packed move colour", empty.colour() == COLOUR::NONE);
    expect_equal("empty packed move source bitboard", empty.start_location(), 0ULL);
    expect_equal("empty packed move destination bitboard", empty.end_location(), 0ULL);
}

bool contains_kingside_castle(const std::vector<move> &moves) {
    for (const move &mv : moves) {
        if (mv.castle_kside())
            return true;
    }
    return false;
}

void run_perft_case(const std::string &name, const std::string &fen, const std::vector<node_count> &expected) {
    board position;
    position.load_fen(fen);
    std_rules rules;

    for (std::size_t index = 0; index < expected.size(); ++index) {
        const int depth = static_cast<int>(index + 1);
        const auto start = std::chrono::steady_clock::now();
        const node_count actual = perft(position, position.turn, depth, rules);
        const auto end = std::chrono::steady_clock::now();
        expect_equal(name + " depth " + std::to_string(depth), actual, expected[index]);
        std::cout << "       " << std::chrono::duration<double>(end - start).count() << " s\n";
    }
}

void run_perft_tests() {
    run_perft_case("start position",
                   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                   {20, 400, 8902, 197281, 4865609});

    run_perft_case("kiwipete",
                   "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                   {48, 2039, 97862, 4085603});

    run_perft_case("position 3",
                   "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                   {14, 191, 2812, 43238, 674624});

    run_perft_case("position 4",
                   "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                   {6, 264, 9467, 422333});

    run_perft_case("position 5",
                   "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
                   {44, 1486, 62379, 2103487});

    run_perft_case("position 6",
                   "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
                   {46, 2079, 89890, 3894594});
}

void run_hash_tests() {
    std_rules rules;

    board partial_rights;
    partial_rights.load_fen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
    const move rook_move(PIECE::ROOK, COLOUR::WHITE, 1ULL, 1ULL << 8);
    partial_rights.apply_move(rook_move);
    rules.update_flags(partial_rights, rook_move);
    expect_true("hash after one castling right is removed", hash_is_consistent(partial_rights));

    const move king_move(PIECE::KING, COLOUR::WHITE, 1ULL << 4, 1ULL << 12);
    partial_rights.apply_move(king_move);
    rules.update_flags(partial_rights, king_move);
    expect_true("hash after king moves with one remaining right", hash_is_consistent(partial_rights));

    game<std_rules> castling_game("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    expect_true("game accepts legal kingside castle", castling_game.make_move(castling_game.parse_move("K")));
    expect_true("hash after game-level castling", hash_is_consistent(castling_game.chess_board));

    board reloaded;
    reloaded.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    reloaded.load_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    expect_equal("piece count after reloading FEN", pop_ct(reloaded.all_pieces()), 2);
    expect_true("hash after reloading FEN", hash_is_consistent(reloaded));

    board hash_walk;
    hash_walk.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    bool hashes_valid = true;
    perft(hash_walk, hash_walk.turn, 3, rules, &hashes_valid);
    expect_true("incremental hashes across kiwipete depth 3", hashes_valid);
}

void run_castling_tests() {
    std_rules rules;

    game<std_rules> captured_rook("4k3/8/8/8/8/8/5n2/4K2R b K - 0 1");
    expect_true("rook capture is legal", captured_rook.make_move(captured_rook.parse_move("f2h1")));
    expect_equal("rook capture revokes castling right",
                 static_cast<bool>(captured_rook.chess_board.flags &
                                   static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)),
                 false);
    expect_equal("castling is absent after rook capture",
                 contains_kingside_castle(rules.get_all_legal_moves(captured_rook.chess_board, captured_rook.turn)),
                 false);

    board occupied_path;
    occupied_path.load_fen("4k3/8/8/8/8/8/8/4Kn1R w K - 0 1");
    expect_equal("castling through enemy-occupied transit square",
                 contains_kingside_castle(rules.get_all_legal_moves(occupied_path, COLOUR::WHITE)),
                 false);

    board missing_rook;
    missing_rook.load_fen("4k3/8/8/8/8/8/8/4K3 w K - 0 1");
    expect_equal("castling without a rook",
                 contains_kingside_castle(rules.get_all_legal_moves(missing_rook, COLOUR::WHITE)),
                 false);
}

void run_quiescence_tests() {
    engine<std_rules> search;

    board checked;
    checked.load_fen("4r1k1/8/8/8/8/8/8/4K3 w - - 0 1");
    expect_near("checked side must search quiet evasions",
                search.base_case_minimax(checked, COLOUR::WHITE, 0, -INF, INF),
                -52.5f);

    board stalemate;
    stalemate.load_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    expect_near("quiescence stalemate", search.base_case_minimax(stalemate, COLOUR::BLACK, 0, -INF, INF), 0.0f);

    board promotion;
    promotion.load_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    const std::vector<move> tactical = search.rules.get_all_legal_tactical_moves(promotion, COLOUR::WHITE);
    expect_equal("quiet promotions are tactical", tactical.size(), std::size_t{4});
    bool all_promotions = true;
    for (const move &mv : tactical)
        all_promotions = all_promotions && mv.promotion_piece() != PIECE::EMPTY;
    expect_true("tactical promotion move flags", all_promotions);
}

} // namespace

int main() {
    run_move_packing_tests();
    run_perft_tests();
    run_hash_tests();
    run_castling_tests();
    run_quiescence_tests();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << "All perft and regression tests passed\n";
    return 0;
}
