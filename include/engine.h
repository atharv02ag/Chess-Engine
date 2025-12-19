#pragma once
#include "../include/board.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

constexpr std::array<float, 7> PIECE_VALUES = {0, 10, 50, 30, 30, 500, 90};

// piece square tables
constexpr std::array<float, 64> knight_st = {-5, -4,  -3,  -3,  -3,  -3,  -4,  -5, -4, -2, 0,   0.5, 0.5, 0,   -2, -4,
                                             -3, 0.5, 1,   1.5, 1.5, 1,   0.5, -3, -3, 0,  1.5, 2.0, 2.0, 1.5, 0,  -3,
                                             -3, 0.5, 1.5, 2.0, 2.0, 1.5, 0.5, -3, -3, 0,  1,   1.5, 1.5, 1,   0,  -3,
                                             -4, -2,  0,   0,   0,   2,   -2,  -4, -5, -4, -3,  -3,  -3,  -3,  -4, -5};

constexpr std::array<float, 64> bishop_st = {-2, -1, -1, -1, -1, -1, -1, -2, -1, 0.5, 0,  0,  0,  0,  0.5, -1,
                                             -1, 0,  1,  1,  1,  1,  0,  -1, -1, 0,   1,  2,  2,  1,  0,   -1,
                                             -1, 0,  1,  2,  2,  1,  0,  -1, -1, 1,   1,  2,  2,  1,  1,   -1,
                                             -1, 0,  0,  0,  0,  0,  0,  -1, -2, -1,  -1, -1, -1, -1, -1,  -2};

constexpr std::array<float, 64> rook_st = {0,    0, 0, 0.5, 0.5, 0, 0, 0,    -0.5, 0, 0, 0, 0, 0, 0, -0.5,
                                           -0.5, 0, 0, 0,   0,   0, 0, -0.5, -0.5, 0, 0, 0, 0, 0, 0, -0.5,
                                           -0.5, 0, 0, 0,   0,   0, 0, -0.5, -0.5, 0, 0, 0, 0, 0, 0, -0.5,
                                           0.5,  1, 1, 1,   1,   1, 1, 0.5,  0,    0, 0, 0, 0, 0, 0, 0};

constexpr std::array<float, 64> queen_st = {
    -2,  -1,  -1,   -0.5, -0.5, -1,  -1,  -2,  -1, 0,    0, 0,  0,   0,   0,   -1,   -1,   0,    0.5, 0.5, 0.5, 0.5,
    0,   -1,  -0.5, 0,    0.5,  0.5, 0.5, 0.5, 0,  -0.5, 0, 0,  0.5, 0.5, 0.5, 0.5,  0,    -0.5, -1,  0.5, 0.5, 0.5,
    0.5, 0.5, 0,    -1,   -1,   0,   0,   0,   0,  0,    0, -1, -2,  -1,  -1,  -0.5, -0.5, -1,   -1,  -2};

constexpr std::array<float, 64> pawn_st = {0,   0, 0, 0,  0,  0, 0, 0,   5,   5,    5,  5,   5,   5,  5,    5,
                                           1,   1, 2, 3,  3,  2, 1, 1,   0.5, 0.5,  1,  2.5, 2.5, 1,  0.5,  0.5,
                                           0,   0, 0, 2,  2,  0, 0, 0,   0.5, -0.5, -1, 0,   0,   -1, -0.5, 0.5,
                                           0.5, 1, 1, -2, -2, 1, 1, 0.5, 0,   0,    0,  0,   0,   0,  0,    0};

constexpr std::array<float, 64> king_mg_st = {2,  3,  1,  0,  0,  1,  3,  2,  2,  2,  0,  0,  0,  0,  2,  2,
                                              -1, -2, -2, -2, -2, -2, -2, -1, -2, -3, -3, -4, -4, -3, -3, -2,
                                              -3, -4, -4, -5, -5, -4, -4, -3, -4, -5, -5, -6, -6, -5, -5, -4,
                                              -5, -6, -6, -7, -7, -6, -6, -5, -6, -7, -7, -8, -8, -7, -7, -6};

