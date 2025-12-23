#pragma once
#include "globals.h"

class move {
  public:
    PIECE piece = PIECE::EMPTY;
    COLOUR colour = COLOUR::NONE;
    u64 start_location = 0ULL;
    u64 end_location = 0ULL;
    PIECE promotion_piece = PIECE::EMPTY;
    bool piece_captured = false;
    bool enpass = false;
    bool castle_kside = false;
    bool castle_qside = false;

    move() = default;
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end)
        : piece(p), colour(c), start_location(start), end_location(end) {}
    move(const PIECE& p, const COLOUR &c, const u64 &start, const u64 &end, const PIECE &prom_piece)
        : piece(p), colour(c), start_location(start), end_location(end), promotion_piece(prom_piece) {}
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap)
        : piece(p), colour(c), start_location(start), end_location(end), piece_captured(p_cap) {}
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const bool &ep)
        : piece(p), colour(c), start_location(start), end_location(end), piece_captured(p_cap), enpass(ep) {}
    move(const PIECE &p, const COLOUR &c, const bool &castle_k, const bool &castle_q)
        : piece(p), colour(c), castle_kside(castle_k), castle_qside(castle_q) {}
    bool operator==(const move &other) const;
    void print_move() const;
};
