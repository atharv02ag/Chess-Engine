#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

int main() {
    game<std_rules> g;

    //freopen("log.txt","w",stdout);
    engine<std_rules> e;
    // int depth = 1;
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