#include "../include/engine.h"
#include "../include/game.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <optional>
#include <string>

namespace {

std::string trim(std::string text) {
    const auto first = std::find_if_not(text.begin(), text.end(),
                                        [](unsigned char ch) { return std::isspace(ch); });
    const auto last = std::find_if_not(text.rbegin(), text.rend(),
                                       [](unsigned char ch) { return std::isspace(ch); }).base();
    return first < last ? std::string(first, last) : std::string{};
}

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return text;
}

std::optional<std::string> normalise_move(const std::string &input, COLOUR turn) {
    std::string text = lower(trim(input));
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char ch) {
                   return std::isspace(ch) || ch == '-' || ch == 'x';
               }),
               text.end());

    if (text == "oo" || text == "00")
        return turn == COLOUR::WHITE ? "K" : "k";
    if (text == "ooo" || text == "000")
        return turn == COLOUR::WHITE ? "Q" : "q";
    if (text.size() != 4 && text.size() != 5)
        return std::nullopt;

    const auto is_file = [](char ch) { return ch >= 'a' && ch <= 'h'; };
    const auto is_rank = [](char ch) { return ch >= '1' && ch <= '8'; };
    if (!is_file(text[0]) || !is_rank(text[1]) || !is_file(text[2]) || !is_rank(text[3]))
        return std::nullopt;
    if (text.size() == 5 && std::string("qrbn").find(text[4]) == std::string::npos)
        return std::nullopt;
    return text;
}

std::string move_text(const move &mv) {
    if (mv.castle_kside())
        return "O-O";
    if (mv.castle_qside())
        return "O-O-O";
    if (!mv.has_coordinates())
        return "(none)";

    const int start = mv.start_square();
    const int end = mv.end_square();
    std::string result;
    result += static_cast<char>('a' + start % 8);
    result += static_cast<char>('1' + start / 8);
    result += static_cast<char>('a' + end % 8);
    result += static_cast<char>('1' + end / 8);
    switch (mv.promotion_piece()) {
    case PIECE::QUEEN: result += 'q'; break;
    case PIECE::ROOK: result += 'r'; break;
    case PIECE::BISHOP: result += 'b'; break;
    case PIECE::KNIGHT: result += 'n'; break;
    default: break;
    }
    return result;
}

const char *side_name(COLOUR side) { return side == COLOUR::WHITE ? "White" : "Black"; }

void print_help() {
    std::cout << "Enter moves as e2e4 (also accepts E2-E4 or e2 e4).\n"
                 "Promotions need a suffix, for example e7e8q. Castling is O-O or O-O-O.\n"
                 "Commands: moves, board, help, quit\n";
}

COLOUR choose_side() {
    while (true) {
        std::cout << "Choose your side [white/black] (default: white): ";
        std::string input;
        if (!std::getline(std::cin, input))
            return COLOUR::NONE;
        input = lower(trim(input));
        if (input.empty() || input == "white" || input == "w")
            return COLOUR::WHITE;
        if (input == "black" || input == "b")
            return COLOUR::BLACK;
        std::cout << "Please enter 'white' or 'black'.\n";
    }
}

int choose_depth() {
    while (true) {
        std::cout << "Search depth [1-10] (default: 6): ";
        std::string input;
        if (!std::getline(std::cin, input))
            return 6;
        input = trim(input);
        if (input.empty())
            return 6;
        try {
            std::size_t used = 0;
            const int depth = std::stoi(input, &used);
            if (used == input.size() && depth >= 1 && depth <= 10)
                return depth;
        } catch (const std::exception &) {
        }
        std::cout << "Please enter a whole number from 1 to 10.\n";
    }
}

bool announce_game_over(const board &position, COLOUR turn, const std::vector<move> &legal_moves) {
    if (!legal_moves.empty())
        return false;
    move_generator state(position, turn);
    if (state.is_in_check())
        std::cout << "Checkmate! " << side_name(turn == COLOUR::WHITE ? COLOUR::BLACK : COLOUR::WHITE)
                  << " wins.\n";
    else
        std::cout << "Stalemate. The game is a draw.\n";
    return true;
}

} // namespace

