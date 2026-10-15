#pragma once
#include <random>
#include"../types.hpp"

namespace chess{
    constexpr int seed = 1952515;
    std::mt19937 mt(seed);
    
    // Generates a Random number from 0 to 2^64-1
    u64 randomInt()
    {
        std::uniform_int_distribution<u64> dist(0, UINT64_MAX);
        return dist(mt);
    }

    types::array<types::array<u64, 12>, 64> gen_zobrist(){
        types::array<types::array<u64, 12>, 64> zobrist;

        for(int i = 0; i < 64; ++i){
            for(int j = 0; j < 12; ++j){
                zobrist[i][j] = randomInt();
            }
        }
        return zobrist;
    }


    types::array<u64, 8> gen_en_passant_files_hash(){
        types::array<u64, 8> en_passant_hash;

        for(int i = 0; i < 8; ++i){
            en_passant_hash[i] = randomInt();
        }
        return en_passant_hash;
    }

    types::array<types::array<u64, 12>, 64> zobrist_hashtable{gen_zobrist()};

    u64 black_side_to_move_hash{randomInt()};

    u64 White_OO_hash{randomInt()};

    u64 White_OOO_hash{randomInt()};

    u64 Black_OO_hash{randomInt()};

    u64 Black_OOO_hash{randomInt()};

   


    types::array<u64, 8> en_passant_files_hash {gen_en_passant_files_hash()};
}
