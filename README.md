# C++ Chess Engine

A compact chess engine written in C++. It includes a playable command-line game, complete legal move generation, fixed-depth minimax search, and correctness/performance tests.

## Board representation and move generation

The board uses [bitboards](https://www.chessprogramming.org/Bitboards): one 64-bit integer for each piece/colour pair, for twelve bitboards in total. Bit `0` represents `a1`, bit `63` represents `h8`, and set operations efficiently answer questions such as "which squares are occupied?" or "which enemy pieces can be captured?" Moves are packed into 32 bits, including their source, destination, piece, colour, and special flags.

The move generator supports checks, castling, en passant, captures, and all promotion choices. A pseudo-legal move is accepted only if applying it does not leave its own king attacked.

## Magic bitboards

Rooks and bishops are sliding pieces: their attacks continue along a rank, file, or diagonal until the first blocker. A straightforward generator walks outward one square at a time in every direction, adding each square and stopping when it meets a piece. Although simple, that work is repeated for every rook, bishop, and queen at a very large number of search nodes. It also contains blocker-dependent loops and branches.

This engine instead uses [magic bitboards](https://www.chessprogramming.org/Magic_Bitboards), a form of perfect hashing for sliding-piece attacks. The expensive work is performed in advance and the result is stored in attack tables. During search, generating all attacks for one sliding piece requires only a mask, multiplication, shift, and array lookup.

### Relevant blockers and the lookup

For each square, `bishop_movement_masks_no_edges` contains every square on the bishop's diagonals except the outer edge squares. The rook has an equivalent `rook_movement_masks_no_edges` table. Edge squares can be omitted from the blocker hash because an edge square is always the final square on that ray: whether it is empty or occupied does not change any attack squares beyond it.

For a bishop on `pos`:

```text
relevant_mask = bishop_movement_masks_no_edges[pos]
blockers      = relevant_mask & all_pieces
r             = popcount(relevant_mask)
index         = (blockers * bishop_magic_keys[pos]) >> (64 - r)
legal_moves   = bishop_table[pos][index]
```

There are `2^r` possible configurations of the `r` relevant blocker bits. Multiplication by a carefully selected sparse 64-bit magic number mixes those scattered bits; retaining the upper `r` bits produces a compact integer suitable for indexing the precomputed table. Rook lookup works identically with the rook mask, magic key, shift, and table. A queen combines the corresponding bishop and rook attack bitboards.

### Finding the magic numbers

The magic numbers and attack tables were generated offline. For each of the 64 squares, the generator enumerates every possible subset of the relevant blocker mask and computes the correct attacks with a reference ray generator. It then repeatedly tries random sparse keys until one maps every blocker configuration safely:

```text
for each square pos:
    r = popcount(movement_mask_without_edges[pos])

    repeat:
        magic = random_sparse_64_bit_number()
        clear attack_table
        collision = false

        for each possible blocker subset:
            index   = (blockers * magic) >> (64 - r)
            attacks = reference_sliding_attacks(pos, blockers)

            if attack_table[index] is empty:
                attack_table[index] = attacks
            else if attack_table[index] != attacks:
                collision = true
                break

        if no collision occurred:
            save pos, magic, shift, and attack_table
            break
```

Two blocker configurations may share an index only when they produce the same attack bitboard; this is a *constructive collision* and does not change the result. A collision between different attack bitboards rejects that candidate magic. Consequently, the accepted mapping behaves as perfect hashing for the information the move generator needs.

Compared with manually walking every ray at runtime, the lookup has fixed work and very few branches. It trades some memory and one-time generation effort for much faster, more predictable move generation throughout the search. The precomputed tables are embedded in `magic.h`.

The original generator can be inspected on the [`magic` branch](https://github.com/atharv02ag/Chess-Engine/tree/magic), particularly commit [`d9a8811`](https://github.com/atharv02ag/Chess-Engine/commit/d9a8811).

## Evaluation

Static evaluation is positive for White and negative for Black. It combines material values with [piece-square tables](https://www.chessprogramming.org/Piece-Square_Tables) (PSTs): each piece receives a small bonus or penalty according to its square. This encodes basic positional preferences such as knight centralisation, pawn advancement, and king shelter. Black's square index is vertically mirrored so both colours share the same tables.

The current PSTs are fixed and intentionally simple; they do not yet distinguish middlegame from endgame or evaluate pawn structure, mobility, passed pawns, bishop pairs, or king attacks.

## Minimax and alpha-beta pruning

[Minimax](https://www.chessprogramming.org/Minimax) models both sides as choosing their best continuation: White selects the highest evaluation, while Black selects the lowest. Searching every continuation grows exponentially, so the engine uses [alpha-beta pruning](https://www.chessprogramming.org/Alpha-Beta).

- `alpha` is the best score White can already guarantee along the current path.
- `beta` is the best score Black can already guarantee.
- Therefore the interesting window is `alpha <= score <= beta`.

Suppose White has already found move `w1` with score `alpha`, and the engine is now examining `w2`. If one Black reply to `w2` produces a score at or below `alpha`, White will never choose `w2`; Black already has a sufficient refutation. The remaining Black replies cannot change the parent's decision and are pruned. The same reasoning applies in reverse at a Black node. Thus, when `beta <= alpha`, search stops exploring that branch without changing the result that full minimax would return. Good [move ordering](https://www.chessprogramming.org/Move_Ordering) makes this much more effective, so promotions and favourable captures are searched first.

At the nominal depth limit, [quiescence search](https://www.chessprogramming.org/Quiescence_Search) continues through captures and promotions to reduce the error caused by evaluating in the middle of an unresolved exchange. The static evaluation is also considered as the **stand-pat** score, meaning the side may make no tactical move if its current position is already better. Standing pat is illegal while in check, so every legal check evasion is searched instead. The main search also extends checked positions by one ply.

## Transposition table and node types

Different move orders can reach the same position, called a transposition. The engine maintains an incremental [Zobrist hash](https://www.chessprogramming.org/Zobrist_Hashing) and caches search results (like best move, position eval, etc.) in a small sized [transposition table](https://www.chessprogramming.org/Transposition_Table). Each bucket contains four compact entries; stale entries are replaced first, followed by shallow entries.

As described by [alpha-beta node types](https://www.chessprogramming.org/Node_Types), a stored score is not always exact:

- **PV node / exact:** `alpha < score < beta`. No cutoff determines the result, so the stored score is the position's exact searched value.
- **Cut node / lower bound (fail high):** `score >= beta`. The position is already too good for the minimizing parent to choose it. Search may stop after one refuting child, so the true value is only known to be at least the returned score.
- **All node / upper bound (fail low):** `score <= alpha`. All moves failed to improve alpha. Because descendants may themselves have cut off, the true value is only known to be at most the returned score.

On a later lookup, an exact value can be returned immediately. A lower bound raises `alpha`, an upper bound lowers `beta`, and either may close the window and cause a cutoff. Stored best moves also provide useful move-ordering hints.

## Performance

The engine can solve basic tactical puzzles and identify checkmating sequences. It can search to approximately depth 7 at usable speeds, typically around 10-30 seconds per move, although the exact time depends on the position and hardware. More systematic benchmarking and an estimated Elo rating are still future work, but it is able to beat upto stockfish 6, hence its rating should be around **1900**

## Build and run

Requires Python 3.9+ and a C++20-capable `g++` or `clang++`. The helper script builds with `-O3`, native CPU optimisations, and link-time optimisation; executables are written to `build/`.

```sh
# Play against the engine
python scripts/run_target.py src/main.cpp

# Tactical search/performance tests
python scripts/run_target.py tests/test.cpp

# Perft and regression tests
python scripts/run_target.py tests/perft_tests.cpp
```

Select a compiler with `--compiler clang++`, or add `--compile-only` to build without running.

## Possible improvements

- [Iterative deepening](https://www.chessprogramming.org/Iterative_Deepening), time management, and aspiration windows.
- [Killer moves](https://www.chessprogramming.org/Killer_Heuristic), history scoring, and stronger capture ordering such as static exchange evaluation.
- Principal variation search, null-move pruning, late-move reductions, and selective pruning.
- Tapered/dynamic PSTs, mobility, pawn structure, king safety, and endgame-specific evaluation.
