#include "../include/move.h"

u32 move::encode(PIECE piece, COLOUR colour, u64 start, u64 end, u32 flags, bool coordinates_present) {
    coordinates_present = coordinates_present && start != 0ULL && end != 0ULL;
    const u32 start_square = coordinates_present ? static_cast<u32>(lsb(start)) : 0U;
    const u32 end_square = coordinates_present ? static_cast<u32>(lsb(end)) : 0U;

    return ((start_square & SQUARE_MASK) << START_SHIFT) |
           ((end_square & SQUARE_MASK) << END_SHIFT) |
           ((static_cast<u32>(piece) & PIECE_MASK) << PIECE_SHIFT) |
           ((static_cast<u32>(colour) & COLOUR_MASK) << COLOUR_SHIFT) |
           ((flags & FLAGS_MASK) << FLAGS_SHIFT) |
           (coordinates_present ? COORDINATES_PRESENT_MASK : 0U);
}

u32 move::promotion_flag(PIECE promotion_piece) {
    switch (promotion_piece) {
    case PIECE::QUEEN:
        return static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN);
    case PIECE::ROOK:
        return static_cast<u32>(MOVE_FLAGS::PROMOTE_ROOK);
    case PIECE::BISHOP:
        return static_cast<u32>(MOVE_FLAGS::PROMOTE_BISHOP);
    case PIECE::KNIGHT:
        return static_cast<u32>(MOVE_FLAGS::PROMOTE_KNIGHT);
    default:
        return 0U;
    }
}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end)
    : data_(encode(p, c, start, end, 0U)) {}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const PIECE &prom_piece)
    : data_(encode(p, c, start, end, promotion_flag(prom_piece))) {}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap,
           const PIECE &prom_piece)
    : data_(encode(p, c, start, end,
                   promotion_flag(prom_piece) | (p_cap ? static_cast<u32>(MOVE_FLAGS::CAPTURE) : 0U))) {}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap)
    : data_(encode(p, c, start, end, p_cap ? static_cast<u32>(MOVE_FLAGS::CAPTURE) : 0U)) {}

move::move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const bool &ep)
    : data_(encode(p, c, start, end, 0U)) {
    if (!p_cap && ep) {
        cout << "invalid move : enpassant without capture" << endl;
        return;
    }

    const u32 move_flags = (p_cap ? static_cast<u32>(MOVE_FLAGS::CAPTURE) : 0U) |
                           (ep ? static_cast<u32>(MOVE_FLAGS::ENPASS) : 0U);
    data_ = encode(p, c, start, end, move_flags);
}

move::move(const PIECE &p, const COLOUR &c, const bool &castle_k, const bool &castle_q)
    : data_(encode(p, c, 0ULL, 0ULL, 0U, false)) {
    if (castle_k && castle_q) {
        cout << "invalid move : both castles true" << endl;
        return;
    }

    if (!castle_k && !castle_q)
        return;

    const int home_rank = c == COLOUR::BLACK ? 7 : 0;
    const u64 start = 1ULL << (home_rank * 8 + 4);
    const u64 end = 1ULL << (home_rank * 8 + (castle_k ? 6 : 2));
    const u32 move_flags = static_cast<u32>(castle_k ? MOVE_FLAGS::CASTLE_KSIDE : MOVE_FLAGS::CASTLE_QSIDE);
    data_ = encode(p, c, start, end, move_flags);
}

bool move::piece_captured() const { return flags() & static_cast<u32>(MOVE_FLAGS::CAPTURE); }

bool move::enpass() const { return flags() & static_cast<u32>(MOVE_FLAGS::ENPASS); }

bool move::castle_kside() const { return flags() & static_cast<u32>(MOVE_FLAGS::CASTLE_KSIDE); }

bool move::castle_qside() const { return flags() & static_cast<u32>(MOVE_FLAGS::CASTLE_QSIDE); }

PIECE move::promotion_piece() const {
    if (flags() & static_cast<u32>(MOVE_FLAGS::PROMOTE_QUEEN))
        return PIECE::QUEEN;
    if (flags() & static_cast<u32>(MOVE_FLAGS::PROMOTE_ROOK))
        return PIECE::ROOK;
    if (flags() & static_cast<u32>(MOVE_FLAGS::PROMOTE_BISHOP))
        return PIECE::BISHOP;
    if (flags() & static_cast<u32>(MOVE_FLAGS::PROMOTE_KNIGHT))
        return PIECE::KNIGHT;
    return PIECE::EMPTY;
}

void move::print_move() const {
    if (castle_kside()) {
        cout << "COLOUR : " << colour_names[static_cast<int>(colour())] << " CASTLED KSIDE" << endl;
        return;
    }
    if (castle_qside()) {
        cout << "COLOUR: " << colour_names[static_cast<int>(colour())] << " CASTLED QSIDE" << endl;
        return;
    }
    if (!has_coordinates()) {
        cout << "INVALID MOVE" << endl;
        return;
    }

    const int start_row = start_square() / 8;
    const int start_col = start_square() % 8;
    const int end_row = end_square() / 8;
    const int end_col = end_square() % 8;
    if (piece_captured())
        cout << "CAPTURE, ";
    if (enpass())
        cout << "ENPASSANT, ";
    cout << "COLOUR : " << colour_names[static_cast<int>(colour())] << " PIECE : "
         << piece_names[static_cast<int>(piece())] << " START : (" << start_row << "," << start_col << ") END : ("
         << end_row << "," << end_col << ")";

    if (promotion_piece() != PIECE::EMPTY)
        cout << " PROMOTION to " << piece_names[static_cast<int>(promotion_piece())] << endl;
    else
        cout << endl;
}
