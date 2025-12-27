#pragma once
#include "globals.h"

class zobrist{
public:
    //for each board state variable, a random number is associated.
    static std::array<std::array<u64,64>,12> rv_pieces;

    // mapped to castle rights
    static std::array<u64,4> rv_flags;

    //enpass mapping : rv_enpass[0:7] - a6,b6..h6, rv_enpass[8:15] - a3,b3..h3
    static std::array<u64,16> rv_enpass;

    //part of hash if turn = white
    static u64 rv_colour;

    static void init();
};