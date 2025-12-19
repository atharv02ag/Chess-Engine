#pragma once
#include "board.h"
#include "move.h"

class move_generator {
  public:
    move_generator(const board &cur_state, const COLOUR &turn_to_play) : current(cur_state), turn(turn_to_play) {}
    move_generator(const move_generator &mg) : current(mg.current), turn(mg.turn) {}
    move_generator(move_generator &&mg) noexcept : current(std::move(mg.current)), turn(std::move(mg.turn)) {}

    vector<move> get_bishop_moves();
    vector<move> get_rook_moves();
    vector<move> get_queen_moves();
    vector<move> get_knight_moves();
    vector<move> get_king_moves();
    vector<move> get_pawn_moves();
    vector<move> get_all_generic_moves();
    bool is_sqr_attacked(const u64 &target_location);
    bool is_in_check();
    bool is_in_check_after(const move &mv);
    void update(const board &new_board);
    void update(const COLOUR &new_turn);
    void update(const board &new_board, const COLOUR &new_turn);

  private:
    board current;
    COLOUR turn;
    void filter_diagonal_moves(const PIECE &piece, const u64 &piece_location, vector<move> &candidates);
    void filter_hv_moves(const PIECE &piece, const u64 &piece_location, vector<move> &candidates);
    bool is_attacked_diagonally(const u64 &target_location, const u64 &attacker_location);
    bool is_attacked_hv(const u64 &target_location, const u64 &attacker_location);
    bool is_attacked_by_pawn(const u64 &target_location, const u64 &attacker_location);

    template <auto &offsets>
    bool is_attacked_by_offset_piece(const u64 &target_location, const u64 &attacker_location) {
        int target_r = lsb(target_location) / 8;
        int target_c = lsb(target_location) % 8;
        int attacker_r = lsb(attacker_location) / 8;
        int attacker_c = lsb(attacker_location) % 8;
        for (const auto &[d_r, d_c] : offsets) {
            if (attacker_r + d_r < 0 || attacker_r + d_r > 7 || attacker_c + d_c < 0 || attacker_c + d_c > 7)
                continue;
            if (attacker_r + d_r == target_r && attacker_c + d_c == target_c)
                return true;
        }
        return false;
    }

    template <auto &offsets>
    void filter_offset_moves(const PIECE &piece, const u64 &piece_location, vector<move> &candidates) {
        int r = lsb(piece_location) / 8;
        int c = lsb(piece_location) % 8;
        for (const auto &[d_r, d_c] : offsets) {
            if (r + d_r < 0 || r + d_r > 7 || c + d_c < 0 || c + d_c > 7)
                continue;
            u64 landing_location = (1ULL << (8 * (r + d_r))) << (c + d_c);
            if ((turn == COLOUR::BLACK && (current.all_black_pieces() & landing_location)) ||
                (turn == COLOUR::WHITE && (current.all_white_pieces() & landing_location)))
                continue;
            if ((turn == COLOUR::WHITE && (current.all_black_pieces() & landing_location)) ||
                (turn == COLOUR::BLACK && (current.all_white_pieces() & landing_location))) {
                candidates.push_back(move(piece, turn, piece_location, landing_location, true));
            } else
                candidates.push_back(move(piece, turn, piece_location, landing_location));
        }
    }
};
