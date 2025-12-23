#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

// TODO : implement minimax board copy by reference - implement board.unmake(move)
// TODO : implement pawn promotion, en passant
// TODO : some errors in evaluation function. Check from fen = r1b1k1nB/1ppq3p/p1n2p2/8/4p3/P7/P1PPPPPP/1R1QKB1R w Kq - 1 11

int main() {
    game<std_rules> g ("rnbqkbnr/ppp1pppp/8/8/2PpPP2/8/PP1P2PP/RNBQKBNR b KQkq c3 0 3");
    // freopen("log.txt", "w", stdout);
    engine<std_rules> e;
    int depth = 4;
    g.show_board();
    cout << g.chess_board.enpass_capture_square << endl;

    //  move_generator mg(g.chess_board,g.turn);
    auto all = e.rules.get_all_legal_moves(g.chess_board, g.turn);
    // auto all = mg.get_all_generic_moves();
    for(auto m : all){
        m.print_move();
    }

    g.make_move(g.parse_move("d4c3"));
    g.show_board();
    // auto [w, ew] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
    // w.print_move();
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