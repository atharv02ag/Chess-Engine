#pragma once
#include "board.h"
#include "move_generator.h"

template <typename rulebook> class game {
  public:
    board chess_board;
    COLOUR turn;
    rulebook rules;

    game();
    game(const string &fen);
    void show_board();
    bool make_move(const move &move_to_make);
};

template <typename rulebook> game<rulebook>::game() : turn(COLOUR::WHITE) { rules.init_board(chess_board); }

template <typename rulebook> game<rulebook>::game(const string &fen) : turn(COLOUR::WHITE) {

    int rank = 7;
    int file = 0;

    for (char c : fen) {
        if (c == ' ')
            break;

        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (isdigit(c)) {
            file += c - '0';
            continue;
        }

        // int piece = piece_index(c);
        // int sq = rank * 8 + file;

        // b.pieces[piece] |= (1ULL << sq);
        
        file++;
    }
}

template <typename rulebook> void game<rulebook>::show_board() { chess_board.print_board(); }

template <typename rulebook> bool game<rulebook>::make_move(const move &move_to_make) {

    if (move_to_make.colour == COLOUR::NONE || move_to_make.piece == PIECE::EMPTY || move_to_make.colour != turn)
        return false;

    vector<move> all_legal_moves = rules.get_all_legal_moves(chess_board, turn);

    bool found = false;
    for (const auto &m : all_legal_moves) {
        if (m == move_to_make)
            found = true;
    }
    if (!found)
        return false;
    if (move_to_make.castle_kside) {
        rules.castle_kside(chess_board, move_to_make.colour);
        turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
        return true;
    }
    if (move_to_make.castle_qside) {
        rules.castle_qside(chess_board, move_to_make.colour);
        turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
        return true;
    }
    chess_board.apply_move(move_to_make);
    rules.update_flags(chess_board, move_to_make);
    turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
    return true;
}
