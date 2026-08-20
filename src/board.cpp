#include "../include/board.h"

#include <cctype>

board::board() {
    for (int i = 0; i < 12; i++)
        pieces[i] = 0ULL;
    turn = COLOUR::WHITE;
    enpass_capture_square = 0ULL;
    zhash = 0ULL;
    flags = 0;
    flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
    flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
    flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
    flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);
}

void board::init_hash(){
    zhash = (turn == COLOUR::WHITE) ? zobrist::rv_colour : 0ULL;
    for(int i = 0; i<12; i++){
        for(int j = 0; j<64; j++){
            u64 loc = 1ULL << j;
            if(pieces[i] & loc) zhash ^= zobrist::rv_pieces[i][j];
        }
    }
    u64 cur = flags;
    while(cur){
        zhash ^= zobrist::rv_flags[lsb(cur)];
        cur &= cur-1;
    }
    if(enpass_capture_square){
        int enpass_idx = lsb(enpass_capture_square)%8 + 8 * (lsb(enpass_capture_square)/8 <= 2);
        zhash ^= zobrist::rv_enpass[enpass_idx];
    }
}

void board::load_fen(const std::string &fen) {
    for (u64 &piece_set : pieces)
        piece_set = 0ULL;

    std::stringstream ss(fen);
    std::string placement, side, castling, ep;
    int halfmove, fullmove;

    ss >> placement >> side >> castling >> ep >> halfmove >> fullmove;

    int rank = 7;
    int file = 0;

    for (char c : placement) {
        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (isdigit(c)) {
            file += c - '0';
            continue;
        }

        int piece = -1;
        switch (c) {
        case 'P':
            piece = 0;
            break;
        case 'R':
            piece = 1;
            break;
        case 'N':
            piece = 2;
            break;
        case 'B':
            piece = 3;
            break;
        case 'K':
            piece = 4;
            break;
        case 'Q':
            piece = 5;
            break;
        case 'p':
            piece = 6;
            break;
        case 'r':
            piece = 7;
            break;
        case 'n':
            piece = 8;
            break;
        case 'b':
            piece = 9;
            break;
        case 'k':
            piece = 10;
            break;
        case 'q':
            piece = 11;
            break;
        default:
            piece = -1;
            break;
        }
        int sq = rank * 8 + file;
        pieces[piece] |= (1ULL << sq);
        file++;
    }

    turn = (side == "w") ? COLOUR::WHITE : COLOUR::BLACK;
    flags = 0;
    if (castling.find('K') != std::string::npos)
        flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_WHITE);
    if (castling.find('Q') != std::string::npos)
        flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_WHITE);
    if (castling.find('k') != std::string::npos)
        flags |= static_cast<u32>(FLAGS::CASTLE_KSIDE_BLACK);
    if (castling.find('q') != std::string::npos)
        flags |= static_cast<u32>(FLAGS::CASTLE_QSIDE_BLACK);

    if (ep != "-") {
        int ep_file = ep[0] - 'a';
        int ep_rank = ep[1] - '1';
        enpass_capture_square = (1ULL << (ep_rank * 8 + ep_file));
    } else {
        enpass_capture_square = 0ULL;
    }
    // halmove and fullmove ma chudaye
    init_hash();
}

u64 board::all_white_pieces() const {
    u64 all_wp = 0ULL;
    for (int i = 0; i < 6; i++)
        all_wp |= pieces[i];
    return all_wp;
}

u64 board::all_black_pieces() const {
    u64 all_bp = 0ULL;
    for (int i = 6; i < 12; i++)
        all_bp |= pieces[i];
    return all_bp;
}

u64 board::all_pieces() const { return all_white_pieces() ^ all_black_pieces(); }

bool board::set_piece(const u64 &location, const PIECE &piece, const COLOUR &colour) {
    if (!location || pop_ct(location) > 1) {
        cout << "board::set_piece : invalid location : " << location
             << " colour : " << colour_names[static_cast<int>(colour)] << endl;
        cout << piece_names[static_cast<int>(piece)] << endl;
        return false;
    }

    if (piece == PIECE::EMPTY) {
        for (int i = 0; i < 12; i++) {
            if(pieces[i] & location){
                zhash ^= zobrist::rv_pieces[i][lsb(location)];
            }
            pieces[i] &= ~location;
        }
        return true;
    } else {
        int piece_idx = get_piece_idx(colour, piece);
        pieces[piece_idx] |= location;
        zhash ^= zobrist::rv_pieces[piece_idx][lsb(location)];
    }
    return true;
}

