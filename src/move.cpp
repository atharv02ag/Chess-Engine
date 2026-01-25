#include "../include/move.h"
#include "../include/move_generator.h"

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end)
    : piece(p), colour(c), start_location(start), end_location(end) {}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const PIECE &prom_piece)
    : piece(p), colour(c), start_location(start), end_location(end) {

    move_flags = 0;
    switch (prom_piece) {
    case PIECE::QUEEN:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN);
        break;
    case PIECE::ROOK:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_ROOK);
        break;
    case PIECE::BISHOP:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_BISHOP);
        break;
    case PIECE::KNIGHT:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_KNIGHT);
        break;
    default:
        break;
    }
}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap,
           const PIECE &prom_piece)
    : piece(p), colour(c), start_location(start), end_location(end) {
    move_flags = 0;
    switch (prom_piece) {
    case PIECE::QUEEN:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN);
        break;
    case PIECE::ROOK:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_ROOK);
        break;
    case PIECE::BISHOP:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_BISHOP);
        break;
    case PIECE::KNIGHT:
        move_flags |= static_cast<u32>(MOVE_FLAGS::PROMOTE_KNIGHT);
        break;
    default:
        break;
    }
    if (p_cap)
        move_flags |= static_cast<u32>(MOVE_FLAGS::CAPTURE);
}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap)
    : piece(p), colour(c), start_location(start), end_location(end) {

    move_flags = 0;
    if (p_cap)
        move_flags |= static_cast<u32>(MOVE_FLAGS::CAPTURE);
}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const bool &ep)
    : piece(p), colour(c), start_location(start), end_location(end) {

    move_flags = 0;
    if (!p_cap && ep) {
        cout << "invalid move : enpassant without capture" << endl;
        return;
    }
    if (p_cap)
        move_flags |= static_cast<u32>(MOVE_FLAGS::CAPTURE);
    if (ep)
        move_flags |= static_cast<u32>(MOVE_FLAGS::ENPASS);
}

move::move(const PIECE &p, const COLOUR &c, const bool &castle_k, const bool &castle_q) : piece(p), colour(c) {

    move_flags = 0;
    if (castle_k && castle_q) {
        cout << "invalid move : both castles true" << endl;
        return;
    }

    if (castle_k)
        move_flags |= static_cast<u32>(MOVE_FLAGS::CASTLE_KSIDE);
    if (castle_q)
        move_flags |= static_cast<u32>(MOVE_FLAGS::CASTLE_QSIDE);
}

bool move::piece_captured() const { return move_flags & static_cast<u32>(MOVE_FLAGS::CAPTURE); }

bool move::enpass() const { return move_flags & static_cast<u32>(MOVE_FLAGS::ENPASS); }

bool move::castle_kside() const { return move_flags & static_cast<u32>(MOVE_FLAGS::CASTLE_KSIDE); }

bool move::castle_qside() const { return move_flags & static_cast<u32>(MOVE_FLAGS::CASTLE_QSIDE); }

PIECE move::promotion_piece() const {
    if (move_flags & static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN))
        return PIECE::QUEEN;
    if (move_flags & static_cast<u32>(MOVE_FLAGS::PROMOTE_ROOK))
        return PIECE::ROOK;
    if (move_flags & static_cast<u32>(MOVE_FLAGS::PROMOTE_BISHOP))
        return PIECE::BISHOP;
    if (move_flags & static_cast<u32>(MOVE_FLAGS::PROMOTE_KNIGHT))
        return PIECE::KNIGHT;
    return PIECE::EMPTY;
}

void move::print_move() const {
    if (castle_kside()) {
        cout << "COLOUR : " << colour_names[static_cast<int>(colour)] << " CASTLED KSIDE" << endl;
        return;
    }
    if (castle_qside()) {
        cout << "COLOUR: " << colour_names[static_cast<int>(colour)] << " CASTLED QSIDE" << endl;
        return;
    }
    int start_row = lsb(start_location) / 8;
    int start_col = lsb(start_location) % 8;
    int end_row = lsb(end_location) / 8;
    int end_col = lsb(end_location) % 8;
    if (piece_captured())
        cout << "CAPTURE, ";
    if (enpass()) {
        cout << "ENPASSANT, ";
    }
    cout << "COLOUR : " << colour_names[static_cast<int>(colour)] << " PIECE : " << piece_names[static_cast<int>(piece)]
         << " START : (" << start_row << "," << start_col << ") END : (" << end_row << "," << end_col << ")";

    if (promotion_piece() != PIECE::EMPTY)
        cout << " PROMOTION to " << piece_names[static_cast<int>(promotion_piece())] << endl;
    else
        cout << endl;
}

bool move::operator==(const move &other) const {
    return (other.start_location == start_location && other.end_location == end_location &&
            other.move_flags == move_flags);
}