#pragma once
#include "board.h"
#include "move.h"

class move_generator {
  public:
    move_generator(const board &cur_state, const COLOUR &turn_to_play) : b(cur_state), turn(turn_to_play) {}
    move_generator(const move_generator &mg) : b(mg.b), turn(mg.turn) {}
    move_generator(move_generator &&mg) noexcept : b(std::move(mg.b)), turn(std::move(mg.turn)) {}

    // return pseudo-legal move masks per piece
    std::vector<u64> get_bishop_moves();
    std::vector<u64> get_rook_moves();
    std::vector<u64> get_queen_moves();
    std::vector<u64> get_knight_moves();
    u64 get_king_moves();

    std::vector<move> get_pawn_moves();
    std::vector<move> get_all_generic_moves();
    bool is_sqr_attacked(const u64 &target_location);
    bool is_in_check();
    bool is_in_check_after(const move &mv);
    void update(const board &new_board);
    void update(const COLOUR &new_turn);
    void update(const board &new_board, const COLOUR &new_turn);

  private:
    board b;
    COLOUR turn;
    u64 get_diagonal_mask(const u64 &blockers, const int &pos);
    u64 get_hv_mask(const u64 &blockers, const int &pos);
    u64 get_king_mask(const int& pos);
    bool is_attacked_by_bishop(const u64 &target_location);
    bool is_attacked_by_rook(const u64 &target_location);
    bool is_attacked_by_pawn(const u64 &target_location);
    bool is_attacked_by_queen(const u64 &target_location);
    bool is_attacked_by_knight(const u64 &target_location);
    bool is_attacked_by_king(const u64 &target_location);
};