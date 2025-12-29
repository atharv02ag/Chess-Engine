#pragma once
#include "globals.h"

class move {
  public:
    PIECE piece = PIECE::EMPTY;
    COLOUR colour = COLOUR::NONE;
    u64 start_location = 0ULL;
    u64 end_location = 0ULL;
    u32 move_flags = 0;

    move() = default;
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const PIECE &prom_piece);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const PIECE &prom_piece);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const bool &ep);
    move(const PIECE &p, const COLOUR &c, const bool &castle_k, const bool &castle_q);
    bool piece_captured() const;
    bool enpass() const;
    bool castle_kside() const;
    bool castle_qside() const;
    PIECE promotion_piece() const;
    bool operator==(const move &other) const;
    void print_move() const;
};
