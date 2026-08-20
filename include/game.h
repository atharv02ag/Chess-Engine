#pragma once
#include "board.h"
#include "move_generator.h"

template <typename rulebook> class game {
  public:
    board chess_board;
    COLOUR turn;
    rulebook rules;

    game();
    game(const std::string &fen);
    void show_board();
    move parse_move(const std::string &move_str);
    bool make_move(const move &move_to_make);
};

template <typename rulebook> game<rulebook>::game() : turn(COLOUR::WHITE) { rules.init_board(chess_board); }

template <typename rulebook> game<rulebook>::game(const std::string &fen) {
    chess_board.load_fen(fen);
    turn = chess_board.turn;
}

template <typename rulebook> void game<rulebook>::show_board() { chess_board.print_board(); }

// Parses moves in format "e2e4", "g1f3", "a7a8q" (for promotion),
// k" and "q" (for black castling), "K" and "Q" (for white castling)
template <typename rulebook> move game<rulebook>::parse_move(const std::string &move_str) {
    if (move_str[0] == 'k') {
        return move(PIECE::KING, COLOUR::BLACK, true, false);
    } else if (move_str[0] == 'q') {
        return move(PIECE::KING, COLOUR::BLACK, false, true);
    } else if (move_str[0] == 'K') {
        return move(PIECE::KING, COLOUR::WHITE, true, false);
    } else if (move_str[0] == 'Q') {
        return move(PIECE::KING, COLOUR::WHITE, false, true);
    }
    int start_col = move_str[0] - 'a';
    int start_row = move_str[1] - '1';
    int end_col = move_str[2] - 'a';
    int end_row = move_str[3] - '1';

    u64 start_location = 1ULL << (start_row * 8) << start_col;
    u64 end_location = 1ULL << (end_row * 8) << end_col;

    auto [start_piece_colour, start_piece_type] = chess_board.get_piece(start_location);

    if (start_piece_colour != COLOUR::NONE && start_piece_type != PIECE::EMPTY) {
        auto [end_piece_colour, end_piece_type] = chess_board.get_piece(end_location);

        // promotion move detection
        if (move_str.length() == 5) {
            PIECE promo_piece = PIECE::EMPTY;
            switch (move_str[4]) {
            case 'q':
                promo_piece = PIECE::QUEEN;
                break;
            case 'r':
                promo_piece = PIECE::ROOK;
                break;
            case 'b':
                promo_piece = PIECE::BISHOP;
                break;
            case 'n':
                promo_piece = PIECE::KNIGHT;
                break;
            default:
                promo_piece = PIECE::EMPTY;
                break;
            }
            if (promo_piece != PIECE::EMPTY)
                // promo + capture detection
                if (end_piece_type != PIECE::EMPTY)
                    return move(start_piece_type, start_piece_colour, start_location, end_location, true, promo_piece);
                else
                    return move(start_piece_type, start_piece_colour, start_location, end_location, promo_piece);
            else
                return move(PIECE::EMPTY, COLOUR::NONE, start_location, end_location);
        }

        // enpass move detection
        if (start_piece_type == PIECE::PAWN && end_location == chess_board.enpass_capture_square)
            return move(start_piece_type, start_piece_colour, start_location, end_location, true, true);

        // capture move detection
        if (end_piece_type != PIECE::EMPTY)
            return move(start_piece_type, start_piece_colour, start_location, end_location, true);

        return move(start_piece_type, start_piece_colour, start_location, end_location);
    }

    // invalid move
    return move(PIECE::EMPTY, COLOUR::NONE, start_location, end_location);
}

template <typename rulebook> bool game<rulebook>::make_move(const move &move_to_make) {

    if (move_to_make.colour == COLOUR::NONE || move_to_make.piece == PIECE::EMPTY || move_to_make.colour != turn)
        return false;

    std::vector<move> all_legal_moves = rules.get_all_legal_moves(chess_board, turn);

    bool found = false;
    for (const auto &m : all_legal_moves) {
        if (m == move_to_make)
            found = true;
    }
    if (!found)
        return false;
    if (move_to_make.castle_kside()) {
        rules.castle_kside(chess_board, move_to_make.colour);
    } else if (move_to_make.castle_qside()) {
        rules.castle_qside(chess_board, move_to_make.colour);
    } else {
        chess_board.apply_move(move_to_make);
    }
    rules.update_flags(chess_board, move_to_make);

    turn = chess_board.turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
    chess_board.zhash ^= zobrist::rv_colour;
    return true;
}
