#pragma once
#include "../include/board.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"
#include "../include/tt.h"

static constexpr std::array<float, 7> PIECE_VALUES = {0, 10, 50, 30, 30, 100000, 90};
static constexpr float MATE_SCORE = PIECE_VALUES[static_cast<int>(PIECE::KING)];
static constexpr float MATE_TT_THRESHOLD = MATE_SCORE - 1000.0f;

//Note: ply = distance from root node.

// Mate scores produced by the search are relative to the root ply. Store them
// relative to the current position so a transposition remains valid when the
// same position is reached at a different ply.
constexpr float score_to_tt(float score, int ply) {
    if (score >= MATE_TT_THRESHOLD)
        return score + ply;
    if (score <= -MATE_TT_THRESHOLD)
        return score - ply;
    return score;
}

constexpr float score_from_tt(float score, int ply) {
    if (score >= MATE_TT_THRESHOLD)
        return score - ply;
    if (score <= -MATE_TT_THRESHOLD)
        return score + ply;
    return score;
}

// Piece-square tables are indexed a1..h1, a2..h2, ... a8..h8.
// Keeping one source row per rank makes each table read like a chessboard.
static constexpr std::array<float, 64> knight_st = {
    -5, -4,   -3,   -3,   -3,   -3, -4, -5, // rank 1
    -4, -2,    0,  0.5,  0.5,    0, -2, -4, // rank 2
    -3, 0.5,   1,  1.5,  1.5,    1, 0.5, -3, // rank 3
    -3,  0,   1.5,   2,    2,  1.5,  0, -3, // rank 4
    -3, 0.5,  1.5,    2,    2,  1.5, 0.5, -3, // rank 5
    -3,  0,     1,  1.5,  1.5,    1,  0, -3, // rank 6
    -4, -2,     0,    0,    0,    2, -2, -4, // rank 7
    -5, -4,    -3,   -3,   -3,   -3, -4, -5, // rank 8
};

static constexpr std::array<float, 64> bishop_st = {
    -2,  -1, -1, -1, -1, -1,  -1, -2, // rank 1
    -1, 0.5,  0,  0,  0,  0, 0.5, -1, // rank 2
    -1,   0,  1,  1,  1,  1,   0, -1, // rank 3
    -1,   0,  1,  2,  2,  1,   0, -1, // rank 4
    -1,   0,  1,  2,  2,  1,   0, -1, // rank 5
    -1,   1,  1,  2,  2,  1,   1, -1, // rank 6
    -1,   0,  0,  0,  0,  0,   0, -1, // rank 7
    -2,  -1, -1, -1, -1, -1,  -1, -2, // rank 8
};

static constexpr std::array<float, 64> rook_st = {
       0, 0, 0, 0.5, 0.5, 0, 0,    0, // rank 1
    -0.5, 0, 0,   0,   0, 0, 0, -0.5, // rank 2
    -0.5, 0, 0,   0,   0, 0, 0, -0.5, // rank 3
    -0.5, 0, 0,   0,   0, 0, 0, -0.5, // rank 4
    -0.5, 0, 0,   0,   0, 0, 0, -0.5, // rank 5
    -0.5, 0, 0,   0,   0, 0, 0, -0.5, // rank 6
     0.5, 1, 1,   1,   1, 1, 1,  0.5, // rank 7
       0, 0, 0,   0,   0, 0, 0,    0, // rank 8
};

static constexpr std::array<float, 64> queen_st = {
      -2,  -1,  -1, -0.5, -0.5,  -1,  -1,   -2, // rank 1
      -1,   0,   0,    0,    0,   0,   0,   -1, // rank 2
      -1,   0, 0.5,  0.5,  0.5, 0.5,   0,   -1, // rank 3
    -0.5,   0, 0.5,  0.5,  0.5, 0.5,   0, -0.5, // rank 4
       0,   0, 0.5,  0.5,  0.5, 0.5,   0, -0.5, // rank 5
      -1, 0.5, 0.5,  0.5,  0.5, 0.5,   0,   -1, // rank 6
      -1,   0,   0,    0,    0,   0,   0,   -1, // rank 7
      -2,  -1,  -1, -0.5, -0.5,  -1,  -1,   -2, // rank 8
};

