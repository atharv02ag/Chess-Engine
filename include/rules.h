#pragma once
#include "board.h"
#include "move_generator.h"

// CRTP implementation

template <typename derived> class rules {
  public:
    void init_board(board &chess_board) { static_cast<derived *>(this)->init_board(chess_board); }
    void update_flags(board &chess_board, const move &move_made) {
        static_cast<derived *>(this)->update_flags(chess_board, move_made);
    }
    void castle_kside(board &chess_board, const COLOUR &turn) {
        static_cast<derived *>(this)->castle_kside(chess_board, turn);
    }
    void castle_qside(board &chess_board, const COLOUR &turn) {
        static_cast<derived *>(this)->castle_qside(chess_board, turn);
    }
    vector<move> get_castling_moves(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->get_castling_moves(chess_board, turn);
    }
    vector<move> get_all_legal_moves(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->get_all_legal_moves(chess_board, turn);
    }
};

class std_rules : public rules<std_rules> {
  public:
    void init_board(board &chess_board) {
        for (int col = 0; col < 8; col++) {
            chess_board.set_piece((1ULL << 8 << col), PIECE::PAWN, COLOUR::WHITE);
            chess_board.set_piece((1ULL << (8 * 6) << col), PIECE::PAWN, COLOUR::BLACK);
        }

        chess_board.set_piece((1ULL), PIECE::ROOK, COLOUR::WHITE);
        chess_board.set_piece((1ULL << 7), PIECE::ROOK, COLOUR::WHITE);
        chess_board.set_piece((1ULL << (8 * 7)), PIECE::ROOK, COLOUR::BLACK);
        chess_board.set_piece((1ULL << (8 * 7) << 7), PIECE::ROOK, COLOUR::BLACK);

        chess_board.set_piece((1ULL << 1), PIECE::KNIGHT, COLOUR::WHITE);
        chess_board.set_piece((1ULL << 6), PIECE::KNIGHT, COLOUR::WHITE);
        chess_board.set_piece((1ULL << (8 * 7) << 1), PIECE::KNIGHT, COLOUR::BLACK);
        chess_board.set_piece((1ULL << (8 * 7) << 6), PIECE::KNIGHT, COLOUR::BLACK);

        chess_board.set_piece((1ULL << 2), PIECE::BISHOP, COLOUR::WHITE);
        chess_board.set_piece((1ULL << 5), PIECE::BISHOP, COLOUR::WHITE);
        chess_board.set_piece((1ULL << (8 * 7) << 2), PIECE::BISHOP, COLOUR::BLACK);
        chess_board.set_piece((1ULL << (8 * 7) << 5), PIECE::BISHOP, COLOUR::BLACK);

        chess_board.set_piece((1ULL << 3), PIECE::QUEEN, COLOUR::WHITE);
        chess_board.set_piece((1ULL << 4), PIECE::KING, COLOUR::WHITE);
        chess_board.set_piece((1ULL << (8 * 7) << 3), PIECE::QUEEN, COLOUR::BLACK);
        chess_board.set_piece((1ULL << (8 * 7) << 4), PIECE::KING, COLOUR::BLACK);
    }

    void update_flags(board &chess_board, const move &move_made) {
        if (move_made.colour == COLOUR::NONE)
            return;
        update_castling_flags(chess_board, move_made);
    }

    void castle_kside(board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return;
        if (turn == COLOUR::WHITE) {
            chess_board.apply_move(move(PIECE::KING, COLOUR::WHITE, (1ULL << 4), (1ULL << 6)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::WHITE, (1ULL << 7), (1ULL << 5)));
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
        } else {
            chess_board.apply_move(move(PIECE::KING, COLOUR::BLACK, (1ULL << (8 * 7) << 4), (1ULL << (8 * 7) << 6)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::BLACK, (1ULL << (8 * 7) << 7), (1ULL << (8 * 7) << 5)));
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
        }
    }

