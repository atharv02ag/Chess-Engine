#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

// TODO : implement minimax board copy by reference - implement board.unmake(move)
// TODO : implement pawn promotion, en passant

// test 1 = r5k1/1p3ppp/6q1/pPPp1b2/3Nr3/4P2P/P1Q2PPK/1R3R2 b - - 4 20 - PASS
// test 2 = 5r1k/4r2p/2qpB3/B1p1bPP1/P3R1QP/1P6/6K1/8 b - - 0 39 - PASS
// test 3 = 4r1k1/1p3pp1/p2Qp3/3p1qBp/3P3P/PP6/2n2PP1/2R3K1 w - - 4 28 - PASS
// test 4 = r3k2r/ppp2ppp/3p4/2b5/4P2q/2N2b1P/PPP1BP2/R1B1QRK1 b kq - 1 13 - PASS


int main() {
    // freopen("log.txt", "w", stdout);
    game<std_rules> g ("4k2r/Pp6/1P1p1K1b/2pP4/2P4P/8/8/R7 b - - 0 1");
    // auto all = g.rules.get_all_legal_moves(g.chess_board, g.turn);
    // for(auto a : all){
    //     a.print_move();
    // }
    // cout << g.chess_board.flags << endl;
    engine<std_rules> e;
    int depth = 4;
    g.show_board();
    auto [m, em] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
    m.print_move();
    cout << "eval : " << em << endl;
    g.make_move(m);
    g.show_board();
    g.make_move(g.parse_move("h4g5"));
    g.show_board();
    auto [m1, em1] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
    m1.print_move();
    cout << "eval : " << em1 << endl;
    g.make_move(m1);
    g.show_board();
    g.make_move(g.parse_move("a7a8q"));
    g.show_board();
    auto [m2, em2] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
    m2.print_move();
    cout << "eval : " << em2 << endl;
    g.make_move(m2);
    g.show_board();

    // g.make_move(g.parse_move("f5e6"));
    // g.show_board();
    // auto [m2, em2] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
    // m2.print_move();
    // cout << "eval : " << em2 << endl;
    // g.make_move(m2);
    // g.show_board();
    // for(int i = 0; i<20; i++){
    //     cout << "iter : " << i << endl;
    //     g.show_board();
    //     auto [w, ew] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
    //     if (w.piece == PIECE::EMPTY) {
    //         break;
    //     }
    //     w.print_move();
    //     cout << "eval : " << ew << endl;
    //     g.make_move(w);
    //     g.show_board();
    //     std::string enemy_move;
    //     cin >> enemy_move;
    //     move b = g.parse_move(enemy_move);
    //     g.make_move(b);
    // }
    // g.show_board();
    // for (int i = 0; i < 10; i++) {
    //     cout << "iter : " << i << endl;
    //     g.show_board();
    //     auto [w, ew] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
    //     if (w.piece == PIECE::EMPTY) {
    //         break;
    //     }
    //     w.print_move();
    //     cout << "eval : " << ew << endl;
    //     g.make_move(w);
    //     auto [b, eb] = e.minimax(g.chess_board, COLOUR::BLACK, depth, -INF, INF);
    //     if (b.piece == PIECE::EMPTY) {
    //         break;
    //     }
    //     b.print_move();
    //     cout << "eval : " << eb << endl;
    //     g.make_move(b);
    // }

    return 0;
}