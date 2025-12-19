#include "../include/move.h"
#include "../include/move_generator.h"

void move::print_move() const {
    if (castle_kside) {
        cout << "COLOUR : " << colour_names[static_cast<int>(colour)] << " CASTLED KSIDE" << endl;
        return;
    }
    if (castle_qside) {
        cout << "COLOUR: " << colour_names[static_cast<int>(colour)] << " CASTLED QSIDE" << endl;
        return;
    }
    int start_row = lsb(start_location) / 8;
    int start_col = lsb(start_location) % 8;
    int end_row = lsb(end_location) / 8;
    int end_col = lsb(end_location) % 8;
    if (piece_captured)
        cout << "CAPTURE, ";
    cout << "COLOUR : " << colour_names[static_cast<int>(colour)] << " PIECE : " << piece_names[static_cast<int>(piece)]
         << " START : (" << start_row << "," << start_col << ") END : (" << end_row << "," << end_col << ")" << endl;
}

bool move::operator==(const move &other) const {
    return (other.piece == piece && other.colour == colour && other.start_location == start_location &&
            other.end_location == end_location && other.piece_captured == piece_captured &&
            other.castle_kside == castle_kside && other.castle_qside == castle_qside);
}

void move_generator::update(const board &new_board) { current = new_board; }

void move_generator::update(const COLOUR &new_turn) { turn = new_turn; }

void move_generator::update(const board &new_board, const COLOUR &new_turn) {
    current = new_board;
    turn = new_turn;
}

void move_generator::filter_diagonal_moves(const PIECE &piece, const u64 &piece_location, std::vector<move> &candidates) {
    int r = lsb(piece_location) / 8;
    int c = lsb(piece_location) % 8;
    if (turn == COLOUR::WHITE) {
        for (int i = 1; i < 8; i++) {
            if (r + i > 7 || c + i > 7)
                break;
            u64 temp_location = (piece_location << (8 * i)) << i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0 || c - i < 0)
                break;
            u64 temp_location = (piece_location >> (8 * i)) >> i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r + i > 7 || c - i < 0)
                break;
            u64 temp_location = (piece_location << (8 * i)) >> i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0 || c + i > 7)
                break;
            u64 temp_location = (piece_location >> (8 * i)) << i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        return;
    }
    if (turn == COLOUR::BLACK) {
        for (int i = 1; i < 8; i++) {
            if (r + i > 7 || c + i > 7)
                break;
            u64 temp_location = (piece_location << (8 * i)) << i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0 || c - i < 0)
                break;
            u64 temp_location = (piece_location >> (8 * i)) >> i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r + i > 7 || c - i < 0)
                break;
            u64 temp_location = (piece_location << (8 * i)) >> i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0 || c + i > 7)
                break;
            u64 temp_location = (piece_location >> (8 * i)) << i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        return;
    }
}

void move_generator::filter_hv_moves(const PIECE &piece, const u64 &piece_location, std::vector<move> &candidates) {
    int r = lsb(piece_location) / 8;
    int c = lsb(piece_location) % 8;
    if (turn == COLOUR::WHITE) {
        for (int i = 1; i < 8; i++) {
            if (c + i > 7)
                break;
            u64 temp_location = piece_location << i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (c - i < 0)
                break;
            u64 temp_location = piece_location >> i;
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r + i > 7)
                break;
            u64 temp_location = piece_location << (8 * i);
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0)
                break;
            u64 temp_location = piece_location >> (8 * i);
            if (temp_location & current.all_white_pieces())
                break;
            if (temp_location & current.all_black_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        return;
    }
    if (turn == COLOUR::BLACK) {
        for (int i = 1; i < 8; i++) {
            if (c + i > 7)
                break;
            u64 temp_location = piece_location << i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (c - i < 0)
                break;
            u64 temp_location = piece_location >> i;
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r + i > 7)
                break;
            u64 temp_location = piece_location << (8 * i);
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
        for (int i = 1; i < 8; i++) {
            if (r - i < 0)
                break;
            u64 temp_location = piece_location >> (8 * i);
            if (temp_location & current.all_black_pieces())
                break;
            if (temp_location & current.all_white_pieces()) {
                candidates.push_back(move(piece, turn, piece_location, temp_location, true));
                break;
            }
            candidates.push_back(move(piece, turn, piece_location, temp_location));
        }
    }
}

