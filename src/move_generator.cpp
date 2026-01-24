#include "../include/board.h"
#include "../include/globals.h"
#include "../include/magic.h"

/*
get_X_moves :- list of legal move masks (per piece), for X = bishop, rook, queen, king, knight
for (move in X_moves):-
    for(final pos in move):-
        if not in check
*/

u64 get_diagonal_moves(const u64 &blockers, const int &pos) {
    int idx = u64(blockers * bishop_magic_keys[pos]) >> bishop_shifts[pos];
    return bishop_table[pos][idx];
}

u64 get_hv_moves(const u64 &blockers, const int &pos) {
    int idx = u64(blockers * rook_magic_keys[pos]) >> rook_shifts[pos];
    return rook_table[pos][idx];
}

u64 get_king_moves(const COLOUR &turn, const board &b) {
    int r = lsb(b.pieces[get_piece_idx(turn, PIECE::KING)]) / 8;
    int c = lsb(b.pieces[get_piece_idx(turn, PIECE::KING)]) % 8;
    u64 mask = 0ULL;
    if (r + 1 < 7)
        mask |= set_rows[r + 1];
    if (r - 1 > 0)
        mask |= set_rows[r - 1];
    if (c + 1 < 7)
        mask &= set_cols[c + 1];
    if (c - 1 > 0)
        mask &= set_cols[c - 1];
    mask = (turn == COLOUR::WHITE) ? mask & (~b.all_black_pieces()) : mask & (~b.all_white_pieces());
    return mask;
}

std::vector<u64> get_knight_moves(const COLOUR &turn, const board &b) {
    u64 knights = b.pieces[get_piece_idx(turn, PIECE::KNIGHT)];
    std::vector<u64> result;
    while (knights) {
        u64 cur = knights ^ (knights & (knights - 1));
        u64 legal_moves = knight_moves[lsb(cur)];
        legal_moves =
            (turn == COLOUR::WHITE) ? legal_moves & (~b.all_black_pieces()) : legal_moves & (~b.all_white_pieces());
        result.push_back(legal_moves);
        knights &= knights - 1;
    }
    return result;
}

