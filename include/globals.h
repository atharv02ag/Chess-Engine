#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using u64 = uint64_t;
using u32 = uint32_t;
using std::cout;
using std::endl;
using std::cin;

inline constexpr float INF = 1000000.0f;

enum class COLOUR { WHITE, BLACK, NONE };

enum class PIECE { EMPTY, PAWN, ROOK, KNIGHT, BISHOP, KING, QUEEN };

enum class FLAGS : u32 {
    // castling rights, set to 0 if king or any rooks have moved, or already castled. Default 1.
    CASTLE_KSIDE_WHITE = 1 << 0,
    CASTLE_QSIDE_WHITE = 1 << 1,
    CASTLE_KSIDE_BLACK = 1 << 2,
    CASTLE_QSIDE_BLACK = 1 << 3,

};

inline int lsb(const u64 &num) { return __builtin_ctzll(num); }
inline int msb(const u64 &num) { return 63 - __builtin_clzll(num); }
inline int pop_ct(const u64 &num) { return __builtin_popcountll(num); }
inline constexpr std::array<std::string_view, 3> colour_names = {"WHITE", "BLACK", "NONE"};
inline constexpr std::array<std::string_view, 7> piece_names = {"EMPTY",  "PAWN", "ROOK", "KNIGHT",
                                                                "BISHOP", "KING", "QUEEN"};
inline constexpr std::array<std::pair<int, int>, 8> knight_offsets = {
    std::pair{1, 2}, std::pair{1, -2}, std::pair{-1, 2}, std::pair{-1, -2},
    std::pair{2, 1}, std::pair{2, -1}, std::pair{-2, 1}, std::pair{-2, -1}};
inline constexpr std::array<std::pair<int, int>, 8> king_offsets = {
    std::pair{1, 1}, std::pair{1, -1}, std::pair{-1, 1}, std::pair{-1, -1},
    std::pair{0, 1}, std::pair{0, -1}, std::pair{1, 0},  std::pair{-1, 0}};

// make sure colour != NONE and piece != EMPTY
inline int get_piece_idx(const COLOUR &colour, const PIECE &piece) {
    return static_cast<int>(colour) * 6 + (static_cast<int>(piece) - 1);
}