static constexpr std::array<float, 64> pawn_st = {
      0,    0,  0,   0,   0,  0,    0,   0, // rank 1
    0.5,    1,  1,  -2,  -2,  1,    1, 0.5, // rank 2
    0.5, -0.5, -1,   0,   0, -1, -0.5, 0.5, // rank 3
      0,    0,  0,   2,   2,  0,    0,   0, // rank 4
    0.5,  0.5,  1, 2.5, 2.5,  1,  0.5, 0.5, // rank 5
      1,    1,  2,   3,   3,  2,    1,   1, // rank 6
      5,    5,  5,   5,   5,  5,    5,   5, // rank 7
      0,    0,  0,   0,   0,  0,    0,   0, // rank 8
};

static constexpr std::array<float, 64> king_mg_st = {
     2,  3,  1,  0,  0,  1,  3,  2, // rank 1
     2,  2,  0,  0,  0,  0,  2,  2, // rank 2
    -1, -2, -2, -2, -2, -2, -2, -1, // rank 3
    -2, -3, -3, -4, -4, -3, -3, -2, // rank 4
    -3, -4, -4, -5, -5, -4, -4, -3, // rank 5
    -4, -5, -5, -6, -6, -5, -5, -4, // rank 6
    -5, -6, -6, -7, -7, -6, -6, -5, // rank 7
    -6, -7, -7, -8, -8, -7, -7, -6, // rank 8
};

static constexpr std::array<std::array<float, 64>, 6> pst = {
    pawn_st, rook_st, knight_st, bishop_st, king_mg_st, queen_st,
};

/*
MINI-MAX SEARCH : Suppose currently, we are processing white's possible moves. Then among all the subtrees of
computation (ie. depth-1 where we look at black's possible moves) emerging from those possibilities, we will choose that
move leading to the highest (most +ve) eval. Similarly if we are currently processing black's possible moves, we will
choose that move leading to the least (most -ve) eval.

ALPHA BETA PRUNING : If we are processing white's possible moves, then alpha = highest eval we are getting from a move
evaluated so far. Similarly, if we are processing black's possible moves, then beta = lowest eval we are getting.
Therefore, the white player only updates alpha and the black player only updates beta, but both are passed down to
depth-1 as arguments.

Suppose we have processed moves w1, w2 .. wi-1 of white, and are now evaluating wi by checking black's
responses to it, ie. called minimax(depth-1, alpha, beta). If, say, after evaluating move bj, we get beta <= alpha
then clearly the parent (white) will never wish to play wi, because if they do, black will play bj and lead to a
position with eval = beta, when in fact they can play the best move among {w1,w2...wi-1} to get an eval = alpha.
Hence, we 'prune' the branch evaluating wi and move on to wi+1, without having to evaluate bj+1, bj+2.. etc.
Similarly, if while processing white moves, we get beta <= alpha, then we can conclude that the parent (black) will
never play into branch of computation. Hence, for given any position we can say that alpha <= eval(pos) <= beta and at
each node we try to tighten the window, ie. maximise alpha and minimise beta.

NODE CLASSIFICATION :-
PV (principal variation) NODE : alpha < eval(node) < beta, it's value is the exact eval of the position.
CUT NODE (fail high) : beta <= eval(node), we prune this node from the search. To gauge eval(node), atleast one child
of the node must be searched.
ALL NODE (fail low) : eval(node) <= alpha, all children of this node should be evaluated to arrive at this conclusion.

TT (transposition table) : a cache of already processed nodes.
The eval of a PV node is stored with the 'exact' flag, cut node is stored with the 'lower bound' flag and all node is
stored with the 'upper bound' flag.
Justification : Suppose we are currently evaluating white's moves with a given [alpha, beta]. Suppose, after evaluating
some of those moves, ie. seeing black's responses to the positions that arise, we get that eval(pos) >= beta (CUT NODE).
Clearly, we can conclude this position is 'too good' (how? because the black parent already has a better move in mind
leading to eval = beta) for white, hence the parent (black) would never play into it. So, without looking further, the
current (white) node would be pruned --> true eval of the position >= eval returned by minimax. Hence, we store it as a
lower bound - the next time this position shows up (in cache), we can update alpha!

Consider the same scenario, but this time suppose that after seeing all of white's responses, the evals of all the black
positions are <= alpha, ie. eval(pos) <= alpha. However, eval(pos) may NOT be the true eval of the position, because one
of its children nodes could have performed pruning. We can only safely say that the true eval of the position <= eval
returned by minimax, hence stored as upper bound - the next time this position shows up (in cache), we can update beta!

Consider the same scenario, but suppose that after looking at all of white's responses, the window becomes [alpha',
beta], ie. beta > eval(pos) > alpha. Claim : this is the true eval of the position, ie. no child was pruned.

*/