int main() {
    std::cout << "=== Command-line Chess ===\n";
    const COLOUR human_side = choose_side();
    if (human_side == COLOUR::NONE) {
        std::cout << "Input closed. Goodbye.\n";
        return 0;
    }
    const int depth = choose_depth();
    const COLOUR engine_side = human_side == COLOUR::WHITE ? COLOUR::BLACK : COLOUR::WHITE;

    game<std_rules> game_state;
    engine<std_rules> chess_engine;
    std::cout << "You are " << side_name(human_side) << ". Engine depth: " << depth << ".\n";
    print_help();
    game_state.chess_board.print_board(human_side);

    while (true) {
        const std::vector<move> legal_moves =
            game_state.rules.get_all_legal_moves(game_state.chess_board, game_state.turn);
        if (announce_game_over(game_state.chess_board, game_state.turn, legal_moves))
            break;

        move_generator state(game_state.chess_board, game_state.turn);
        if (state.is_in_check())
            std::cout << side_name(game_state.turn) << " is in check.\n";

        if (game_state.turn == engine_side) {
            std::cout << "Engine is thinking..." << std::flush;
            chess_engine.nodes_seen = 0;
            chess_engine.tt_hits = 0;
            const auto started = std::chrono::steady_clock::now();
            const auto [best_move, evaluation] =
                chess_engine.minimax_tt(game_state.chess_board, game_state.turn, depth, -INF, INF);
            const double seconds =
                std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
            std::cout << "\rEngine: " << move_text(best_move) << "  eval: " << std::fixed
                      << std::setprecision(2) << evaluation << "  time: " << seconds << "s  nodes: "
                      << chess_engine.nodes_seen << "  TT hits: " << chess_engine.tt_hits;
            if (seconds > 0.0)
                std::cout << "  nps: " << static_cast<unsigned long long>(chess_engine.nodes_seen / seconds);
            std::cout << "\n";
            if (!game_state.make_move(best_move)) {
                std::cerr << "Engine produced an invalid move; stopping the game.\n";
                return 1;
            }
            game_state.chess_board.print_board(human_side);
            continue;
        }

        std::cout << "Your move (" << side_name(human_side) << "): ";
        std::string input;
        if (!std::getline(std::cin, input)) {
            std::cout << "\nInput closed. Goodbye.\n";
            break;
        }
        const std::string command = lower(trim(input));
        if (command == "quit" || command == "exit" || command == "q") {
            std::cout << "Goodbye.\n";
            break;
        }
        if (command == "help" || command == "?") {
            print_help();
            continue;
        }
        if (command == "board") {
            game_state.chess_board.print_board(human_side);
            continue;
        }
        if (command == "moves") {
            std::cout << "Legal moves (" << legal_moves.size() << "): ";
            for (std::size_t i = 0; i < legal_moves.size(); ++i)
                std::cout << (i ? ", " : "") << move_text(legal_moves[i]);
            std::cout << "\n";
            continue;
        }

        const auto normalised = normalise_move(input, game_state.turn);
        if (!normalised) {
            std::cout << "Invalid format. Use e2e4, e7e8q for promotion, or O-O/O-O-O for castling.\n";
            continue;
        }
        const move chosen_move = game_state.parse_move(*normalised);
        if (lower(trim(input)) != lower(*normalised))
            std::cout << "Interpreting that as " << move_text(chosen_move) << ".\n";
        if (!game_state.make_move(chosen_move)) {
            std::cout << "That move is not legal in this position. Type 'moves' to list legal moves.\n";
            continue;
        }
        std::cout << "You played " << move_text(chosen_move) << ".\n";
        game_state.chess_board.print_board(human_side);
    }
    return 0;
}
