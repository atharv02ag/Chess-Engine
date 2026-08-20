#pragma once
#include "globals.h"
#include "move.h"

inline constexpr std::size_t TT_BUCKET_SIZE = 4;
inline constexpr std::size_t TT_BUCKET_COUNT = 1U << 20;

struct tt_entry {
    // The bucket index already verifies 20 low hash bits. Keeping the upper
    // 32 bits here gives a 52-bit effective key while retaining a 16-byte entry.
    u32 key = 0U;
    float eval = 0.0f;
    move best_move{};
    std::int8_t depth = -1;
    TT_FLAG node_type = TT_FLAG::EXACT;
    std::uint16_t generation = 0;
};

struct tt_bucket {
    std::array<tt_entry, TT_BUCKET_SIZE> entries{};
};

class tt {
  public:
    std::uint16_t tt_generation = 1;
    std::vector<tt_bucket> container;

    tt();
    const tt_entry *probe(u64 hash) const;
    void insert(u64 hash, int depth, float eval, const move &best_move, TT_FLAG node_type);
    void clear();
};
