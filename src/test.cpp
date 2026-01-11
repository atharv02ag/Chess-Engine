#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"
#include "../include/magic.h"

u64 get_bishop_moves(const u64& bishop_pos, const board& b){
    int piece_pos = lsb(bishop_pos);
    u64 blockers = (b.all_pieces() & bishop_movement_masks_no_edges[piece_pos]);
    int idx = u64(bishop_magic_keys[piece_pos] * blockers) >> bishop_shifts[piece_pos];
    return bishop_table[piece_pos][idx];
}

u64 get_rook_moves(const u64& rook_pos, const board& b){
    int piece_pos = lsb(rook_pos);
    u64 blockers = (b.all_pieces() & rook_movement_masks_no_edges[piece_pos]);
    int idx = u64(rook_magic_keys[piece_pos] * blockers) >> rook_shifts[piece_pos];
    return rook_table[piece_pos][idx];
}

int main() {
    
    game<std_rules> g1("r1bqkbn1/ppp3p1/2np3r/4p2p/P3P2P/1P5R/2PP1PP1/RNBQKB2 w Qq - 1 8");

    g1.show_board();
    auto temp = g1.chess_board.pieces[get_piece_idx(COLOUR::WHITE, PIECE::ROOK)];
    while(temp){
        auto cur = temp ^ (temp & (temp-1));
        print_bitboard(get_rook_moves(cur, g1.chess_board));
        cout << endl;
        temp &= temp-1;
    }
    temp = g1.chess_board.pieces[get_piece_idx(COLOUR::BLACK, PIECE::ROOK)];
    while(temp){
        auto cur = temp ^ (temp & (temp-1));
        print_bitboard(get_rook_moves(cur, g1.chess_board));
        cout << endl;
        temp &= temp-1;
    }

    return 0;
}