/*
board pass by reference - slower
cached best move given preference in move ordering - slower
*/

template <typename rulebook> class engine {
  public:
    rulebook rules;
    tt table;
    u32 nodes_seen = 0;
    u32 tt_hits = 0;

    void order_moves(const board &b, std::vector<move> &moves, const move& first_to_try);
    std::pair<move, float> minimax(const board &b, const COLOUR &turn, int depth, float alpha, float beta, int ply);
    std::pair<move, float> minimax_tt(const board &b, const COLOUR &turn, int depth, float alpha, float beta, int ply);
    std::pair<move, float> minimax_tt_wrap(const board& b, const COLOUR& turn, int depth);
    float base_case_minimax(const board &b, const COLOUR &turn, int depth, float alpha, float beta, int ply = 0);
    float base_eval(const board &b);
    int get_depth_extension(const board &b, const COLOUR &turn);
};

template <typename rulebook> void engine<rulebook>::order_moves(const board &b, std::vector<move> &moves, const move& first_to_try) {
    if (moves.empty())
        return;

    // priority ordering: promotions > checks and captures > others
    auto score_move = [&](const move &mv) {
        float score = 1.0f;
        if (mv.promotion_piece() != PIECE::EMPTY) {
            score += 10 * PIECE_VALUES[static_cast<int>(mv.promotion_piece())];
        }
        if (mv.piece_captured()) {
            auto [end_piece_colour, end_piece] = b.get_piece(mv.end_location());
            score += 10 * PIECE_VALUES[static_cast<int>(end_piece)] - PIECE_VALUES[static_cast<int>(mv.piece())];
        }
        if (first_to_try == mv)
            score += 50;
            
        return score;
    };

    std::sort(moves.begin(), moves.end(),
              [&](const move &left, const move &right) { return score_move(left) > score_move(right); });
}

// Callable wrapper for minimax_tt
template <typename rulebook>
std::pair<move, float> engine<rulebook>::minimax_tt_wrap(const board& b, const COLOUR&  turn, int depth){
    return minimax_tt(b, turn, depth, -INF, INF, 0);
}

