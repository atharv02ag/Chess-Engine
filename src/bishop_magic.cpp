#include "../include/board.h"
#include "../include/engine.h"
#include "../include/game.h"
#include "../include/globals.h"
#include "../include/move.h"
#include "../include/move_generator.h"
#include "../include/rules.h"

#include <chrono>
#include <random>

/*
bishop_movement_masks2 = bishop_movement_masks with edge cells not included
blockers(pos) = bishop_movement_masks2[pos] & all_pieces();
GOAL = map : blockers --> rook_legal_moves;
let r = pop_ct(bishop_movement_masks2[pos]) <-- no. of bits required to represent all possible blocker configs
let idx = ( blockers(pos) * magic[pos] ) >> (64 - r) <-- one way hash function
bishop_legal_moves(pos, board) = bishop_moves[pos][idx];

constructing 64 magic numbers

for(all 64 positions):-
    let pos = current pos
    let r = pop_ct(bishop_movement_masks2[pos])
    for(n times):-
        let magic = random sparse key
        let table : empty
        for(all possible blockers configs):-
            let blockers = current blockers
            let idx = (blockers * magic) >> (64 - r)
            let bishop_legal_moves = get_bishop_legal_moves_mask(pos, blockers)
            if(!table[idx] or table[idx] == bishop_legal_moves) table[idx] = bishop_legal_moves
            else break // hash collision

        if (!collision) print to log (pos, magic) and the table
        else continue

*/

static std::mt19937 rng(0xC0FFEE);

inline u64 gen_rand_key() { return (u64(rng()) << 32) | u64(rng()); }

inline u64 gen_rand_sparse_key() { return gen_rand_key() & gen_rand_key() & gen_rand_key(); }

u64 get_bishop_mvmt_mask(int r, int c) {
    u64 ans = 0ULL;
    for (int i = 0; i < 8; i++) {
        int j = r + c - i;
        if (j >= 0 && j < 8)
            ans |= (1ULL << (8 * i + j));
    }
    for (int i = 0; i < 8; i++) {
        int j = c - r + i;
        if (j < 8 && j >= 0)
            ans |= (1ULL << (8 * i + j));
    }
    ans &= ~(1ULL << (8 * r + c));
    return ans;
}

inline constexpr u64 bishop_movement_masks[64] = {
    9241421688590303744ULL, 36099303471056128ULL,   141012904249856ULL,      550848566272ULL,
    6480472064ULL,          1108177604608ULL,       283691315142656ULL,      72624976668147712ULL,
    4620710844295151618ULL, 9241421688590368773ULL, 36099303487963146ULL,    141017232965652ULL,
    1659000848424ULL,       283693466779728ULL,     72624976676520096ULL,    145249953336262720ULL,
    2310355422147510788ULL, 4620710844311799048ULL, 9241421692918565393ULL,  36100411639206946ULL,
    424704217196612ULL,     72625527495610504ULL,   145249955479592976ULL,   290499906664153120ULL,
    1155177711057110024ULL, 2310355426409252880ULL, 4620711952330133792ULL,  9241705379636978241ULL,
    108724279602332802ULL,  145390965166737412ULL,  290500455356698632ULL,   580999811184992272ULL,
    577588851267340304ULL,  1155178802063085600ULL, 2310639079102947392ULL,  4693335752243822976ULL,
    9386671504487645697ULL, 326598935265674242ULL,  581140276476643332ULL,   1161999073681608712ULL,
    288793334762704928ULL,  577868148797087808ULL,  1227793891648880768ULL,  2455587783297826816ULL,
    4911175566595588352ULL, 9822351133174399489ULL, 1197958188344280066ULL,  2323857683139004420ULL,
    144117404414255168ULL,  360293502378066048ULL,  720587009051099136ULL,   1441174018118909952ULL,
    2882348036221108224ULL, 5764696068147249408ULL, 11529391036782871041ULL, 4611756524879479810ULL,
    567382630219904ULL,     1416240237150208ULL,    2833579985862656ULL,     5667164249915392ULL,
    11334324221640704ULL,   22667548931719168ULL,   45053622886727936ULL,    18049651735527937ULL};