    void castle_qside(board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return;
        if (turn == COLOUR::WHITE) {
            chess_board.apply_move(move(PIECE::KING, COLOUR::WHITE, (1ULL << 4), (1ULL << 2)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::WHITE, 1ULL, (1ULL << 3)));
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
        } else {
            chess_board.apply_move(move(PIECE::KING, COLOUR::WHITE, (1ULL << (8 * 7) << 4), (1ULL << (8 * 7) << 2)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::WHITE, (1ULL << (8 * 7)), (1ULL << (8 * 7) << 3)));
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
        }
    }

    vector<move> get_castling_moves(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return {};
        move_generator helper(chess_board, turn);
        vector<move> candidates;
        if (turn == COLOUR::WHITE) {
            u64 rook_landing = (1ULL << 5);
            u64 king_landing = (1ULL << 6);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)) &&
                !(chess_board.all_white_pieces() & rook_landing) && !(chess_board.all_white_pieces() & king_landing) &&
                !helper.is_sqr_attacked(rook_landing) && !helper.is_sqr_attacked(king_landing)) {

                candidates.push_back(move(PIECE::KING, COLOUR::WHITE, true, false));
            }
            rook_landing = (1ULL << 3);
            king_landing = (1ULL << 2);
            u64 other_square = (1ULL << 1);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE)) &&
                !(chess_board.all_white_pieces() & rook_landing) && !(chess_board.all_white_pieces() & king_landing) &&
                !(chess_board.all_white_pieces() & other_square) && !helper.is_sqr_attacked(rook_landing) &&
                !helper.is_sqr_attacked(king_landing)) {

                candidates.push_back(move(PIECE::KING, COLOUR::WHITE, false, true));
            }
        } else {
            u64 rook_landing = (1ULL << (8 * 7) << 5);
            u64 king_landing = (1ULL << (8 * 7) << 6);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)) &&
                !(chess_board.all_black_pieces() & rook_landing) && !(chess_board.all_black_pieces() & king_landing) &&
                !helper.is_sqr_attacked(rook_landing) && !helper.is_sqr_attacked(king_landing)) {

                candidates.push_back(move(PIECE::KING, COLOUR::BLACK, true, false));
            }
            rook_landing = (1ULL << (8 * 7) << 3);
            king_landing = (1ULL << (8 * 7) << 2);
            u64 other_sqaure = (1ULL << (8 * 7) << 1);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK)) &&
                !(chess_board.all_black_pieces() & rook_landing) && !(chess_board.all_black_pieces() & king_landing) &&
                !(chess_board.all_black_pieces() & other_sqaure) && !helper.is_sqr_attacked(rook_landing) &&
                !helper.is_sqr_attacked(king_landing)) {

                candidates.push_back(move(PIECE::KING, COLOUR::BLACK, false, true));
            }
        }

        return candidates;
    }

    vector<move> get_all_legal_moves(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return {};
        move_generator generator(chess_board, turn);
        vector<move> generic_moves = generator.get_all_generic_moves();
        vector<move> castling_moves = get_castling_moves(chess_board, turn);
        if (castling_moves.empty())
            return generic_moves;
        vector<move> result;
        for (auto &m : generic_moves) {
            result.push_back(std::move(m));
        }
        for (auto &m : castling_moves) {
            result.push_back(std::move(m));
        }
        return result;
    }

  private:
    void update_castling_flags(board &chess_board, const move &move_made) {
        if (move_made.colour == COLOUR::NONE)
            return;
        if (move_made.colour == COLOUR::WHITE) {
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)) && move_made.piece == PIECE::ROOK &&
                move_made.start_location == (1ULL << 7)) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
            }
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE)) && move_made.piece == PIECE::ROOK &&
                move_made.start_location == 1ULL) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
            }
            if (((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)) ||
                 (chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE))) &&
                move_made.piece == PIECE::KING) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
            }
        } else {
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)) && move_made.piece == PIECE::ROOK &&
                move_made.start_location == (1ULL << (8 * 7) << 7)) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
            }
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK)) && move_made.piece == PIECE::ROOK &&
                move_made.start_location == (1ULL << (8 * 7))) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
            }
            if (((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)) ||
                 (chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK))) &&
                move_made.piece == PIECE::KING) {
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
                chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
            }
        }
    }
};
