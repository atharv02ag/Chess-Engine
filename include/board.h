#pragma once
#include "globals.h"
#include "move.h"

class board {
  public:
    u64 pieces[12];            // wp, wr, wn, wb, wk, wq, bp, br, bn, bb, bk, bq
    u32 flags;                 // castling flags
    u64 enpass_capture_square; // if enpass is possible, stores end position of the pawn after capture
    COLOUR turn;
    u64 zhash;                 // zobrist hash of the position

    board();
    board(const board &b) = default;
    board(board &&b) noexcept = default;
    board &operator=(const board &b) = default;
    void init_hash();
    void load_fen(const std::string &fen);
    u64 all_white_pieces() const;
    u64 all_black_pieces() const;
    u64 all_pieces() const;
    bool set_piece(const u64 &location, const PIECE &piece, const COLOUR &colour);
    std::pair<COLOUR, PIECE> get_piece(const u64 &location) const;
    void apply_move(const move &move_to_make);
    void print_board() const;
};