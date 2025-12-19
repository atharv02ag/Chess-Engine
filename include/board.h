#pragma once
#include "globals.h"
#include "move.h"

class board {
  public:
    // wp, wr, wn, wb, wk, wq, bp, br, bn, bb, bk, bq
    u64 pieces[12];
    u32 flags;
    COLOUR turn;

    board();
    board(const board &b) = default;
    board(board &&b) noexcept = default;
    board &operator=(const board &b) = default;
    void load_fen(const std::string &fen);
    u64 all_white_pieces() const;
    u64 all_black_pieces() const;
    u64 all_pieces() const;
    bool set_piece(u64 location, PIECE piece, COLOUR colour);
    std::pair<COLOUR, PIECE> get_piece(u64 location) const;
    void apply_move(const move &move_to_make);
    void print_board() const;
};