inline constexpr u64 bishop_movement_masks_no_edges[64] = {
    18049651735527936ULL, 70506452091904ULL,    275415828992ULL,      1075975168ULL,        38021120ULL,
    8657588224ULL,        2216338399232ULL,     567382630219776ULL,   9024825867763712ULL,  18049651735527424ULL,
    70506452221952ULL,    275449643008ULL,      9733406720ULL,        2216342585344ULL,     567382630203392ULL,
    1134765260406784ULL,  4512412933816832ULL,  9024825867633664ULL,  18049651768822272ULL, 70515108615168ULL,
    2491752130560ULL,     567383701868544ULL,   1134765256220672ULL,  2269530512441344ULL,  2256206450263040ULL,
    4512412900526080ULL,  9024834391117824ULL,  18051867805491712ULL, 637888545440768ULL,   1135039602493440ULL,
    2269529440784384ULL,  4539058881568768ULL,  1128098963916800ULL,  2256197927833600ULL,  4514594912477184ULL,
    9592139778506752ULL,  19184279556981248ULL, 2339762086609920ULL,  4538784537380864ULL,  9077569074761728ULL,
    562958610993152ULL,   1125917221986304ULL,  2814792987328512ULL,  5629586008178688ULL,  11259172008099840ULL,
    22518341868716544ULL, 9007336962655232ULL,  18014673925310464ULL, 2216338399232ULL,     4432676798464ULL,
    11064376819712ULL,    22137335185408ULL,    44272556441600ULL,    87995357200384ULL,    35253226045952ULL,
    70506452091904ULL,    567382630219776ULL,   1134765260406784ULL,  2832480465846272ULL,  5667157807464448ULL,
    11333774449049600ULL, 22526811443298304ULL, 9024825867763712ULL,  18049651735527936ULL};

std::vector<u64> possible_blockers_configs(const int &pos) {
    std::vector<u64> result;
    u64 num_blockers = 1ULL << pop_ct(bishop_movement_masks_no_edges[pos]);
    for (u64 blockers = 0; blockers < num_blockers; blockers++) {
        u64 temp = blockers;
        u64 blockers_mask = 0ULL;
        u64 movement_mask = bishop_movement_masks_no_edges[pos];
        while (movement_mask) {
            int location = lsb(movement_mask);
            blockers_mask |= ((temp & 1) << location);
            temp = (temp >> 1);
            movement_mask &= movement_mask - 1;
        }
        result.push_back(blockers_mask);
    }
    return result;
}

u64 legal_bishop_moves_mask(const int &pos, const u64 &blockers) {
    int r = pos / 8;
    int c = pos % 8;
    u64 piece_location = 1ULL << pos;
    u64 result = 0ULL;
    for (int i = 1; i < 8; i++) {
        if (c + i > 7 || r + i > 7)
            break;
        u64 temp_location = (piece_location << (8 * i)) << i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (c - i < 0 || r - i < 0)
            break;
        u64 temp_location = (piece_location >> (8 * i)) >> i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (r + i > 7 || c - i < 0)
            break;
        u64 temp_location = (piece_location << (8 * i)) >> i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (r - i < 0 || c + i > 7)
            break;
        u64 temp_location = (piece_location >> (8 * i)) << i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    return result;
}

constexpr int num_iters = 1e7;

int main() {
    freopen("bishop.txt", "w", stdout);

    for (int pos = 0; pos < 64; pos++) {
        int r = pop_ct(bishop_movement_masks_no_edges[pos]);
        auto configs = possible_blockers_configs(pos);
        for (int iter = 0; iter < num_iters; iter++) {
            u64 magic_key = gen_rand_sparse_key();
            std::vector<u64> table((1ULL << r), 0ULL);
            bool collision = false;
            for (const auto &blockers : configs) {
                int idx = (u64(magic_key * blockers) >> (64 - r));
                u64 legal_moves_mask = legal_bishop_moves_mask(pos, blockers);
                if (!table[idx] || table[idx] == legal_moves_mask)
                    table[idx] = legal_moves_mask;
                else {
                    collision = true;
                    break;
                }
            }
            if (!collision) {
                cout << pos << " " << magic_key << endl;
                cout << table.size() << endl;
                for (const auto &mask : table) {
                    cout << mask << " ";
                }
                cout << endl;
                break;
            }
        }
    }

    return 0;
}