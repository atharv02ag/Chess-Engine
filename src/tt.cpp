#include "../include/tt.h"

#include <limits>

namespace {

constexpr std::size_t bucket_index(u64 hash) {
    return static_cast<std::size_t>(hash) & (TT_BUCKET_COUNT - 1);
}

constexpr u32 key_signature(u64 hash) { return static_cast<u32>(hash >> 32); }

std::int8_t stored_depth(int depth) {
    constexpr int minimum = std::numeric_limits<std::int8_t>::min();
    constexpr int maximum = std::numeric_limits<std::int8_t>::max();
    return static_cast<std::int8_t>(std::clamp(depth, minimum, maximum));
}

} // namespace

tt::tt() : container(TT_BUCKET_COUNT) {}

const tt_entry *tt::probe(u64 hash) const {
    const u32 key = key_signature(hash);
    const tt_bucket &bucket = container[bucket_index(hash)];
    for (const tt_entry &entry : bucket.entries) {
        if (entry.generation == tt_generation && entry.key == key)
            return &entry;
    }
    return nullptr;
}

void tt::insert(u64 hash, int depth, float eval, const move &best_move, TT_FLAG node_type) {
    const u32 key = key_signature(hash);
    const std::int8_t compact_depth = stored_depth(depth);
    tt_bucket &bucket = container[bucket_index(hash)];
    tt_entry *replacement = nullptr;

    for (tt_entry &entry : bucket.entries) {
        if (entry.generation == tt_generation && entry.key == key) {
            if (compact_depth > entry.depth ||
                (compact_depth == entry.depth &&
                 (node_type == TT_FLAG::EXACT || entry.node_type != TT_FLAG::EXACT))) {
                entry = {key, eval, best_move, compact_depth, node_type, tt_generation};
            }
            return;
        }

        if (entry.generation != tt_generation) {
            replacement = &entry;
            break;
        }

        if (replacement == nullptr || entry.depth < replacement->depth)
            replacement = &entry;
    }

    if (replacement != nullptr &&
        (replacement->generation != tt_generation || compact_depth >= replacement->depth)) {
        *replacement = {key, eval, best_move, compact_depth, node_type, tt_generation};
    }
}

void tt::clear() {
    if (tt_generation == std::numeric_limits<u16>::max()) {
        std::fill(container.begin(), container.end(), tt_bucket{});
        tt_generation = 1;
        return;
    }
    ++tt_generation;
}
