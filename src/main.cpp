#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

int main() {
    game<std_rules> g("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
    g.show_board();
    //g.make_move(move(PIECE::KNIGHT, COLOUR::WHITE, (1ULL << (8 * 2) << 5), (1ULL << (8 * 4) << 6)));
    //g.show_board();
    auto all = g.rules.get_all_legal_moves(g.chess_board, g.turn);
    for(auto &mv : all){
        mv.print_move();
        g.make_move(mv);
        g.show_board();
    }   

    // freopen("log.txt","w",stdout);
    //  engine<std_rules> e;
    //  int depth = 1;
    //  for (int i = 0; i < 10; i++) {
    //      cout << "iter : " << i << endl;
    //      g.show_board();
    //      auto [w, ew] = e.minimax(g.chess_board, COLOUR::WHITE, depth, -INF, INF);
    //      if (w.piece == PIECE::EMPTY) {
    //          break;
    //      }
    //      w.print_move();
    //      cout << "eval : " << ew << endl;
    //      g.make_move(w);
    //      auto [b, eb] = e.minimax(g.chess_board, COLOUR::BLACK, depth, -INF, INF);
    //      if (b.piece == PIECE::EMPTY) {
    //          break;
    //      }
    //      b.print_move();
    //      cout << "eval : " << eb << endl;
    //      g.make_move(b);
    //  }

    return 0;
}