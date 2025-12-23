#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

// TODO : implement minimax board copy by reference - implement board.unmake(move)

// test 1 = r5k1/1p3ppp/6q1/pPPp1b2/3Nr3/4P2P/P1Q2PPK/1R3R2 b - - 4 20 - PASS
// test 2 = 5r1k/4r2p/2qpB3/B1p1bPP1/P3R1QP/1P6/6K1/8 b - - 0 39 - PASS
// test 3 = 4r1k1/1p3pp1/p2Qp3/3p1qBp/3P3P/PP6/2n2PP1/2R3K1 w - - 4 28 - PASS
// test 4 = r3k2r/ppp2ppp/3p4/2b5/4P2q/2N2b1P/PPP1BP2/R1B1QRK1 b kq - 1 13 - PASS


int main() {
    // freopen("log.txt", "w", stdout);
    game<std_rules> g;
    engine<std_rules> e;
    int depth = 6;
    g.show_board();
    for(int i = 0; i<50; i++){
        std::string enemy_move;
        cin >> enemy_move;
        move b = g.parse_move(enemy_move);
        g.make_move(b);
        cout << "iter : " << i << endl;
        g.show_board();
        auto [m, em] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
        if (m.piece == PIECE::EMPTY) {
            break;
        }
        m.print_move();
        cout << "eval : " << em << endl;
        g.make_move(m);
        g.show_board();
    }
    // g.show_board();
    // g.make_move(g.parse_move("g2g4"));
    // cout << g.chess_board.enpass_capture_square << endl;
    // g.show_board();
    // g.make_move(g.parse_move("c8g4"));
    // cout << g.chess_board.enpass_capture_square << endl;
    // g.show_board();
    // g.make_move(g.parse_move("f3g4"));
    // cout << g.chess_board.enpass_capture_square << endl;
    // g.show_board();
    // auto all = g.rules.get_all_legal_moves(g.chess_board, g.turn);
    // for (const auto &m : all) {
    //     m.print_move();
    // }

    return 0;
}