std::vector<move> move_generator::get_bishop_moves() {
    if (turn == COLOUR::NONE)
        return {};
    std::vector<move> candidates;
    u64 bishops = current.pieces[get_piece_idx(turn, PIECE::BISHOP)];
    while (bishops) {
        u64 bishop_location = bishops ^ (bishops & (bishops - 1));
        filter_diagonal_moves(PIECE::BISHOP, bishop_location, candidates);
        bishops &= bishops - 1;
    }
    return candidates;
}
std::vector<move> move_generator::get_rook_moves() {
    if (turn == COLOUR::NONE)
        return {};
    std::vector<move> candidates;
    u64 rooks = current.pieces[get_piece_idx(turn, PIECE::ROOK)];
    while (rooks) {
        u64 rook_location = rooks ^ (rooks & (rooks - 1));
        filter_hv_moves(PIECE::ROOK, rook_location, candidates);
        rooks &= rooks - 1;
    }
    return candidates;
}

std::vector<move> move_generator::get_queen_moves() {
    if (turn == COLOUR::NONE)
        return {};

    std::vector<move> candidates;
    u64 queens = current.pieces[get_piece_idx(turn, PIECE::QUEEN)];
    while (queens) {
        u64 queen_location = queens ^ (queens & (queens - 1));
        filter_diagonal_moves(PIECE::QUEEN, queen_location, candidates);
        filter_hv_moves(PIECE::QUEEN, queen_location, candidates);
        queens &= queens - 1;
    }
    return candidates;
}

std::vector<move> move_generator::get_knight_moves() {
    if (turn == COLOUR::NONE)
        return {};

    std::vector<move> candidates;
    u64 knights = current.pieces[get_piece_idx(turn, PIECE::KNIGHT)];
    while (knights) {
        u64 knight_location = knights ^ (knights & (knights - 1));
        filter_offset_moves<knight_offsets>(PIECE::KNIGHT, knight_location, candidates);
        knights &= knights - 1;
    }
    return candidates;
}

std::vector<move> move_generator::get_king_moves() {
    if (turn == COLOUR::NONE)
        return {};

    std::vector<move> candidates;
    u64 king_location = current.pieces[get_piece_idx(turn, PIECE::KING)];
    filter_offset_moves<king_offsets>(PIECE::KING, king_location, candidates);

    return candidates;
}

std::vector<move> move_generator::get_pawn_moves() {
    if (turn == COLOUR::NONE)
        return {};

    std::vector<move> candidates;
    if (turn == COLOUR::WHITE) {
        u64 white_pawns = current.pieces[get_piece_idx(turn, PIECE::PAWN)];
        while (white_pawns) {
            u64 pawn = white_pawns ^ (white_pawns & (white_pawns - 1));
            int pawn_loc = lsb(pawn);

            if (pawn_loc / 8 == 7) {
                white_pawns &= white_pawns - 1;
                continue;
            }

            u64 single_step = pawn << 8;
            if (!(single_step & current.all_pieces()))
                candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, single_step));

            // if at start square, can have double push
            if (pawn_loc / 8 == 1) {
                u64 double_step = pawn << (2 * 8);
                if (!(single_step & current.all_pieces()) && !(double_step & current.all_pieces()))
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, double_step));
            }

            // left capture (possible if not on a-file)
            if (pawn_loc % 8 != 0) {
                u64 left_capture = (pawn << 8) >> 1;
                if ((left_capture & current.all_black_pieces()))
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, left_capture, true));
            }

            // right capture (possible if not on h-file)
            if (pawn_loc % 8 != 7) {
                u64 right_capture = (pawn << 8) << 1;
                if (right_capture & current.all_black_pieces())
                    candidates.push_back(move(PIECE::PAWN, COLOUR::WHITE, pawn, right_capture, true));
            }
            white_pawns &= white_pawns - 1;
        }
    } else {
        u64 black_pawns = current.pieces[get_piece_idx(turn, PIECE::PAWN)];
        while (black_pawns) {
            u64 pawn = black_pawns ^ (black_pawns & (black_pawns - 1));
            int pawn_loc = lsb(pawn);

            if (pawn_loc / 8 == 0) {
                black_pawns &= black_pawns - 1;
                continue;
            }

            u64 single_step = pawn >> 8;
            if (!(single_step & current.all_pieces()))
                candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, single_step));

            // if at start square, can have double push
            if (pawn_loc / 8 == 6) {
                u64 double_step = pawn >> (2 * 8);
                if (!(single_step & current.all_pieces()) && !(double_step & current.all_pieces()))
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, double_step));
            }

            // left capture (possible if not on a-file)
            if (pawn_loc % 8 != 0) {
                u64 left_capture = (pawn >> 8) >> 1;
                if ((left_capture & current.all_white_pieces()))
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, left_capture, true));
            }

            // right capture (possible if not on h-file)
            if (pawn_loc % 8 != 7) {
                u64 right_capture = (pawn >> 8) << 1;
                if (right_capture & current.all_white_pieces())
                    candidates.push_back(move(PIECE::PAWN, COLOUR::BLACK, pawn, right_capture, true));
            }
            black_pawns &= black_pawns - 1;
        }
    }

    return candidates;
}

