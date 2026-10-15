#pragma once
#include "../types.hpp"
#include <cassert>
namespace chess
{
    class transposition_moves_table
    {
    private:
        inline constexpr static int size = 16777259;

    public:
        struct Node
        {
        public:
        
            Move move;
            u8 depth;
            inline Node(Move _move = 0, u8 d = 0) : move(_move), depth(d) {}
            inline bool is_empty() { return depth == 0; }
        };
        Node *table;

        transposition_moves_table()
        {
            table = new Node[size];
        }
        void clear()
        {
            std::fill(table, table + size, Node());
        }
        inline Node& operator[](const u64 hash)
        {
            return table[hash % size];
        }
        void do_aging(int add_age)
        {
            for (auto *i = table, *end = table + size; i != end; ++i)
            {
                i->depth = std::max(0, i->depth - add_age);
            }
        }
        ~transposition_moves_table()
        {
            delete[] table;
            table = nullptr;
        }
    };
}