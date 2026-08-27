#include "../include/engine.h"
#include "../include/game.h"
#include "../include/magic.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace {

using clock_type = std::chrono::steady_clock;
using node_count = std::uint64_t;

struct configuration {
    int depth = 6;
    int runs = 3;
    std::uint64_t magic_iterations = 50'000'000;
};

struct search_sample {
    double seconds = 0.0;
    node_count nodes = 0;
    node_count qnodes = 0;
    node_count tt_probes = 0;
    node_count tt_hits = 0;
    node_count tt_cutoffs = 0;
};

struct attack_input {
    u64 occupied;
    int square;
};

constexpr std::array<std::string_view, 4> benchmark_fens = {
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "r5k1/1p3ppp/6q1/pPPp1b2/3Nr3/4P2P/P1Q2PPK/1R3R2 b - - 4 20",
    "5r1k/4r2p/2qpB3/B1p1bPP1/P3R1QP/1P6/6K1/8 b - - 0 39",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
};

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

node_count perft(const board &position, COLOUR turn, int depth, std_rules &rules) {
    if (depth == 0)
        return 1;
    node_count nodes = 0;
    for (const move &mv : rules.get_all_legal_moves(position, turn)) {
        board next = position;
        apply_move(next, mv, rules);
        nodes += perft(next, opposite(turn), depth - 1, rules);
    }
    return nodes;
}

template <typename T> T median(std::vector<T> values) {
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

std::string move_text(const move &mv) {
    if (!mv.has_coordinates())
        return "(none)";
    const int start = mv.start_square();
    const int end = mv.end_square();
    std::string result;
    result += static_cast<char>('a' + start % 8);
    result += static_cast<char>('1' + start / 8);
    result += static_cast<char>('a' + end % 8);
    result += static_cast<char>('1' + end / 8);
    return result;
}

search_sample run_search_suite(int depth, bool use_tt, std::vector<std::pair<move, float>> &answers) {
    search_sample result;
    answers.clear();
    for (std::string_view fen : benchmark_fens) {
        game<std_rules> position{std::string(fen)};
        engine<std_rules> search;
        search.reset_search_stats();
        const auto started = clock_type::now();
        const auto answer = search.minimax_tt_wrap(position.chess_board, position.turn, depth, use_tt);
        result.seconds += std::chrono::duration<double>(clock_type::now() - started).count();
        result.nodes += search.nodes_seen;
        result.qnodes += search.qnodes_seen;
        result.tt_probes += search.tt_probes;
        result.tt_hits += search.tt_hits;
        result.tt_cutoffs += search.tt_cutoffs;
        answers.push_back(answer);
    }
    return result;
}

void print_search_sample(std::string_view label, int run, const search_sample &sample) {
    const double nps = sample.seconds > 0.0 ? static_cast<double>(sample.nodes) / sample.seconds : 0.0;
    std::cout << "  " << label << " run " << run << ": " << std::fixed << std::setprecision(3)
              << sample.seconds << " s, " << sample.nodes << " nodes, " << std::setprecision(0) << nps << " nps";
    if (sample.tt_probes != 0) {
        const double hit_rate = 100.0 * static_cast<double>(sample.tt_hits) / sample.tt_probes;
        std::cout << ", " << sample.tt_hits << " usable TT hits (" << std::setprecision(2) << hit_rate
                  << "%), " << sample.tt_cutoffs << " TT cutoffs";
    }
    std::cout << '\n';
}

void benchmark_search(const configuration &config) {
    std::cout << "\nSearch benchmark: " << benchmark_fens.size() << " positions at depth " << config.depth << '\n';
    std::vector<search_sample> without_tt;
    std::vector<search_sample> with_tt;
    without_tt.reserve(config.runs);
    with_tt.reserve(config.runs);

    for (int run = 1; run <= config.runs; ++run) {
        std::vector<std::pair<move, float>> plain_answers;
        std::vector<std::pair<move, float>> tt_answers;
        search_sample plain;
        search_sample cached;
        if (run % 2 == 1) {
            plain = run_search_suite(config.depth, false, plain_answers);
            cached = run_search_suite(config.depth, true, tt_answers);
        } else {
            cached = run_search_suite(config.depth, true, tt_answers);
            plain = run_search_suite(config.depth, false, plain_answers);
        }
        for (std::size_t index = 0; index < plain_answers.size(); ++index) {
            if (plain_answers[index].first != tt_answers[index].first ||
                std::fabs(plain_answers[index].second - tt_answers[index].second) > 0.001f) {
                std::cerr << "Search mismatch at position " << index + 1 << ": no-TT "
                          << move_text(plain_answers[index].first) << "/" << plain_answers[index].second << ", TT "
                          << move_text(tt_answers[index].first) << "/" << tt_answers[index].second << '\n';
                std::exit(2);
            }
        }
        without_tt.push_back(plain);
        with_tt.push_back(cached);
        print_search_sample("no TT", run, plain);
        print_search_sample("TT   ", run, cached);
    }

    std::vector<double> plain_times;
    std::vector<double> cached_times;
    for (const search_sample &sample : without_tt)
        plain_times.push_back(sample.seconds);
    for (const search_sample &sample : with_tt)
        cached_times.push_back(sample.seconds);
    const double plain_median = median(plain_times);
    const double cached_median = median(cached_times);
    const search_sample &plain_reference = without_tt.front();
    const search_sample &cached_reference = with_tt.front();
    const double node_reduction = 100.0 * (1.0 - static_cast<double>(cached_reference.nodes) / plain_reference.nodes);
    const double speedup = plain_median / cached_median;
    const double qnode_share = 100.0 * static_cast<double>(cached_reference.qnodes) / cached_reference.nodes;

    std::cout << "  median result: " << std::setprecision(3) << speedup << "x TT speedup, "
              << std::setprecision(2) << node_reduction << "% fewer nodes, " << qnode_share
              << "% of TT-search nodes in quiescence\n";
}

void benchmark_perft(int runs) {
    board position;
    position.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std_rules rules;
    constexpr int depth = 5;
    constexpr node_count expected = 4'865'609;
    std::vector<double> times;
    for (int run = 0; run < runs; ++run) {
        const auto started = clock_type::now();
        const node_count nodes = perft(position, position.turn, depth, rules);
        times.push_back(std::chrono::duration<double>(clock_type::now() - started).count());
        if (nodes != expected) {
            std::cerr << "Perft mismatch: expected " << expected << ", got " << nodes << '\n';
            std::exit(2);
        }
    }
    const double seconds = median(times);
    std::cout << "\nMove-generation benchmark: start-position perft(5)\n"
              << "  " << expected << " verified leaf nodes in " << std::fixed << std::setprecision(3) << seconds
              << " s (" << std::setprecision(0) << static_cast<double>(expected) / seconds << " leaves/s, median of "
              << runs << ")\n";
}

u64 ray_attacks(int square, u64 occupied, const std::array<std::pair<int, int>, 4> &directions) {
    u64 attacks = 0;
    const int start_rank = square / 8;
    const int start_file = square % 8;
    for (const auto &[rank_delta, file_delta] : directions) {
        int rank = start_rank + rank_delta;
        int file = start_file + file_delta;
        while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
            const u64 target = 1ULL << (rank * 8 + file);
            attacks |= target;
            if (occupied & target)
                break;
            rank += rank_delta;
            file += file_delta;
        }
    }
    return attacks;
}