std::pair<COLOUR, PIECE> board::get_piece(const u64 &location) const {

    if (!location || pop_ct(location) > 1) {
        cout << "board::get_piece : invalid location" << endl;
        return {COLOUR::NONE, PIECE::EMPTY};
    }

    COLOUR colour = COLOUR::NONE;
    PIECE piece = PIECE::EMPTY;
    for (int i = 0; i < 12; i++) {
        if (pieces[i] & location) {
            if (i < 6)
                colour = COLOUR::WHITE;
            else
                colour = COLOUR::BLACK;
            piece = static_cast<PIECE>(i % 6 + 1);
            break;
        }
    }
    return std::pair{colour, piece};
}

// NOTE : Castling is handled by 'rules' class, called in games::make_move
// ASSUMPTION : the colour and piece returned by get_piece() is the same corresponding to move_to_make

void board::apply_move(const move &move_to_make) {
    if (move_to_make.colour == COLOUR::NONE || move_to_make.piece == PIECE::EMPTY) {
        cout << "board::apply_move : invalid move" << endl;
        return;
    }

    if (!move_to_make.start_location || !move_to_make.end_location) {
        cout << "board::apply_move : invalid locations" << endl;
        move_to_make.print_move();
        print_board();
    }

    // enpassant move captures the pawn behind the target square
    if (move_to_make.enpass()) {
        if (move_to_make.colour == COLOUR::WHITE) {
            u64 captured_pawn_location = move_to_make.end_location >> 8;
            set_piece(captured_pawn_location, PIECE::EMPTY, COLOUR::NONE);
        } else {
            u64 captured_pawn_location = move_to_make.end_location << 8;
            set_piece(captured_pawn_location, PIECE::EMPTY, COLOUR::NONE);
        }
    }

    set_piece(move_to_make.start_location, PIECE::EMPTY, COLOUR::NONE);
    set_piece(move_to_make.end_location, PIECE::EMPTY, COLOUR::NONE);
    
    if (move_to_make.promotion_piece() != PIECE::EMPTY)
        set_piece(move_to_make.end_location, move_to_make.promotion_piece(), move_to_make.colour);
    else
        set_piece(move_to_make.end_location, move_to_make.piece, move_to_make.colour);
}

void board::print_board(COLOUR perspective) const {
    const bool flipped = perspective == COLOUR::BLACK;
    auto piece_symbol = [](PIECE piece, COLOUR colour) {
        char symbol = '.';
        switch (piece) {
            case PIECE::PAWN: symbol = 'P'; break;
            case PIECE::ROOK: symbol = 'R'; break;
            case PIECE::KNIGHT: symbol = 'N'; break;
            case PIECE::BISHOP: symbol = 'B'; break;
            case PIECE::KING: symbol = 'K'; break;
            case PIECE::QUEEN: symbol = 'Q'; break;
            default: return symbol;
        }
        return colour == COLOUR::BLACK ? static_cast<char>(std::tolower(symbol)) : symbol;
    };

    cout << "\n    ";
    for (int column = 0; column < 8; ++column) {
        const int file = flipped ? 7 - column : column;
        cout << static_cast<char>('a' + file) << ' ';
    }
    cout << "\n  +-----------------+\n";
    for (int row = 0; row < 8; ++row) {
        const int rank = flipped ? row : 7 - row;
        cout << rank + 1 << " | ";
        for (int column = 0; column < 8; ++column) {
            const int file = flipped ? 7 - column : column;
            const auto [colour, piece] = get_piece(1ULL << (rank * 8 + file));
            cout << piece_symbol(piece, colour) << ' ';
        }
        cout << "| " << rank + 1 << '\n';
    }
    cout << "  +-----------------+\n    ";
    for (int column = 0; column < 8; ++column) {
        const int file = flipped ? 7 - column : column;
        cout << static_cast<char>('a' + file) << ' ';
    }
    cout << "\n\nUppercase = White, lowercase = Black\n";
}