std::vector<u64> get_bishop_moves(const COLOUR &turn, const board &b) {
    u64 bishops = b.pieces[get_piece_idx(turn, PIECE::BISHOP)];
    std::vector<u64> result;
    while (bishops) {
        u64 cur = bishops ^ (bishops & (bishops - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & bishop_movement_masks_no_edges[pos]);
        u64 legal_moves = get_diagonal_moves(blockers, pos);
        legal_moves =
            (turn == COLOUR::WHITE) ? legal_moves & (~b.all_black_pieces()) : legal_moves & (~b.all_white_pieces());
        result.push_back(legal_moves);
        bishops &= bishops - 1;
    }
    return result;
}

std::vector<u64> get_rook_moves(const COLOUR &turn, const board &b) {
    u64 rooks = b.pieces[get_piece_idx(turn, PIECE::ROOK)];
    std::vector<u64> result;
    while (rooks) {
        u64 cur = rooks ^ (rooks & (rooks - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & rook_movement_masks_no_edges[pos]);
        u64 legal_moves = get_hv_moves(blockers, pos);
        legal_moves =
            (turn == COLOUR::WHITE) ? legal_moves & (~b.all_black_pieces()) : legal_moves & (~b.all_white_pieces());
        result.push_back(legal_moves);
        rooks &= rooks - 1;
    }

    return result;
}

std::vector<u64> get_queen_moves(const COLOUR &turn, const board &b) {
    u64 queens = b.pieces[get_piece_idx(turn, PIECE::QUEEN)];
    std::vector<u64> result;
    while (queens) {
        u64 cur = queens ^ (queens & (queens - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & bishop_movement_masks_no_edges[pos]);
        u64 legal_moves_diag = get_diagonal_moves(blockers, pos);
        blockers = (b.all_pieces() & rook_movement_masks_no_edges[pos]);
        u64 legal_moves_hv = get_hv_moves(blockers, pos);
        u64 legal_moves = (turn == COLOUR::WHITE) ? (legal_moves_diag | legal_moves_hv) & (~b.all_black_pieces())
                                                  : (legal_moves_diag | legal_moves_hv) & (~b.all_white_pieces());
        result.push_back(legal_moves);
        queens &= queens - 1;
    }

    return result;
}

std::vector<move> get_pawn_moves(const COLOUR &turn, const board &current) {
    if (turn == COLOUR::NONE)
        return {};

    std::vector<move> candidates;
    if (turn == COLOUR::WHITE) {
        u64 white_pawns = current.pieces[get_piece_idx(turn, PIECE::PAWN)];
        while (white_pawns) {
            u64 pawn = white_pawns ^ (white_pawns & (white_pawns - 1));
            int pawn_loc = lsb(pawn);

            u64 single_step = pawn << 8;

            // pawn promotion on single step push
            if (pawn_loc / 8 == 6) {
                if (!(single_step & current.all_pieces())) {
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step, PIECE::QUEEN));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step, PIECE::ROOK));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step, PIECE::BISHOP));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step, PIECE::KNIGHT));
                }
            }

            if (pawn_loc / 8 != 6 && !(single_step & current.all_pieces())) {
                candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step));

                // if at start square, can have double push
                if (pawn_loc / 8 == 1) {
                    u64 double_step = pawn << (2 * 8);
                    if (!(double_step & current.all_pieces()))
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, double_step));
                }
            }

            // left capture (possible if not on a-file)
            if (pawn_loc % 8 != 0) {
                u64 left_capture = (pawn << 8) >> 1;
                if (left_capture & current.all_black_pieces()) {
                    // pawn promotion + capture
                    if (pawn_loc / 8 == 6) {
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true, PIECE::QUEEN));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true, PIECE::ROOK));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true, PIECE::BISHOP));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true, PIECE::KNIGHT));
                    } else
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true));
                }
                if (left_capture == current.enpass_capture_square)
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true, true));
            }

            // right capture (possible if not on h-file)
            if (pawn_loc % 8 != 7) {
                u64 right_capture = (pawn << 8) << 1;
                if (right_capture & current.all_black_pieces()) {
                    // pawn promotion + capture
                    if (pawn_loc / 8 == 6) {
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true, PIECE::QUEEN));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true, PIECE::ROOK));
                        candidates.push_back(
                            move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true, PIECE::BISHOP));
                        candidates.push_back(
                            move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true, PIECE::KNIGHT));
                    } else
                        candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true));
                }
                if (right_capture == current.enpass_capture_square)
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true, true));
            }
            white_pawns &= white_pawns - 1;
        }
    } else {
        u64 black_pawns = current.pieces[get_piece_idx(turn, PIECE::PAWN)];
        while (black_pawns) {
            u64 pawn = black_pawns ^ (black_pawns & (black_pawns - 1));
            int pawn_loc = lsb(pawn);

            u64 single_step = pawn >> 8;

            // pawn promotion on single step push
            if (pawn_loc / 8 == 1) {
                if (!(single_step & current.all_pieces())) {
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step, PIECE::QUEEN));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step, PIECE::ROOK));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step, PIECE::BISHOP));
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step, PIECE::KNIGHT));
                }
            }

            if (pawn_loc / 8 != 1 && !(single_step & current.all_pieces())) {
                candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step));

                // if at start square, can have double push
                if (pawn_loc / 8 == 6) {
                    u64 double_step = pawn >> (2 * 8);
                    if (!(double_step & current.all_pieces()))
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, double_step));
                }
            }

            // left capture (possible if not on a-file)
            if (pawn_loc % 8 != 0) {
                u64 left_capture = (pawn >> 8) >> 1;
                if (left_capture & current.all_white_pieces()) {
                    // pawn promotion + capture
                    if (pawn_loc / 8 == 1) {
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true, PIECE::QUEEN));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true, PIECE::ROOK));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true, PIECE::BISHOP));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true, PIECE::KNIGHT));
                    } else
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true));
                }
                if (left_capture == current.enpass_capture_square)
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true, true));
            }

            // right capture (possible if not on h-file)
            if (pawn_loc % 8 != 7) {
                u64 right_capture = (pawn >> 8) << 1;
                if (right_capture & current.all_white_pieces()) {
                    // pawn promotion + capture
                    if (pawn_loc / 8 == 1) {
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true, PIECE::QUEEN));
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true, PIECE::ROOK));
                        candidates.push_back(
                            move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true, PIECE::BISHOP));
                        candidates.push_back(
                            move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true, PIECE::KNIGHT));
                    } else
                        candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true));
                }
                if (right_capture == current.enpass_capture_square)
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true, true));
            }
            black_pawns &= black_pawns - 1;
        }
    }

    return candidates;
}