bool move_generator::is_attacked_diagonally(const u64 &target_location, const u64 &attacker_location) {
    if (!target_location || !attacker_location)
        return false;

    int target_r = lsb(target_location) / 8;
    int target_c = lsb(target_location) % 8;
    int attacker_r = lsb(attacker_location) / 8;
    int attacker_c = lsb(attacker_location) % 8;

    if (target_r + target_c != attacker_r + attacker_c && target_r - target_c != attacker_r - attacker_c)
        return false;
    u64 full_board = current.all_pieces();
    if (target_r + target_c == attacker_r + attacker_c && target_r < attacker_r) {
        for (int i = 1; i < attacker_r - target_r; i++) {
            u64 temp_location = (1ULL << (8 * (target_r + i))) << (target_c - i);
            if (temp_location & full_board)
                return false;
        }
    } else if (target_r + target_c == attacker_r + attacker_c && target_r > attacker_r) {
        for (int i = 1; i < target_r - attacker_r; i++) {
            u64 temp_location = (1ULL << (8 * (target_r - i))) << (target_c + i);
            if (temp_location & full_board)
                return false;
        }
    } else if (target_r - target_c == attacker_r - attacker_c && target_r < attacker_r) {
        for (int i = 1; i < attacker_r - target_r; i++) {
            u64 temp_location = (1ULL << (8 * (target_r + i))) << (target_c + i);
            if (temp_location & full_board)
                return false;
        }
    } else if (target_r - target_c == attacker_r - attacker_c && target_r > attacker_r) {
        for (int i = 1; i < target_r - attacker_r; i++) {
            u64 temp_location = (1ULL << (8 * (target_r - i))) << (target_c - i);
            if (temp_location & full_board)
                return false;
        }
    }
    return true;
}

bool move_generator::is_attacked_hv(const u64 &target_location, const u64 &attacker_location) {
    if (!target_location || !attacker_location)
        return false;

    int target_r = lsb(target_location) / 8;
    int target_c = lsb(target_location) % 8;
    int attacker_r = lsb(attacker_location) / 8;
    int attacker_c = lsb(attacker_location) % 8;

    if (target_r != attacker_r && target_c != attacker_c)
        return false;
    u64 full_board = current.all_pieces();
    if (target_c == attacker_c && target_r < attacker_r) {
        for (int i = target_r + 1; i < attacker_r; i++) {
            u64 temp_location = (1ULL << (8 * i)) << target_c;
            if (temp_location & full_board)
                return false;
        }
    } else if (target_c == attacker_c && target_r > attacker_r) {
        for (int i = target_r - 1; i > attacker_r; i--) {
            u64 temp_location = (1ULL << (8 * i)) << target_c;
            if (temp_location & full_board)
                return false;
        }
    } else if (target_r == attacker_r && target_c < attacker_c) {
        for (int i = target_c + 1; i < attacker_c; i++) {
            u64 temp_location = (1ULL << (8 * target_r)) << i;
            if (temp_location & full_board)
                return false;
        }
    } else if (target_r == attacker_r && target_c > attacker_c) {
        for (int i = target_c - 1; i > attacker_c; i--) {
            u64 temp_location = (1ULL << (8 * target_r)) << i;
            if (temp_location & full_board)
                return false;
        }
    }
    return true;
}

