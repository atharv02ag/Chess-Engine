#pragma once
#include "globals.h"

class prng{
    private:
        std::mt19937* generator;
    public:
        prng(){
            std::random_device os_seed;
            const u32 seed = os_seed();
            generator = new std::mt19937(seed);
        }
        u64 gen_num(u64 l, u64 r){
            std::uniform_int_distribution<u64> distribute(l,r);
            return distribute(*generator);
        }
};