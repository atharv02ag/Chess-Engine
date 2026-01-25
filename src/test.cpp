#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/magic.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"


int main() {

    // freopen("log.txt", "w", stdout);
    game<std_rules> g1;
    g1.make_move(g1.parse_move("e2e4"));
    g1.show_board();
    int depth = 6;
    engine<std_rules> e;
    auto [m, em] = e.minimax_tt(g1.chess_board, g1.turn, depth, -INF, INF);
    m.print_move();
    g1.make_move(m);
    g1.show_board();

    // game<std_rules> g("rnbqkbnr/ppp1pppp/8/8/4p3/8/PPPPKPPP/RNBQ1BNR w kq - 0 1");
    // move_generator mg(g.chess_board, g.turn);
    // auto all = mg.get_all_generic_moves();
    // for(const auto mv : all){
    //     mv.print_move();
    // }
    // g.make_move(g.parse_move("e2f3"));
    // mg.update(g.chess_board);
    // g.show_board();
    // if(mg.is_attacked_by_pawn(g.chess_board.pieces[get_piece_idx(COLOUR::WHITE,PIECE::KING)])){
    //     cout << "ATTACKED" << endl;
    // }
    // else{
    //     cout << "NOT ATTACKED" << endl;
    // }

    return 0;
}