// minimax with alpha beta pruning upto specified depth along with caching (transposition table)
// returns current evaluation of the position, and the best move.
template <typename rulebook>
std::pair<move, float> engine<rulebook>::minimax_tt(const board &b, const COLOUR &turn, int depth, float alpha,
                                                    float beta, int ply) {
    if (turn == COLOUR::NONE)
        return {move(), 0.0f};

    nodes_seen++;
    float alpha_orig = alpha;
    float beta_orig = beta;

    const tt_entry *entry = table.probe(b.zhash);
    if (entry != nullptr && entry->depth >= depth) {
        tt_hits++;
        const float tt_eval = score_from_tt(entry->eval, ply);
        switch (entry->node_type) {
        case TT_FLAG::EXACT:
            return std::pair{entry->best_move, tt_eval};
            break;
        case TT_FLAG::LOWER_BOUND:
            alpha = std::max(alpha, tt_eval);
            break;
        case TT_FLAG::UPPER_BOUND:
            beta = std::min(beta, tt_eval);
            break;
        default:
            break;
        }

        if (alpha >= beta) {
            return {entry->best_move, tt_eval};
        }
    }

    move first_move_to_try;
    if (entry != nullptr && entry->depth >= depth) {
        first_move_to_try = entry->best_move;
    }

    std::vector<move> all_moves = rules.get_all_legal_moves(b, turn);

    // if no moves
    if (all_moves.empty()) {
        move_generator checker(b, turn);

        // checkmate
        if (checker.is_in_check()) {
            float mate_eval = (turn == COLOUR::WHITE) ? -PIECE_VALUES[static_cast<int>(PIECE::KING)] + ply
                                                      : PIECE_VALUES[static_cast<int>(PIECE::KING)] - ply;
            return {move(), mate_eval};
        }
        // stalemate
        else {
            return {move(), 0.0f};
        }
    }

    std::pair<move, float> best_move = (turn == COLOUR::WHITE) ? std::pair{move(), -INF} : std::pair{move(), INF};
    order_moves(b, all_moves, first_move_to_try);

    for (const move &mv : all_moves) {
        board next_board = b;
        COLOUR next_turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
        next_board.zhash ^= zobrist::rv_colour;
        next_board.turn = next_turn;

        // simulate current move
        if (mv.castle_kside()) {
            rules.castle_kside(next_board, turn);
        } else if (mv.castle_qside()) {
            rules.castle_qside(next_board, turn);
        } else {
            next_board.apply_move(mv);
        }
        rules.update_flags(next_board, mv);
        // cout << "depth : " << depth << " ";
        // mv.print_move();
        // b.print_board();

        float cur_evaluation = 0.0f;
        int depth_extension = get_depth_extension(next_board, next_turn);
        if (depth + depth_extension == 1)
            cur_evaluation = base_case_minimax(next_board, next_turn, 0, alpha, beta, ply+1);
        else {
            auto temp = minimax_tt(next_board, next_turn, depth - 1 + depth_extension, alpha, beta, ply+1);
            cur_evaluation = temp.second;
        }

        if (turn == COLOUR::WHITE && cur_evaluation > best_move.second) {
            best_move = {mv, cur_evaluation};
            alpha = std::max(alpha, cur_evaluation);
        } else if (turn == COLOUR::BLACK && cur_evaluation < best_move.second) {
            best_move = {mv, cur_evaluation};
            beta = std::min(beta, cur_evaluation);
        }
        if (beta <= alpha)
            break;
    }

    TT_FLAG flag = TT_FLAG::EXACT;
    if (best_move.second >= beta_orig) {
        flag = TT_FLAG::LOWER_BOUND;
    } else if (best_move.second <= alpha_orig) {
        flag = TT_FLAG::UPPER_BOUND;
    }
    table.insert(b.zhash, depth, score_to_tt(best_move.second, ply), best_move.first, flag);
    return best_move;
}

// minimax with alpha beta pruning upto specified depth
// returns current evaluation of the position, and the best move.
template <typename rulebook>
std::pair<move, float> engine<rulebook>::minimax(const board &b, const COLOUR &turn, int depth, float alpha,
                                                 float beta, int ply) {
    if (turn == COLOUR::NONE)
        return {move(), 0.0f};

    std::vector<move> all_moves = rules.get_all_legal_moves(b, turn);

    // if no moves
    if (all_moves.empty()) {
        move_generator checker(b, turn);

        // checkmate
        if (checker.is_in_check()) {
            float mate_eval = (turn == COLOUR::WHITE) ? -PIECE_VALUES[static_cast<int>(PIECE::KING)] + ply
                                                      : PIECE_VALUES[static_cast<int>(PIECE::KING)] - ply;
            return {move(), mate_eval};
        }
        // stalemate
        else {
            return {move(), 0.0f};
        }
    }

    std::pair<move, float> best_move = (turn == COLOUR::WHITE) ? std::pair{move(), -INF} : std::pair{move(), INF};
    order_moves(b, all_moves);

    for (const move &mv : all_moves) {
        board next_board = b;
        COLOUR next_turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
        next_board.zhash ^= zobrist::rv_colour;
        next_board.turn = next_turn;

        // simulate current move
        if (mv.castle_kside()) {
            rules.castle_kside(next_board, turn);
        } else if (mv.castle_qside()) {
            rules.castle_qside(next_board, turn);
        } else {
            next_board.apply_move(mv);
        }
        rules.update_flags(next_board, mv);
        // cout << "depth : " << depth << " ";
        // mv.print_move();

        float cur_evaluation = 0.0f;
        if (depth == 1)
            cur_evaluation = base_case_minimax(next_board, next_turn, 0, alpha, beta, ply+1);
        else {
            auto temp = minimax(next_board, next_turn, depth - 1, alpha, beta, ply+1);
            cur_evaluation = temp.second;
        }

        if (turn == COLOUR::WHITE && cur_evaluation > best_move.second) {
            best_move = {mv, cur_evaluation};
            alpha = std::max(alpha, cur_evaluation);
        } else if (turn == COLOUR::BLACK && cur_evaluation < best_move.second) {
            best_move = {mv, cur_evaluation};
            beta = std::min(beta, cur_evaluation);
        }
        if (beta <= alpha)
            break;
    }
    return best_move;
}