bool move_generator::is_attacked_by_pawn(const u64 &target_location, const u64 &attacker_location) {
    if (turn == COLOUR::NONE || !target_location || !attacker_location)
        return false;
    if (turn == COLOUR::WHITE) {
        int pawn_loc = lsb(attacker_location);
        if (pawn_loc == 7)
            return false;
        if (pawn_loc % 8 != 0 && target_location == ((attacker_location << 8) >> 1))
            return true;
        if (pawn_loc % 8 != 7 && target_location == ((attacker_location << 8) << 1))
            return true;
        return false;
    } else {
        int pawn_loc = lsb(attacker_location);
        if (pawn_loc == 0)
            return false;
        if (pawn_loc % 8 != 0 && target_location == ((attacker_location >> 8) >> 1))
            return true;
        if (pawn_loc % 8 != 7 && target_location == ((attacker_location >> 8) << 1))
            return true;
        return false;
    }
}

bool move_generator::is_sqr_attacked(const u64 &target_location) {
    if (turn == COLOUR::NONE || !target_location)
        return false;

    COLOUR enemy_colour = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
    u64 cur_enemy =
        current.pieces[get_piece_idx(enemy_colour, PIECE::BISHOP)] | current.pieces[get_piece_idx(enemy_colour, PIECE::QUEEN)];
    while (cur_enemy) {
        u64 bishop = cur_enemy ^ (cur_enemy & (cur_enemy - 1));
        if (is_attacked_diagonally(target_location, bishop))
            return true;
        cur_enemy &= cur_enemy - 1;
    }

    cur_enemy = current.pieces[get_piece_idx(enemy_colour, PIECE::ROOK)] | current.pieces[get_piece_idx(enemy_colour, PIECE::QUEEN)];
    while (cur_enemy) {
        u64 rook = cur_enemy ^ (cur_enemy & (cur_enemy - 1));
        if (is_attacked_hv(target_location, rook))
            return true;
        cur_enemy &= cur_enemy - 1;
    }

    cur_enemy = current.pieces[get_piece_idx(enemy_colour, PIECE::KNIGHT)];
    while (cur_enemy) {
        u64 knight = cur_enemy ^ (cur_enemy & (cur_enemy - 1));
        if (is_attacked_by_offset_piece<knight_offsets>(target_location, knight))
            return true;
        cur_enemy &= cur_enemy - 1;
    }

    cur_enemy = current.pieces[get_piece_idx(enemy_colour, PIECE::KING)];
    if (is_attacked_by_offset_piece<king_offsets>(target_location, cur_enemy))
        return true;

    cur_enemy = current.pieces[get_piece_idx(enemy_colour, PIECE::PAWN)];
    while (cur_enemy) {
        u64 pawn = cur_enemy ^ (cur_enemy & (cur_enemy - 1));
        if (is_attacked_by_pawn(target_location, pawn))
            return true;
        cur_enemy &= cur_enemy - 1;
    }
    
    return false;
}

bool move_generator::is_in_check() {
    if (turn == COLOUR::NONE)
        return false;

    return is_sqr_attacked(current.pieces[get_piece_idx(turn, PIECE::KING)]);
}

bool move_generator::is_in_check_after(const move &mv) {
    if (turn == COLOUR::NONE)
        return false;

    board temp = current;
    current.apply_move(mv);

    if (is_in_check()) {
        current = temp;
        return true;
    } else {
        current = temp;
        return false;
    }
}

std::vector<move> move_generator::get_all_generic_moves() {
    if (turn == COLOUR::NONE)
        return {};

    auto bishops = get_bishop_moves();
    auto rooks = get_rook_moves();
    auto queens = get_queen_moves();
    auto knights = get_knight_moves();
    auto kings = get_king_moves();
    auto pawns = get_pawn_moves();

    std::vector<move> result;

    for (auto &mv : bishops) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    for (auto &mv : rooks) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    for (auto &mv : queens) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    for (auto &mv : knights) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    for (auto &mv : kings) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    for (auto &mv : pawns) {
        if (!is_in_check_after(mv))
            result.push_back(std::move(mv));
    }

    return result;
}