std::vector<move> get_all_generic_moves(const COLOUR &turn, const board &b) {
    std::vector<move> all_moves;
    u64 enemy_pieces = (turn == COLOUR::WHITE) ? b.all_black_pieces() : b.all_white_pieces();
    auto bishop_moves = get_bishop_moves(turn, b);
    int i = 0;
    u64 cur_pieces = b.pieces[get_piece_idx(turn, PIECE::BISHOP)];
    while (cur_pieces) {
        u64 cur_start_pos = cur_pieces ^ (cur_pieces & (cur_pieces - 1));
        while (bishop_moves[i]) {
            u64 cur_end_pos = bishop_moves[i] ^ (bishop_moves[i] & (bishop_moves[i] - 1));
            if (cur_end_pos & enemy_pieces)
                all_moves.push_back(move(PIECE::BISHOP, turn, cur_start_pos, cur_end_pos, true));
            else
                all_moves.push_back(move(PIECE::BISHOP, turn, cur_start_pos, cur_end_pos));
            bishop_moves[i] &= bishop_moves[i] - 1;
        }
        i++;
        cur_pieces &= cur_pieces - 1;
    }

    auto rook_moves = get_rook_moves(turn, b);
    i = 0;
    cur_pieces = b.pieces[get_piece_idx(turn, PIECE::ROOK)];
    while (cur_pieces) {
        u64 cur_start_pos = cur_pieces ^ (cur_pieces & (cur_pieces - 1));
        while (rook_moves[i]) {
            u64 cur_end_pos = rook_moves[i] ^ (rook_moves[i] & (rook_moves[i] - 1));
            if (cur_end_pos & enemy_pieces)
                all_moves.push_back(move(PIECE::ROOK, turn, cur_start_pos, cur_end_pos, true));
            else
                all_moves.push_back(move(PIECE::ROOK, turn, cur_start_pos, cur_end_pos));
            rook_moves[i] &= rook_moves[i] - 1;
        }
        i++;
        cur_pieces &= cur_pieces - 1;
    }

    auto queen_moves = get_queen_moves(turn, b);
    i = 0;
    cur_pieces = b.pieces[get_piece_idx(turn, PIECE::QUEEN)];
    while (cur_pieces) {
        u64 cur_start_pos = cur_pieces ^ (cur_pieces & (cur_pieces - 1));
        while (queen_moves[i]) {
            u64 cur_end_pos = queen_moves[i] ^ (queen_moves[i] & (queen_moves[i] - 1));
            if (cur_end_pos & enemy_pieces)
                all_moves.push_back(move(PIECE::QUEEN, turn, cur_start_pos, cur_end_pos, true));
            else
                all_moves.push_back(move(PIECE::QUEEN, turn, cur_start_pos, cur_end_pos));
            queen_moves[i] &= queen_moves[i] - 1;
        }
        i++;
        cur_pieces &= cur_pieces - 1;
    }

    u64 king_moves = get_king_moves(turn, b);
    u64 cur_start_pos = b.pieces[get_piece_idx(turn, PIECE::KING)];
    while (king_moves) {
        u64 cur_end_pos = king_moves ^ (king_moves & (king_moves - 1));
        if (cur_end_pos & enemy_pieces)
            all_moves.push_back(move(PIECE::KING, turn, cur_start_pos, cur_end_pos, true));
        else
            all_moves.push_back(move(PIECE::KING, turn, cur_start_pos, cur_end_pos));
        king_moves &= king_moves - 1;
    }

    auto knight_moves = get_knight_moves(turn, b);
    i = 0;
    cur_pieces = b.pieces[get_piece_idx(turn, PIECE::KNIGHT)];
    while (cur_pieces) {
        u64 cur_start_pos = cur_pieces ^ (cur_pieces & (cur_pieces - 1));
        while (knight_moves[i]) {
            u64 cur_end_pos = knight_moves[i] ^ (knight_moves[i] & (knight_moves[i] - 1));
            if (cur_end_pos & enemy_pieces)
                all_moves.push_back(move(PIECE::KNIGHT, turn, cur_start_pos, cur_end_pos, true));
            else
                all_moves.push_back(move(PIECE::KNIGHT, turn, cur_start_pos, cur_end_pos));
            knight_moves[i] &= knight_moves[i] - 1;
        }
        i++;
        cur_pieces &= cur_pieces - 1;
    }

    auto pawn_moves = get_pawn_moves(turn, b);

    return all_moves;
}

