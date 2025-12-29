#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

#include <chrono>

// TODO : implement minimax board copy by reference - implement board.unmake(move)

// test 1 = 8/5kpp/5p2/5K2/1pP1P1P1/p6P/P7/8 b - - 0 1 - PASS
// test 2 = 5rk1/1pq2p1p/2p2Bp1/p7/7P/1P4P1/P1P2P2/3Q2K1 w - - 0 1 - PASS
// test 3 = 6rk/pp3pp1/3qp2p/2R3n1/1P6/PN6/4QPPP/6K1 w - - 0 1 - PASS
// test 4 = 5B1k/p4Q1p/3qp1p1/3pNp2/4n3/1P2P2K/P4rPP/7R b - - 0 28 - PASS
// test 5 = 1rr3k1/pp3q1p/3p4/P2p2pN/n2Pp1Q1/2P1P3/2R2P1P/5BK1 b - - 0 29 - FAIL!


int main() {
    // freopen("log.txt", "w", stdout);
    game<std_rules> g("r1kr4/4P1p1/pBB2p1p/8/2p5/p1P2KP1/1P5P/8 w - - 0 33");
    engine<std_rules> e;
    int depth = 6;
    g.show_board();
    auto start1 = std::chrono::high_resolution_clock::now();
    auto [m , em] = e.minimax(g.chess_board, g.turn, depth, -INF, INF);
    auto end1 = std::chrono::high_resolution_clock::now();
    cout << "time without tt : " << std::chrono::duration<double>(end1 - start1).count() << endl;
    m.print_move();
    cout << "eval : " << em << endl;
    
    auto start2 = std::chrono::high_resolution_clock::now();
    auto [m1 , em1] = e.minimax_tt(g.chess_board, g.turn, depth, -INF, INF);
    auto end2 = std::chrono::high_resolution_clock::now();
    cout << "time with tt : " << std::chrono::duration<double>(end2 - start2).count() << endl;
    m.print_move();
    cout << "eval : " << em1 << endl;
    cout << "tt hits : " << e.tt_hits << ", nodes searched : " << e.nodes_seen << endl;
    
    g.make_move(m1);
    g.show_board();

    return 0;
}