constexpr std::array<std::array<float, 64>, 6> pst = {pawn_st, rook_st, knight_st, bishop_st, king_mg_st, queen_st};

template <typename rulebook> class engine {
  public:
    rulebook rules;

    void order_moves(const board &b, std::vector<move> &moves) {
        if (moves.empty())
            return;
        std::vector<float> score(moves.size());
        // std::vector<move> temp;
        COLOUR colour = moves[0].colour;
        move_generator mg(b, colour);

        for (int i = 0; i < moves.size(); i++) {
            if (moves[i].piece_captured) {
                auto [end_piece_colour, end_piece] = b.get_piece(moves[i].end_location);
                // capture piece of higher value
                // if (PIECE_VALUES[static_cast<int>(moves[i].piece)] <= PIECE_VALUES[static_cast<int>(end_piece)]) {
                //     score[i] = 10;
                //     temp.push_back(moves[i]);
                // }
                // // capture piece of lower value
                // else {
                //     temp.push_back(moves[i]);
                // }
                score[i] =
                    10 * PIECE_VALUES[static_cast<int>(end_piece)] - PIECE_VALUES[static_cast<int>(moves[i].piece)];
            }
            // else if(mg.is_attacked_by_pawn(moves[i].end_location,)){

            // }
            else {
                score[i] = 1;
                // temp.push_back(moves[i]);
            }
            // else if()
        }
        std::vector<int> idx(moves.size());
        for (int i = 0; i < moves.size(); i++)
            idx[i] = i;
        std::sort(idx.begin(), idx.end(), [&](const int &i, const int &j) { return score[i] > score[j]; });
        std::vector<move> temp;
        for (int i = 0; i < moves.size(); i++) {
            temp.push_back(moves[idx[i]]);
        }
        for (int i = 0; i < moves.size(); i++) {
            moves[i] = temp[i];
        }
    }

    // minimax with alpha beta pruning upto specified depth
    // returns current evaluation of the position, and the best move.
    std::pair<move, float> minimax(const board &b, const COLOUR &turn, int depth, float alpha, float beta) {
        move empty;
        if (turn == COLOUR::NONE)
            return {empty, 0.0f};

        std::vector<move> all_moves = rules.get_all_legal_moves(b, turn);

        // if no moves
        if (all_moves.empty()) {
            move_generator checker(b, turn);
            if (checker.is_in_check()) {
                float mate_eval = (turn == COLOUR::WHITE) ? -PIECE_VALUES[static_cast<int>(PIECE::KING)]
                                                          : PIECE_VALUES[static_cast<int>(PIECE::KING)];
                return {empty, mate_eval};
            } else {
                return {empty, 0.0f};
            }
        }

        std::pair<move, float> best_move = (turn == COLOUR::WHITE) ? std::pair{empty, -INF} : std::pair{empty, INF};

        order_moves(b, all_moves);

        for (const move &mv : all_moves) {
            board next_board = b;
            COLOUR next_turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
            if (mv.castle_kside) {
                rules.castle_kside(next_board, turn);
            } else if (mv.castle_qside) {
                rules.castle_qside(next_board, turn);
            } else {
                next_board.apply_move(mv);
            }
            rules.update_flags(next_board, mv);
            // cout << "depth : " << depth << " ";
            // mv.print_move();

            float cur_evaluation = 0.0f;
            if (depth == 1)
                cur_evaluation = base_case_minimax(next_board, next_turn, 0, alpha, beta);
            else {
                auto temp = minimax(next_board, next_turn, depth - 1, alpha, beta);
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

    float base_case_minimax(const board &b, const COLOUR &turn, int depth, float alpha, float beta) {
        if (turn == COLOUR::NONE)
            return 0;
        std::vector<move> all_moves = rules.get_all_legal_moves(b, turn);

        if (all_moves.empty()) {
            // checkmate
            move_generator checker(b, turn);
            if (checker.is_in_check()) {
                float mate_eval = (turn == COLOUR::WHITE) ? -PIECE_VALUES[static_cast<int>(PIECE::KING)]
                                                          : PIECE_VALUES[static_cast<int>(PIECE::KING)];
                return mate_eval;
            }
            // stalemate
            else {
                return 0.0f;
            }
        }

        float best_eval = (turn == COLOUR::WHITE) ? -INF : INF;
        bool no_capture_moves = true;
        order_moves(b, all_moves);

        for (const move &mv : all_moves) {
            if (!mv.piece_captured)
                continue;
            no_capture_moves = false;
            board next_board = b;
            COLOUR next_turn = (turn == COLOUR::WHITE) ? COLOUR::BLACK : COLOUR::WHITE;
            if (mv.castle_kside) {
                rules.castle_kside(next_board, turn);
            } else if (mv.castle_qside) {
                rules.castle_qside(next_board, turn);
            } else {
                next_board.apply_move(mv);
            }

            // cout << "depth : " << depth << " ";
            // mv.print_move();

            float cur_evaluation = base_case_minimax(next_board, next_turn, depth - 1, alpha, beta);
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
        if (no_capture_moves)
            return base_eval(b, turn);
        else
            return best_eval;
    }

    float base_eval(const board &b, const COLOUR &turn) {
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
                eval += pst[i - 6][pos] + PIECE_VALUES[i - 5];
                cur &= cur - 1;
            }
        }
        // u64 cur = b.white_bishops;
        // while (cur) {
        //     int pos = lsb(cur);
        //     eval += bishop_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::BISHOP)];
        //     cur &= cur - 1;
        // }
        // cur = b.black_bishops;
        // while (cur) {
        //     int pos = lsb(cur) ^ 56;
        //     eval -= bishop_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::BISHOP)];
        //     cur &= cur - 1;
        // }

        // cur = b.white_knights;
        // while (cur) {
        //     int pos = lsb(cur);
        //     eval += knight_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::KNIGHT)];
        //     cur &= cur - 1;
        // }
        // cur = b.black_knights;
        // while (cur) {
        //     int pos = lsb(cur) ^ 56;
        //     eval -= knight_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::KNIGHT)];
        //     cur &= cur - 1;
        // }

        // cur = b.white_rooks;
        // while (cur) {
        //     int pos = lsb(cur);
        //     eval += rook_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::ROOK)];
        //     cur &= cur - 1;
        // }
        // cur = b.black_rooks;
        // while (cur) {
        //     int pos = lsb(cur) ^ 56;
        //     eval -= rook_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::ROOK)];
        //     cur &= cur - 1;
        // }

        // cur = b.white_queens;
        // while (cur) {
        //     int pos = lsb(cur);
        //     eval += queen_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::QUEEN)];
        //     cur &= cur - 1;
        // }
        // cur = b.black_queens;
        // while (cur) {
        //     int pos = lsb(cur) ^ 56;
        //     eval -= queen_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::QUEEN)];
        //     cur &= cur - 1;
        // }

        // cur = b.white_pawns;
        // while (cur) {
        //     int pos = lsb(cur);
        //     eval += pawn_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::PAWN)];
        //     cur &= cur - 1;
        // }
        // cur = b.black_pawns;
        // while (cur) {
        //     int pos = lsb(cur) ^ 56;
        //     eval -= pawn_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::PAWN)];
        //     cur &= cur - 1;
        // }

        // int pos = lsb(b.white_king);
        // eval += king_mg_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::KING)];

        // pos = lsb(b.black_king) ^ 56;
        // eval -= king_mg_st[pos] + PIECE_VALUES[static_cast<int>(PIECE::KING)];

        // cout << "final eval : " << eval << endl;
        // b.print_board();
        return eval;
    }
};