bool is_attacked_by_bishop(const board &b, const COLOUR &attacker, const u64 &target_location) {
    if (!target_location)
        return false;

    u64 bishops = b.pieces[get_piece_idx(attacker, PIECE::BISHOP)];
    while (bishops) {
        u64 cur = bishops ^ (bishops & (bishops - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & bishop_movement_masks_no_edges[pos]);
        u64 legal_moves = get_diagonal_moves(blockers, pos);
        if (legal_moves & target_location)
            return true;
        bishops &= bishops - 1;
    }
    return false;
}

bool is_attacked_by_rook(const board &b, const COLOUR &attacker, const u64 &target_location) {
    if (!target_location)
        return false;

    u64 rooks = b.pieces[get_piece_idx(attacker, PIECE::ROOK)];
    while (rooks) {
        u64 cur = rooks ^ (rooks & (rooks - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & rook_movement_masks_no_edges[pos]);
        u64 legal_moves = get_hv_moves(blockers, pos);
        if (legal_moves & target_location)
            return true;
        rooks &= rooks - 1;
    }
    return false;
}

bool is_attacked_by_queen(const board &b, const COLOUR &attacker, const u64 &target_location) {
    if (!target_location)
        return false;

    u64 queens = b.pieces[get_piece_idx(attacker, PIECE::ROOK)];
    while (queens) {
        u64 cur = queens ^ (queens & (queens - 1));
        int pos = lsb(cur);
        u64 blockers = (b.all_pieces() & rook_movement_masks_no_edges[pos]);
        u64 legal_moves = get_hv_moves(blockers, pos);
        if (legal_moves & target_location)
            return true;

        blockers = (b.all_pieces() & bishop_movement_masks_no_edges[pos]);
        legal_moves = get_diagonal_moves(blockers, pos);
        if (legal_moves & target_location)
            return true;

        queens &= queens - 1;
    }
    return false;
}

bool is_attacked_by_knight(const board &b, const COLOUR &attacker, const u64 &target_location) {
    u64 knights = b.pieces[get_piece_idx(attacker, PIECE::KNIGHT)];
    while (knights) {
        u64 cur = knights ^ (knights & (knights - 1));
        u64 legal_moves = knight_moves[lsb(cur)];
        if (legal_moves & target_location)
            return true;
        knights &= knights - 1;
    }
    return false;
}

bool is_attacked_by_king(const board &b, const COLOUR &attacker, const u64 &target_location) {
    if (!target_location)
        return false;
    u64 attacker_mask = get_king_moves(attacker, b);
    if (target_location & attacker_mask)
        return true;

    return false;
}

bool is_attacked_by_pawn(const board &b, const COLOUR &attacker, const u64 &target_location) {
    if (!target_location)
        return false;

    u64 pawns = b.pieces[get_piece_idx(attacker, PIECE::PAWN)];
    while (pawns) {
        u64 attacker_location = pawns ^ (pawns & (pawns - 1));
        if (attacker == COLOUR::WHITE) {
            int pawn_loc = lsb(attacker_location);
            if (pawn_loc == 7)
                continue;
            if (pawn_loc % 8 != 0 && target_location == ((attacker_location >> 8) >> 1))
                return true;
            if (pawn_loc % 8 != 7 && target_location == ((attacker_location >> 8) << 1))
                return true;
        } else {
            int pawn_loc = lsb(attacker_location);
            if (pawn_loc == 0)
                continue;
            if (pawn_loc % 8 != 0 && target_location == ((attacker_location << 8) >> 1))
                return true;
            if (pawn_loc % 8 != 7 && target_location == ((attacker_location << 8) << 1))
                return true;
        }
        pawns &= pawns - 1;
    }

    return false;
}

bool is_sqr_attacked(const board &b, const COLOUR &attacker, const u64 &target_location) {
    return is_attacked_by_bishop(b, attacker, target_location) | is_attacked_by_rook(b, attacker, target_location) |
           is_attacked_by_queen(b, attacker, target_location) | is_attacked_by_knight(b, attacker, target_location) |
           is_attacked_by_pawn(b, attacker, target_location) | is_attacked_by_king(b, attacker, target_location);
}

bool is_in_check(const board &b, const COLOUR &turn) {
    COLOUR attacker = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
    return is_sqr_attacked(b, attacker, b.pieces[get_piece_idx(turn, PIECE::KING)]);
}

bool is_in_check_after(board &b, const COLOUR &turn, const move &mv) {
    board temp = b;
    b.apply_move(mv);
    if (is_in_check(b, turn)) {
        b = temp;
        return true;
    } else{
        b = temp;
        return false;
    }
}