u64 rook_ray_attacks(int square, u64 occupied) {
    static constexpr std::array<std::pair<int, int>, 4> directions = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    return ray_attacks(square, occupied, directions);
}

u64 bishop_ray_attacks(int square, u64 occupied) {
    static constexpr std::array<std::pair<int, int>, 4> directions = {{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
    return ray_attacks(square, occupied, directions);
}

u64 rook_magic_attacks(int square, u64 occupied) {
    const u64 blockers = occupied & rook_movement_masks_no_edges[square];
    const std::size_t index = (blockers * rook_magic_keys[square]) >> rook_shifts[square];
    return rook_table[square][index];
}

u64 bishop_magic_attacks(int square, u64 occupied) {
    const u64 blockers = occupied & bishop_movement_masks_no_edges[square];
    const std::size_t index = (blockers * bishop_magic_keys[square]) >> bishop_shifts[square];
    return bishop_table[square][index];
}

std::vector<attack_input> make_attack_inputs() {
    constexpr std::size_t count = 1U << 16;
    std::vector<attack_input> inputs;
    inputs.reserve(count);
    u64 state = 0x9e3779b97f4a7c15ULL;
    for (std::size_t index = 0; index < count; ++index) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        inputs.push_back({state, static_cast<int>((state >> 58) & 63ULL)});
    }
    return inputs;
}

template <typename RookFunction, typename BishopFunction>
double time_attacks(const std::vector<attack_input> &inputs, std::uint64_t iterations,
                    RookFunction rook_function, BishopFunction bishop_function, u64 &checksum) {
    u64 sum = 0;
    const std::size_t mask = inputs.size() - 1;
    const auto started = clock_type::now();
    for (std::uint64_t index = 0; index < iterations; ++index) {
        const attack_input &input = inputs[static_cast<std::size_t>(index) & mask];
        sum ^= rook_function(input.square, input.occupied);
        sum ^= bishop_function(input.square, input.occupied);
    }
    const double seconds = std::chrono::duration<double>(clock_type::now() - started).count();
    checksum ^= sum;
    return seconds;
}

void benchmark_magic(const configuration &config) {
    const std::vector<attack_input> inputs = make_attack_inputs();
    for (const attack_input &input : inputs) {
        if (rook_magic_attacks(input.square, input.occupied) != rook_ray_attacks(input.square, input.occupied) ||
            bishop_magic_attacks(input.square, input.occupied) != bishop_ray_attacks(input.square, input.occupied)) {
            std::cerr << "Magic lookup validation failed\n";
            std::exit(2);
        }
    }

    std::vector<double> ray_times;
    std::vector<double> magic_times;
    u64 ray_checksum = 0;
    u64 magic_checksum = 0;
    for (int run = 0; run < config.runs; ++run) {
        if (run % 2 == 0) {
            ray_times.push_back(time_attacks(inputs, config.magic_iterations, rook_ray_attacks, bishop_ray_attacks,
                                             ray_checksum));
            magic_times.push_back(time_attacks(inputs, config.magic_iterations, rook_magic_attacks,
                                               bishop_magic_attacks, magic_checksum));
        } else {
            magic_times.push_back(time_attacks(inputs, config.magic_iterations, rook_magic_attacks,
                                               bishop_magic_attacks, magic_checksum));
            ray_times.push_back(time_attacks(inputs, config.magic_iterations, rook_ray_attacks, bishop_ray_attacks,
                                             ray_checksum));
        }
    }

    if (ray_checksum != magic_checksum) {
        std::cerr << "Sliding-attack benchmark checksums differ\n";
        std::exit(2);
    }

    const double ray_median = median(ray_times);
    const double magic_median = median(magic_times);
    const double calls = 2.0 * static_cast<double>(config.magic_iterations);
    std::cout << "\nSliding-attack microbenchmark: " << static_cast<std::uint64_t>(calls) << " attack queries/run\n"
              << "  ray walking: " << std::fixed << std::setprecision(3) << ray_median << " s ("
              << std::setprecision(0) << calls / ray_median << " queries/s)\n"
              << "  magic table: " << std::setprecision(3) << magic_median << " s (" << std::setprecision(0)
              << calls / magic_median << " queries/s)\n"
              << "  median result: " << std::setprecision(2) << ray_median / magic_median
              << "x faster magic lookups; checksum " << magic_checksum << '\n';
}

configuration parse_arguments(int argc, char **argv) {
    configuration config;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        auto require_value = [&](std::string_view name) {
            if (++index >= argc) {
                std::cerr << "Missing value after " << name << '\n';
                std::exit(2);
            }
            return std::string(argv[index]);
        };
        if (argument == "--depth")
            config.depth = std::stoi(require_value(argument));
        else if (argument == "--runs")
            config.runs = std::stoi(require_value(argument));
        else if (argument == "--magic-iterations")
            config.magic_iterations = std::stoull(require_value(argument));
        else {
            std::cerr << "Unknown argument: " << argument << '\n';
            std::exit(2);
        }
    }
    if (config.depth < 1 || config.runs < 1 || config.magic_iterations < 1) {
        std::cerr << "All benchmark parameters must be positive\n";
        std::exit(2);
    }
    return config;
}

} // namespace

int main(int argc, char **argv) {
    const configuration config = parse_arguments(argc, argv);
    std::cout << "=== Chess engine benchmark ===\n"
              << "Compiler: " << __VERSION__ << " | TT: "
              << (sizeof(tt_bucket) * TT_BUCKET_COUNT / (1024 * 1024)) << " MiB | move: " << sizeof(move)
              << " bytes | board: " << sizeof(board) << " bytes\n"
              << "Runs: " << config.runs << " (median reported)\n";
    benchmark_perft(config.runs);
    benchmark_search(config);
    benchmark_magic(config);
    return 0;
}
