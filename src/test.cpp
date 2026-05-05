#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/magic.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

#include <chrono>


int main() {

    // freopen("log.txt", "w", stdout);
    game<std_rules> g1;
    g1.make_move(g1.parse_move("e2e4"));
    g1.show_board();
    int depth = 7;
    engine<std_rules> e;
    auto start = std::chrono::high_resolution_clock::now();
    auto [m, em] = e.minimax_tt(g1.chess_board, g1.turn, depth, -INF, INF);
    auto end = std::chrono::high_resolution_clock::now();
    cout << std::chrono::duration<double>(end - start).count() << "\n";
    m.print_move();
    g1.make_move(m);
    g1.show_board();

    return 0;
}
