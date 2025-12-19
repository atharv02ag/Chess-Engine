#include "../include/board.h"

board::board() {
    white_pawns = white_king = white_queens = white_bishops = white_knights = white_rooks = 0ULL;
    black_pawns = black_king = black_queens = black_bishops = black_knights = black_rooks = 0ULL;
    flags = 0;
    flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
    flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
    flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
    flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
}

u64 board::all_white_pieces() const {
    return white_pawns ^ white_king ^ white_knights ^ white_bishops ^ white_queens ^ white_rooks;
}

u64 board::all_black_pieces() const {
    return black_pawns ^ black_king ^ black_knights ^ black_bishops ^ black_queens ^ black_rooks;
}

u64 board::all_pieces() const { return all_white_pieces() ^ all_black_pieces(); }

bool board::set_piece(u64 location, PIECE piece, COLOUR colour) {
    if (!location || pop_ct(location) > 1) {
        cout << "board::set_piece : invalid location : " << location
             << " colour : " << colour_names[static_cast<int>(colour)] << endl;
        cout << piece_names[static_cast<int>(piece)] << endl;
        return false;
    }

    if (piece == PIECE::EMPTY) {
        white_pawns &= ~location;
        white_king &= ~location;
        white_queens &= ~location;
        white_bishops &= ~location;
        white_knights &= ~location;
        white_rooks &= ~location;
        black_pawns &= ~location;
        black_king &= ~location;
        black_queens &= ~location;
        black_knights &= ~location;
        black_rooks &= ~location;
        black_bishops &= ~location;
        return true;
    }
    if (colour == COLOUR::WHITE) {
        switch (piece) {
        case PIECE::PAWN:
            white_pawns |= location;
            break;
        case PIECE::ROOK:
            white_rooks |= location;
            break;
        case PIECE::KNIGHT:
            white_knights |= location;
            break;
        case PIECE::BISHOP:
            white_bishops |= location;
            break;
        case PIECE::KING:
            white_king |= location;
            break;
        case PIECE::QUEEN:
            white_queens |= location;
            break;
        default:
            return false;
        }
    } else if (colour == COLOUR::BLACK) {
        switch (piece) {
        case PIECE::PAWN:
            black_pawns |= location;
            break;
        case PIECE::ROOK:
            black_rooks |= location;
            break;
        case PIECE::KNIGHT:
            black_knights |= location;
            break;
        case PIECE::BISHOP:
            black_bishops |= location;
            break;
        case PIECE::KING:
            black_king |= location;
            break;
        case PIECE::QUEEN:
            black_queens |= location;
            break;
        default:
            return false;
        }
    }
    return true;
}

pair<COLOUR, PIECE> board::get_piece(u64 location) const {

    if (!location || pop_ct(location) > 1) {
        cout << "board::get_piece : invalid location" << endl;
        return {COLOUR::NONE, PIECE::EMPTY};
    }

    if (white_bishops & location)
        return {COLOUR::WHITE, PIECE::BISHOP};
    else if (white_rooks & location)
        return {COLOUR::WHITE, PIECE::ROOK};
    else if (white_knights & location)
        return {COLOUR::WHITE, PIECE::KNIGHT};
    else if (white_king & location)
        return {COLOUR::WHITE, PIECE::KING};
    else if (white_pawns & location)
        return {COLOUR::WHITE, PIECE::PAWN};
    else if (white_queens & location)
        return {COLOUR::WHITE, PIECE::QUEEN};
    else if (black_bishops & location)
        return {COLOUR::BLACK, PIECE::BISHOP};
    else if (black_rooks & location)
        return {COLOUR::BLACK, PIECE::ROOK};
    else if (black_knights & location)
        return {COLOUR::BLACK, PIECE::KNIGHT};
    else if (black_king & location)
        return {COLOUR::BLACK, PIECE::KING};
    else if (black_pawns & location)
        return {COLOUR::BLACK, PIECE::PAWN};
    else if (black_queens & location)
        return {COLOUR::BLACK, PIECE::QUEEN};
    else
        return {COLOUR::NONE, PIECE::EMPTY};
}

void board::apply_move(const move &move_to_make) {
    // ASSUMPTION : the colour and piece returned by get_piece() is the same corresponding to move_to_make

    if (move_to_make.colour == COLOUR::NONE || move_to_make.piece == PIECE::EMPTY) {
        cout << "board::apply_move : invalid move" << endl;
        return;
    }

    if (!move_to_make.start_location || !move_to_make.end_location) {
        cout << "board::apply_move : invalid locations" << endl;
        move_to_make.print_move();
    }

    set_piece(move_to_make.start_location, PIECE::EMPTY, COLOUR::NONE);
    set_piece(move_to_make.end_location, PIECE::EMPTY, COLOUR::NONE);
    set_piece(move_to_make.end_location, move_to_make.piece, move_to_make.colour);
}

void board::print_board() const {
    for (int r = 0; r < 8; r++) {
        cout << "| ";
        for (int c = 0; c < 8; c++) {
            u64 location = (1ULL << (8 * r)) << c;
            auto [colour, piece] = get_piece(location);
            if (colour == COLOUR::WHITE)
                cout << "W_";
            else if (colour == COLOUR::BLACK)
                cout << "B_";
            else
                cout << "  ";
            if (piece == PIECE::BISHOP)
                cout << "B";
            else if (piece == PIECE::KNIGHT)
                cout << "N";
            else if (piece == PIECE::ROOK)
                cout << "R";
            else if (piece == PIECE::KING)
                cout << "K";
            else if (piece == PIECE::QUEEN)
                cout << "Q";
            else if (piece == PIECE::PAWN)
                cout << "P";
            else
                cout << " ";
            cout << " | ";
        }
        cout << "\n";
    }
    cout << endl;
}