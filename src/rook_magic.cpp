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
rook_movement_masks2 = rook_movement_masks with edge cells not included
blockers(pos) = rook_movement_masks2[pos] & all_pieces();
GOAL = map : blockers --> rook_legal_moves;
let r = pop_ct(rook_movement_masks2[pos]) <-- no. of bits required to represent all possible blocker configs
let idx = ( blockers(pos) * magic[pos] ) >> (64 - r) <-- one way hash function
rook_legal_moves(pos, board) = rook_moves[pos][idx];

constructing 64 magic numbers

for(all 64 positions):-
    let pos = current pos
    let r = pop_ct(rook_movement_masks2[pos])
    for(n times):-
        let magic = random sparse key
        let table : empty
        for(all possible blockers configs):-
            let blockers = current blockers
            let idx = (blockers * magic) >> (64 - r)
            let rook_legal_moves = get_rook_legal_moves_mask(pos, blockers)
            if(!table[idx] or table[idx] == rook_legal_moves) table[idx] = rook_legal_moves
            else break // hash collision

        if (!collision) print to log (pos, magic) and the table
        else continue

*/

static std::mt19937 rng(0xC0FFEE);

inline u64 gen_rand_key() { return (u64(rng()) << 32) | u64(rng()); }

inline u64 gen_rand_sparse_key() { return gen_rand_key() & gen_rand_key() & gen_rand_key(); }

u64 get_rook_mvmt_mask(int r, int c) {
    u64 ans = 0ULL;
    for (int i = 0; i < 8; i++) {
        ans |= (1ULL << (8 * r + i));
        ans |= (1ULL << (c + 8 * i));
    }
    return ans;
}

inline constexpr u64 rook_movement_masks[64] = {
    72340172838076926ULL,    144680345676153597ULL,   289360691352306939ULL,   578721382704613623ULL,
    1157442765409226991ULL,  2314885530818453727ULL,  4629771061636907199ULL,  9259542123273814143ULL,
    72340172838141441ULL,    144680345676217602ULL,   289360691352369924ULL,   578721382704674568ULL,
    1157442765409283856ULL,  2314885530818502432ULL,  4629771061636939584ULL,  9259542123273813888ULL,
    72340172854657281ULL,    144680345692602882ULL,   289360691368494084ULL,   578721382720276488ULL,
    1157442765423841296ULL,  2314885530830970912ULL,  4629771061645230144ULL,  9259542123273748608ULL,
    72340177082712321ULL,    144680349887234562ULL,   289360695496279044ULL,   578721386714368008ULL,
    1157442769150545936ULL,  2314885534022901792ULL,  4629771063767613504ULL,  9259542123257036928ULL,
    72341259464802561ULL,    144681423712944642ULL,   289361752209228804ULL,   578722409201797128ULL,
    1157443723186933776ULL,  2314886351157207072ULL,  4629771607097753664ULL,  9259542118978846848ULL,
    72618349279904001ULL,    144956323094725122ULL,   289632270724367364ULL,   578984165983651848ULL,
    1157687956502220816ULL,  2315095537539358752ULL,  4629910699613634624ULL,  9259541023762186368ULL,
    143553341945872641ULL,   215330564830528002ULL,   358885010599838724ULL,   645993902138460168ULL,
    1220211685215703056ULL,  2368647251370188832ULL,  4665518383679160384ULL,  9259260648297103488ULL,
    18302911464433844481ULL, 18231136449196065282ULL, 18087586418720506884ULL, 17800486357769390088ULL,
    17226286235867156496ULL, 16077885992062689312ULL, 13781085504453754944ULL, 9187484529235886208ULL};

inline constexpr u64 rook_movement_masks_no_edges[64] = {
    282578800148862ULL,     565157600297596ULL,     1130315200595066ULL,    2260630401190006ULL,
    4521260802379886ULL,    9042521604759646ULL,    18085043209519166ULL,   36170086419038334ULL,
    282578800180736ULL,     565157600328704ULL,     1130315200625152ULL,    2260630401218048ULL,
    4521260802403840ULL,    9042521604775424ULL,    18085043209518592ULL,   36170086419037696ULL,
    282578808340736ULL,     565157608292864ULL,     1130315208328192ULL,    2260630408398848ULL,
    4521260808540160ULL,    9042521608822784ULL,    18085043209388032ULL,   36170086418907136ULL,
    282580897300736ULL,     565159647117824ULL,     1130317180306432ULL,    2260632246683648ULL,
    4521262379438080ULL,    9042522644946944ULL,    18085043175964672ULL,   36170086385483776ULL,
    283115671060736ULL,     565681586307584ULL,     1130822006735872ULL,    2261102847592448ULL,
    4521664529305600ULL,    9042787892731904ULL,    18085034619584512ULL,   36170077829103616ULL,
    420017753620736ULL,     699298018886144ULL,     1260057572672512ULL,    2381576680245248ULL,
    4624614895390720ULL,    9110691325681664ULL,    18082844186263552ULL,   36167887395782656ULL,
    35466950888980736ULL,   34905104758997504ULL,   34344362452452352ULL,   33222877839362048ULL,
    30979908613181440ULL,   26493970160820224ULL,   17522093256097792ULL,   35607136465616896ULL,
    9079539427579068672ULL, 8935706818303361536ULL, 8792156787827803136ULL, 8505056726876686336ULL,
    7930856604974452736ULL, 6782456361169985536ULL, 4485655873561051136ULL, 9115426935197958144ULL};

std::vector<u64> possible_blockers_configs(const int &pos) {
    std::vector<u64> result;
    u64 num_blockers = 1ULL << pop_ct(rook_movement_masks_no_edges[pos]);
    for (u64 blockers = 0; blockers < num_blockers; blockers++) {
        u64 temp = blockers;
        u64 blockers_mask = 0ULL;
        u64 movement_mask = rook_movement_masks_no_edges[pos];
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

u64 legal_rook_moves_mask(const int &pos, const u64 &blockers) {
    int r = pos / 8;
    int c = pos % 8;
    u64 piece_location = 1ULL << pos;
    u64 result = 0ULL;
    for (int i = 1; i < 8; i++) {
        if (c + i > 7)
            break;
        u64 temp_location = piece_location << i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (c - i < 0)
            break;
        u64 temp_location = piece_location >> i;
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (r + i > 7)
            break;
        u64 temp_location = piece_location << (8 * i);
        if (temp_location & blockers) {
            result |= temp_location;
            break;
        }
        result |= temp_location;
    }
    for (int i = 1; i < 8; i++) {
        if (r - i < 0)
            break;
        u64 temp_location = piece_location >> (8 * i);
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
    freopen("rook.txt", "w", stdout);
    
    for (int pos = 0; pos < 64; pos++) {
        int r = pop_ct(rook_movement_masks_no_edges[pos]);
        auto configs = possible_blockers_configs(pos);
        for (int iter = 0; iter < num_iters; iter++) {
            u64 magic_key = gen_rand_sparse_key();
            std::vector<u64> table((1ULL << r), 0ULL);
            bool collision = false;
            for (const auto &blockers : configs) {
                int idx = (u64(magic_key * blockers) >> (64 - r));
                u64 legal_moves_mask = legal_rook_moves_mask(pos, blockers);
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