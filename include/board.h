#pragma once
#include "globals.h"
#include "move.h"

class board {
  public:
    u64 white_pawns, black_pawns;
    u64 white_king, black_king;
    u64 white_knights, black_knights;
    u64 white_rooks, black_rooks;
    u64 white_bishops, black_bishops;
    u64 white_queens, black_queens;
    // // wp, wr, wn, wb, wk, wq, bp, br, bn, bb, bk, bq
    // u64 pieces[12];
    u32 flags;

    board();
    board(const board &b) = default;
    board(board &&b) noexcept = default;
    board &operator=(const board &b) = default;
    u64 all_white_pieces() const;
    u64 all_black_pieces() const;
    u64 all_pieces() const;
    bool set_piece(u64 location, PIECE piece, COLOUR colour);
    pair<COLOUR, PIECE> get_piece(u64 location) const;
    void apply_move(const move &move_to_make);
    void print_board() const;
};