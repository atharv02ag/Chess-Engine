#include "../include/tt.h"

tt_entry::tt_entry(const u64 &zk, const int &d, const float &ev, const move &mv, const int &gen, const TT_FLAG& type)
    : zkey(zk), depth(d), eval(ev), best_move(mv), generation(gen), node_type(type) {};

tt_entry tt::probe(u64 hash) {
    if (!hash)
        return tt_entry();
    return container[hash % TT_SIZE];
}

tt::tt(){
    container = std::vector<tt_entry>(TT_SIZE, tt_entry());
}

void tt::insert(tt_entry node) {
    if (node.generation != tt_generation) {
        cout << "tt::insert : outdated node trying to be inserted" << endl;
        return;
    }
    container[node.zkey % TT_SIZE] = node;
}

void tt::clear() { tt_generation++; }