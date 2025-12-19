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
    bool make_move(const move &move_to_make);
};

template <typename rulebook> game<rulebook>::game() : turn(COLOUR::WHITE) { rules.init_board(chess_board); }

template <typename rulebook> game<rulebook>::game(const std::string &fen) : turn(COLOUR::WHITE) {

    std::stringstream ss(fen);
    std::string board, side, castling, ep;
    int halfmove, fullmove;

    ss >> board >> side >> castling >> ep >> halfmove >> fullmove;

    int rank = 7;
    int file = 7;

    for (char c : fen) {
        if (c == ' ')
            break;

        if (c == '/') {
            rank--;
            file = 7;
            continue;
        }

        if (isdigit(c)) {
            file -= c - '0';
            continue;
        }

        int piece = -1;
        switch(c){
            case 'P' : piece = 0; break;
            case 'R' : piece = 1; break;
            case 'N' : piece = 2; break;
            case 'B' : piece = 3; break;
            case 'K' : piece = 4; break;
            case 'Q' : piece = 5; break;
            case 'p' : piece = 6; break;
            case 'r' : piece = 7; break;
            case 'n' : piece = 8; break;    
            case 'b' : piece = 9; break;
            case 'k' : piece = 10; break;
            case 'q' : piece = 11; break;
            default : piece = -1; break;
        }
        int sq = rank * 8 + file;
        chess_board.pieces[piece] |= (1ULL << sq);
        file--;
    }

    turn = (side == "w") ? COLOUR::WHITE : COLOUR::BLACK;
    if(castling.find('K') != std::string::npos)
        chess_board.flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
    if(castling.find('Q') !=  std::string::npos)
        chess_board.flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
    if(castling.find('k') != std::string::npos)
        chess_board.flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);   
    if(castling.find('q') != std::string::npos)
        chess_board.flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
    
    // enpass left unimplemented
    // halmove and fullmove ma chudaye
}

template <typename rulebook> void game<rulebook>::show_board() { chess_board.print_board(); }

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
