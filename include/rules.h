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
    std::vector<move> get_castling_moves(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->get_castling_moves(chess_board, turn);
    }
    std::vector<move> get_all_legal_moves(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->get_all_legal_moves(chess_board, turn);
    }
    std::vector<move> get_all_legal_tactical_moves(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->get_all_legal_tactical_moves(chess_board, turn);
    }
    bool has_any_legal_move(const board &chess_board, const COLOUR &turn) {
        return static_cast<derived *>(this)->has_any_legal_move(chess_board, turn);
    }
};

class std_rules : public rules<std_rules> {
  public:
    void init_board(board &chess_board) {
        std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        chess_board.load_fen(start_fen);
    }

    void update_flags(board &chess_board, const move &move_made) {
        if (move_made.colour == COLOUR::NONE)
            return;
        update_castling_flags(chess_board, move_made);
        update_enpass_square(chess_board, move_made);
    }

    void castle_kside(board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return;
        if (turn == COLOUR::WHITE) {
            chess_board.apply_move(move(PIECE::KING, COLOUR::WHITE, (1ULL << 4), (1ULL << 6)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::WHITE, (1ULL << 7), (1ULL << 5)));
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE))];
            }
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE))];
            }
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
        } else {
            chess_board.apply_move(move(PIECE::KING, COLOUR::BLACK, (1ULL << (8 * 7) << 4), (1ULL << (8 * 7) << 6)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::BLACK, (1ULL << (8 * 7) << 7), (1ULL << (8 * 7) << 5)));
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK))];
            }
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK))];
            }
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
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE))];
            }
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE))];
            }
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
        } else {
            chess_board.apply_move(move(PIECE::KING, COLOUR::BLACK, (1ULL << (8 * 7) << 4), (1ULL << (8 * 7) << 2)));
            chess_board.apply_move(move(PIECE::ROOK, COLOUR::BLACK, (1ULL << (8 * 7)), (1ULL << (8 * 7) << 3)));
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK))];
            }
            if(chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK)){
                chess_board.zhash ^= zobrist::rv_flags[lsb(static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK))];
            }
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
            chess_board.flags &= ~static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
        }
    }

    std::vector<move> get_castling_moves(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return {};
        move_generator helper(chess_board, turn);
        std::vector<move> candidates;
        const u64 occupied = chess_board.all_pieces();
        if (turn == COLOUR::WHITE) {
            const u64 king_start = (1ULL << 4);
            const u64 kside_rook_start = (1ULL << 7);
            const u64 qside_rook_start = 1ULL;
            u64 rook_landing = (1ULL << 5);
            u64 king_landing = (1ULL << 6);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE)) &&
                (chess_board.pieces[get_piece_idx(COLOUR::WHITE, PIECE::KING)] & king_start) &&
                (chess_board.pieces[get_piece_idx(COLOUR::WHITE, PIECE::ROOK)] & kside_rook_start) &&
                !(occupied & rook_landing) && !(occupied & king_landing) &&
                !helper.is_sqr_attacked(rook_landing) && !helper.is_sqr_attacked(king_landing) && !helper.is_in_check()) {

                candidates.push_back(move(PIECE::KING, COLOUR::WHITE, true, false));
            }
            rook_landing = (1ULL << 3);
            king_landing = (1ULL << 2);
            u64 other_square = (1ULL << 1);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE)) &&
                (chess_board.pieces[get_piece_idx(COLOUR::WHITE, PIECE::KING)] & king_start) &&
                (chess_board.pieces[get_piece_idx(COLOUR::WHITE, PIECE::ROOK)] & qside_rook_start) &&
                !(occupied & rook_landing) && !(occupied & king_landing) &&
                !(occupied & other_square) && !helper.is_sqr_attacked(rook_landing) &&
                !helper.is_sqr_attacked(king_landing) && !helper.is_in_check()) {

                candidates.push_back(move(PIECE::KING, COLOUR::WHITE, false, true));
            }
        } else {
            const u64 king_start = (1ULL << (8 * 7) << 4);
            const u64 kside_rook_start = (1ULL << (8 * 7) << 7);
            const u64 qside_rook_start = (1ULL << (8 * 7));
            u64 rook_landing = (1ULL << (8 * 7) << 5);
            u64 king_landing = (1ULL << (8 * 7) << 6);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK)) &&
                (chess_board.pieces[get_piece_idx(COLOUR::BLACK, PIECE::KING)] & king_start) &&
                (chess_board.pieces[get_piece_idx(COLOUR::BLACK, PIECE::ROOK)] & kside_rook_start) &&
                !(occupied & rook_landing) && !(occupied & king_landing) &&
                !helper.is_sqr_attacked(rook_landing) && !helper.is_sqr_attacked(king_landing) && !helper.is_in_check()) {

                candidates.push_back(move(PIECE::KING, COLOUR::BLACK, true, false));
            }
            rook_landing = (1ULL << (8 * 7) << 3);
            king_landing = (1ULL << (8 * 7) << 2);
            u64 other_sqaure = (1ULL << (8 * 7) << 1);
            if ((chess_board.flags & static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK)) &&
                (chess_board.pieces[get_piece_idx(COLOUR::BLACK, PIECE::KING)] & king_start) &&
                (chess_board.pieces[get_piece_idx(COLOUR::BLACK, PIECE::ROOK)] & qside_rook_start) &&
                !(occupied & rook_landing) && !(occupied & king_landing) &&
                !(occupied & other_sqaure) && !helper.is_sqr_attacked(rook_landing) &&
                !helper.is_sqr_attacked(king_landing) && !helper.is_in_check()) {

                candidates.push_back(move(PIECE::KING, COLOUR::BLACK, false, true));
            }
        }

        return candidates;
    }

    std::vector<move> get_all_legal_moves(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return {};
        move_generator generator(chess_board, turn);
        std::vector<move> generic_moves = generator.get_all_generic_moves();
        std::vector<move> castling_moves = get_castling_moves(chess_board, turn);
        if (castling_moves.empty())
            return generic_moves;
        generic_moves.reserve(generic_moves.size() + castling_moves.size());
        for (auto &m : castling_moves) {
            generic_moves.push_back(std::move(m));
        }
        return generic_moves;
    }

    std::vector<move> get_all_legal_tactical_moves(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return {};
        move_generator generator(chess_board, turn);
        return generator.get_all_tactical_moves();
    }

    bool has_any_legal_move(const board &chess_board, const COLOUR &turn) {
        if (turn == COLOUR::NONE)
            return false;
        move_generator generator(chess_board, turn);
        if (generator.has_any_generic_move())
            return true;
        return !get_castling_moves(chess_board, turn).empty();
    }

  private:
    void update_castling_flags(board &chess_board, const move &move_made) {
        if (move_made.colour == COLOUR::NONE)
            return;

        auto clear_right = [&](FLAGS right) {
            const u32 mask = static_cast<u32>(right);
            if (chess_board.flags & mask) {
                chess_board.flags &= ~mask;
                chess_board.zhash ^= zobrist::rv_flags[lsb(mask)];
            }
        };

        // Castling rights are also lost when a rook is captured on its original square.
        if (move_made.piece_captured()) {
            if (move_made.end_location == 1ULL)
                clear_right(FLAGS::CASTLE_QSIDE_WHITE);
            else if (move_made.end_location == (1ULL << 7))
                clear_right(FLAGS::CASTLE_KSIDE_WHITE);
            else if (move_made.end_location == (1ULL << (8 * 7)))
                clear_right(FLAGS::CASTLE_QSIDE_BLACK);
            else if (move_made.end_location == (1ULL << (8 * 7) << 7))
                clear_right(FLAGS::CASTLE_KSIDE_BLACK);
        }

        if (move_made.colour == COLOUR::WHITE) {
            if (move_made.piece == PIECE::ROOK && move_made.start_location == (1ULL << 7))
                clear_right(FLAGS::CASTLE_KSIDE_WHITE);
            if (move_made.piece == PIECE::ROOK && move_made.start_location == 1ULL)
                clear_right(FLAGS::CASTLE_QSIDE_WHITE);
            if (move_made.piece == PIECE::KING) {
                clear_right(FLAGS::CASTLE_KSIDE_WHITE);
                clear_right(FLAGS::CASTLE_QSIDE_WHITE);
            }
        } else {
            if (move_made.piece == PIECE::ROOK && move_made.start_location == (1ULL << (8 * 7) << 7))
                clear_right(FLAGS::CASTLE_KSIDE_BLACK);
            if (move_made.piece == PIECE::ROOK && move_made.start_location == (1ULL << (8 * 7)))
                clear_right(FLAGS::CASTLE_QSIDE_BLACK);
            if (move_made.piece == PIECE::KING) {
                clear_right(FLAGS::CASTLE_KSIDE_BLACK);
                clear_right(FLAGS::CASTLE_QSIDE_BLACK);
            }
        }
    }

    void update_enpass_square(board &chess_board, const move &move_made) {
        if(chess_board.enpass_capture_square){
            int enpass_idx = lsb(chess_board.enpass_capture_square)%8 + 8 * (lsb(chess_board.enpass_capture_square)/8 <= 2);
            chess_board.zhash ^= zobrist::rv_enpass[enpass_idx];
        }
        if(move_made.colour == COLOUR::NONE || move_made.piece != PIECE::PAWN){
            chess_board.enpass_capture_square = 0ULL;
            return;
        }
        if(move_made.colour == COLOUR::WHITE){
            if(lsb(move_made.start_location)/8 == 1 && lsb(move_made.end_location)/8 == 3){
                chess_board.enpass_capture_square = move_made.start_location << 8;
                int enpass_idx = lsb(chess_board.enpass_capture_square)%8 + 8 * (lsb(chess_board.enpass_capture_square)/8 <= 2);
                chess_board.zhash ^= zobrist::rv_enpass[enpass_idx];
                return;
            }
        }
        else{
            if(lsb(move_made.start_location)/8 == 6 && lsb(move_made.end_location)/8 == 4){
                chess_board.enpass_capture_square = move_made.start_location >> 8;
                int enpass_idx = lsb(chess_board.enpass_capture_square)%8 + 8 * (lsb(chess_board.enpass_capture_square)/8 <= 2);
                chess_board.zhash ^= zobrist::rv_enpass[enpass_idx];
                return;
            }
        }
        chess_board.enpass_capture_square = 0ULL;
    }
};
