#pragma once
#include "globals.h"
#include "move.h"

class tt_entry {
  public:
    u64 zkey = 0ULL;
    int depth = 0;
    float eval = 0.0f;
    move best_move = move();
    int generation = 0;
    TT_FLAG node_type = TT_FLAG::EXACT;
    tt_entry() = default;
    tt_entry(const u64& zk, const int& d, const float& ev, const move& mv, const int& gen, const TT_FLAG& type);
};

class tt {
    public:
        int tt_generation = 0;
        std::vector<tt_entry> container;

        tt();
        tt_entry probe(u64 hash);
        void insert(tt_entry node);
        void clear();
};