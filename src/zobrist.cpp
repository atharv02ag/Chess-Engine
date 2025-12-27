#include "../include/zobrist.h"
#include "../include/prng.h"

std::array<std::array<u64, 64>, 12> zobrist::rv_pieces{};
std::array<u64, 4> zobrist::rv_flags{};
std::array<u64, 16> zobrist::rv_enpass{};
u64 zobrist::rv_colour{};

void zobrist::init() {
    prng g;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 12; j++) {
            zobrist::rv_pieces[j][i] = g.gen_num(1, UINT64_MAX);
        }
    }
    for (int i = 0; i < 4; i++) {
        zobrist::rv_flags[i] = g.gen_num(1, UINT64_MAX);
    }
    for (int i = 0; i < 16; i++) {
        zobrist::rv_enpass[i] = g.gen_num(1, UINT64_MAX);
    }
    zobrist::rv_colour = g.gen_num(1, UINT64_MAX);
}