//Quiescent search
template <typename rulebook>
float engine<rulebook>::base_case_minimax(const board &b, const COLOUR &turn, int depth, float alpha, float beta, int ply) {
    if (turn == COLOUR::NONE)
        return 0;
    move_generator checker(b, turn);
    const bool in_check = checker.is_in_check();
    std::vector<move> all_moves = in_check ? rules.get_all_legal_moves(b, turn)
                                           : rules.get_all_legal_tactical_moves(b, turn);

    if (all_moves.empty()) {
        if (in_check) {
            return (turn == COLOUR::WHITE) ? -PIECE_VALUES[static_cast<int>(PIECE::KING)] + ply
                                           : PIECE_VALUES[static_cast<int>(PIECE::KING)] - ply;
        }
        if (!rules.has_any_legal_move(b, turn))
            return 0.0f;
        return base_eval(b);
    }

    // Standing pat is legal only outside check. In check, every legal evasion must be searched.
    float best_eval = (turn == COLOUR::WHITE) ? -INF : INF;
    if (!in_check) {
        best_eval = base_eval(b);
        if (turn == COLOUR::WHITE) {
            if (best_eval >= beta)
                return best_eval;
            alpha = std::max(alpha, best_eval);
        } else {
            if (best_eval <= alpha)
                return best_eval;
            beta = std::min(beta, best_eval);
        }
    }

    order_moves(b, all_moves, move());

    for (const move &mv : all_moves) {
        board next_board = b;
        COLOUR next_turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
        next_board.zhash ^= zobrist::rv_colour;
        next_board.turn = next_turn;

        if (mv.castle_kside()) {
            rules.castle_kside(next_board, turn);
        } else if (mv.castle_qside()) {
            rules.castle_qside(next_board, turn);
        } else {
            next_board.apply_move(mv);
        }
        rules.update_flags(next_board, mv);

        // cout << "depth : " << depth << " ";
        // mv.print_move();
        // b.print_board();

        float cur_evaluation = base_case_minimax(next_board, next_turn, depth - 1, alpha, beta, ply+1);
        if (turn == COLOUR::WHITE && cur_evaluation > best_eval) {
            best_eval = cur_evaluation;
            alpha = std::max(alpha, cur_evaluation);
        } else if (turn == COLOUR::BLACK && cur_evaluation < best_eval) {
            best_eval = cur_evaluation;
            beta = std::min(beta, cur_evaluation);
        }
        if (beta <= alpha)
            break;
    }
    return best_eval;
}

template <typename rulebook> float engine<rulebook>::base_eval(const board &b) {
    float eval = 0;
    for (int i = 0; i < 6; i++) {
        u64 cur = b.pieces[i];
        while (cur) {
            int pos = lsb(cur);
            eval += pst[i][pos] + PIECE_VALUES[i + 1];
            cur &= cur - 1;
        }
    }

    for (int i = 6; i < 12; i++) {
        u64 cur = b.pieces[i];
        while (cur) {
            int pos = lsb(cur) ^ 56;
            eval -= pst[i - 6][pos] + PIECE_VALUES[i - 5];
            cur &= cur - 1;
        }
    }
    // cout << "final eval : " << eval << endl;
    // b.print_board();
    return eval;
}

template <typename rulebook> int engine<rulebook>::get_depth_extension(const board &b, const COLOUR &turn) {
    if (turn == COLOUR::NONE)
        return 0;
    move_generator helper(b, turn);
    if (helper.is_in_check())
        return 1;
    return 0;
}
