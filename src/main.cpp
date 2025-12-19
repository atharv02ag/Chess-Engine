#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

// TODO : implement minimax board copy by reference - implement board.unmake(move)
// TODO : implement pawn promotion, en passant
// TODO : some errors in evaluation function. Check from fen = r1b2rk1/1pppqpp1/p4n2/1Bb1p2p/1n2P2P/2N2N2/PPPPQPP1/1RB2RK1 w - - 2 10

int main() {
    game<std_rules> g("r1b2rk1/1pppqpp1/p1n2n2/1Bb1p2p/4P2P/2N2N2/PPPPQPP1/R1B2RK1 w - - 0 9");
    // g.show_board();
    // g.make_move(g.parse_move("f3g5"));
    // g.show_board();
    // auto all = g.rules.get_all_legal_moves(g.chess_board, g.turn);
    // for(auto &mv : all){
    //     mv.print_move();
    // }

    freopen("log.txt", "w", stdout);
    engine<std_rules> e;
    g.show_board();
    int depth = 1;
    // g.make_move(g.parse_move("c6b4"));
    // g.make_move(g.parse_move("a6b5"));
    // g.show_board();
    // g.make_move(g.parse_move("a6b5"));
    // g.show_board();
    // auto all = g.rules.get_all_legal_moves(g.chess_board, g.turn);
    // for (auto &mv : all) {
    //     mv.print_move();
    // }   
    // auto [b2,eb2] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
    // b2.print_move();
    // cout << "eval : " << eb2 << endl;
    for (int i = 0; i < 10; i++) {
        cout << "iter : " << i << endl;
        g.show_board();
        auto [w, ew] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
        if (w.piece == PIECE::EMPTY) {
            break;
        }
        w.print_move();
        cout << "eval : " << ew << endl;
        g.make_move(w);
        auto [b, eb] = e.minimax(g.chess_board, COLOUR::BLACK, depth, -INF, INF);
        if (b.piece == PIECE::EMPTY) {
            break;
        }
        b.print_move();
        cout << "eval : " << eb << endl;
        g.make_move(b);
    }

    return 0;
}