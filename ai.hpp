#pragma once
#include "Board/board.hpp"
#include "MainLogic/movegen.hpp"
#include "Board/transposition_hashtable.hpp"
#include <cassert>
#include <unordered_map>
namespace chess
{
    class AI
    {
    private:
        Board &brd;
        Movelist_ref &global_movelist_ref;

        transposition_moves_table better_hashtable;

        template <Color color>
        int64_t perft(int d, Movelist_ref list_ref)
        {
            if (d == 0)
                return 1;

            Movegen generator(brd, list_ref);

            generator.gen_all_moves<color>();

            int nodes = 0;
            for (Move_full_info *i = list_ref.begin; i != list_ref.end; ++i)
            {
                const Accumulator acc = brd.make_move<color>(*i);

                nodes += perft<change_color(color)>(d - 1, list_ref.get_ref());

                brd.undo_move<color>(*i, acc);
            }
            return nodes;
        }

        template <Color color>
        int64_t hashtest(int d, Movelist_ref list_ref, u64 hash)
        {
            assert(hash == brd.get_hash());
            if (d == 0)
                return 1;

            Movegen generator(brd, list_ref);

            generator.gen_all_moves<color>();

            int nodes = 0;
            for (Move_full_info *i = list_ref.begin; i != list_ref.end; ++i)
            {
                u64 new_hash = brd.get_hash<color>(hash, *i) ^ brd.get_specials_hash();
                const Accumulator acc = brd.make_move<color>(*i);
                new_hash ^= brd.get_specials_hash();

                nodes += hashtest<change_color(color)>(d - 1, list_ref.get_ref(), new_hash);

                brd.undo_move<color>(*i, acc);
            }
            return nodes;
        }

       

        template <Color clr>
        int q_search_ab_checked(int alpha, int beta, Movelist_ref list_ref, const int last_move_id)
        {
            if (list_ref.no_moves())
            {
                constexpr int mate = -3'000'000;
                return mate;
            }
            int eval = brd.eval<clr>();

            for (Move_full_info *i = list_ref.begin; i != list_ref.end; ++i)
            {

                const Accumulator acc = brd.make_move<clr>(*i);

                eval = -q_search_ab<change_color(clr)>(-beta, -alpha, list_ref.get_ref(), i->to_square);

                brd.undo_move<clr>(*i, acc);

                if (eval >= beta)
                {
                    return beta;
                }

                if (eval > alpha)
                {
                    alpha = eval;
                }
            }
            return alpha;
        }

        template <Color clr>
        int q_search_quite(int alpha, int beta, Movelist_ref list_ref, const int last_move_id)
        {
            if (list_ref.no_moves())
            {
                constexpr int stalemate = 0;
                return stalemate;
            }
            for (Move_full_info *i = list_ref.begin; i != list_ref.end; ++i)
            {
                if ((brd[i->to_square] == No_Piece) && (i->special != SP_en_passant) && (i->special != SP_Promotion))
                    continue;

                const Accumulator acc = brd.make_move<clr>(*i);

                int eval = -q_search_ab<change_color(clr)>(-beta, -alpha, list_ref.get_ref(), i->to_square);

                brd.undo_move<clr>(*i, acc);

                if (eval >= beta)
                {
                    return beta;
                }

                if (eval > alpha)
                {
                    alpha = eval;
                }
            }
            return alpha;
        }
        template <Color clr>
        int q_search_ab(int alpha, int beta, Movelist_ref list_ref, const int last_move_id)
        {
            ++all_nodes;
            Movegen generator(brd, list_ref);
            PositionState state;
            int attack_sq;
            Direction attack_dir;
            const int king_id = brd.get_right_king_position<clr>();
            std::tie(state, attack_sq, attack_dir) = generator.get_attackers_information<clr>(king_id);

            if (state != quite)
            {
                generator.gen_all_moves<clr>(king_id, state, attack_sq, attack_dir);
                brd.sort_moves<clr>(list_ref, last_move_id);
                return q_search_ab_checked<clr>(alpha, beta, list_ref, last_move_id);
            }

            int eval = brd.eval<clr>();
            if (eval >= beta)
                return beta;
            if (alpha < eval)
                alpha = eval;

            generator.gen_all_moves<clr>(king_id, state, attack_sq, attack_dir);
            brd.sort_moves<clr>(list_ref, last_move_id);
            return q_search_quite<clr>(alpha, beta, list_ref, last_move_id);
        }

        inline int futility_limit(int d)
        {
            return (d * d - d) * 100 + 300;
        }
        template <Color clr>
        int negamax_ab_with_order_and_hash(int d, int alpha, int beta, Movelist_ref list_ref, const int last_move_id, u64 hash, Hashlist_ref hashlist_ref)
        {
            if ((hashlist_ref.contains_count(hash) >= 2) || (brd.get_fifty_rule() >= 100))
                return 0;
            hashlist_ref.add(hash);
            Movegen generator(brd, list_ref);
            PositionState state;
            int attack_sq;
            Direction attack_dir;
            const int king_id = brd.get_right_king_position<clr>();
            std::tie(state, attack_sq, attack_dir) = generator.get_attackers_information<clr>(king_id);
            if (state != quite)
                ++d;

            if (d == 0)
            {
                return q_search_ab<clr>(alpha, beta, list_ref, last_move_id);
            }

            generator.gen_all_moves<clr>(king_id, state, attack_sq, attack_dir);

            if (list_ref.no_moves())
            {
                constexpr int mate = -3'000'000;
                ++all_nodes;
                if (state >= check)
                    return mate - d;

                constexpr int stalemate = 0;
                return stalemate - d;
            }
            
            if ((state == quite) && ((futility_limit(d) + brd.eval<clr>()) <= alpha))
            {
                return q_search_ab<clr>(alpha, beta, list_ref, last_move_id);
            }

            transposition_moves_table::Node &best_hash_node = better_hashtable[hash];
            if (!best_hash_node.is_empty())
                brd.sort_moves_with_hashed_move<clr>(list_ref, last_move_id, Move_full_info(best_hash_node.move));
            else
                brd.sort_moves<clr>(list_ref, last_move_id);

            Move_full_info real_best_move = *list_ref.begin;

            for (Move_full_info *i = list_ref.begin; i != list_ref.end; ++i)
            {
                u64 new_hash = brd.get_hash<clr>(hash, *i);
                Hashlist_ref new_hashlist;
                if ((brd[i->from_square] == Pawn_with_color<clr>()) || (brd[i->to_square] != No_Piece))
                    new_hashlist = hashlist_ref.get_ref_from_end();
                else
                    new_hashlist = hashlist_ref;

                const Accumulator acc = brd.make_move<clr>(*i);

                const int loc_eval = -negamax_ab_with_order_and_hash<change_color(clr)>(d - 1, -beta, -alpha, list_ref.get_ref(), i->to_square, new_hash, new_hashlist);

                brd.undo_move<clr>(*i, acc);

                if (loc_eval >= beta)
                {
                    if (d > best_hash_node.depth)
                        best_hash_node = {i->to_move(), static_cast<u8>(d)};
                    return beta;
                }

                if (loc_eval > alpha)
                {
                    real_best_move = *i;
                    alpha = loc_eval;
                }
            }
            if (d > best_hash_node.depth)
                best_hash_node = {real_best_move.to_move(), static_cast<u8>(d)};
            return alpha;
        }
        template <Color clr>
        std::tuple<Move_full_info, int> best_move_ab_with_order_and_hash(int d, const int last_move_id, u64 hash, Hashlist_ref hashlist_ref)
        {
            all_nodes = 0;

            Movegen generator(brd, global_movelist_ref);

            PositionState state = generator.gen_all_moves<clr>();

            if ((d == 0) || (global_movelist_ref.no_moves()))
                return {Move_full_info{}, 0};

            transposition_moves_table::Node &best_hash_node = better_hashtable[hash];
            if (!best_hash_node.is_empty())
                brd.sort_moves_with_hashed_move<clr>(global_movelist_ref, last_move_id, Move_full_info(best_hash_node.move));
            else
                brd.sort_moves<clr>(global_movelist_ref, last_move_id);

            // brd.sort_moves<clr>(global_movelist_ref, last_move_id);
            Move_full_info best_move;
            constexpr int inf = 1'000'000'000;

            int alpha = -inf;
            // int beta = inf;
            hashlist_ref.add(brd.get_hash());

            for (Move_full_info *i = global_movelist_ref.begin; i != global_movelist_ref.end; ++i)
            {
                u64 new_hash = brd.get_hash<clr>(hash, *i);
                Hashlist_ref new_hashlist;
                if ((brd[i->from_square] == Pawn_with_color<clr>()) || (brd[i->to_square] != No_Piece))
                    new_hashlist = hashlist_ref.get_ref_from_end();
                else
                    new_hashlist = hashlist_ref;
                const Accumulator acc = brd.make_move<clr>(*i);
                int new_d = d - (state == quite);
                int loc_eval = -negamax_ab_with_order_and_hash<change_color(clr)>(new_d, -inf, -alpha, global_movelist_ref.get_ref(), i->to_square, new_hash, new_hashlist);
                // std::cout << *i << " " << loc_eval << '\n';
                brd.undo_move<clr>(*i, acc);

                if (loc_eval > alpha)
                {
                    best_move = *i;
                    alpha = loc_eval;
                }
            }
            if (d > best_hash_node.depth)
                best_hash_node = {best_move.to_move(), static_cast<u8>(d)};
            return {best_move, alpha};
        }

        std::tuple<Move_full_info, int> best_move_ab_with_order_and_hash(const int d, Hashlist_ref hashlist_ref, const int last_move_id = No_Square)
        {
            return (brd.get_turn() ? best_move_ab_with_order_and_hash<White>(d, last_move_id, brd.get_hash(), hashlist_ref) : best_move_ab_with_order_and_hash<Black>(d, last_move_id, brd.get_hash(), hashlist_ref));
        }

        

    public:

      
        uint64_t all_nodes;
        AI(Board &board, Movelist_ref &movelist_ref) : brd(board), global_movelist_ref(movelist_ref), better_hashtable() {}

        void hash_aging(int add_age)
        {
            better_hashtable.do_aging(add_age);
        }

        std::tuple<Move_full_info, int> best_move_nodes_limit(uint64_t nodes_limit, Hashlist_ref hashlist_ref, const int last_move_id = No_Square)
        {
            all_nodes = 0;
            std::tuple<Move_full_info, int> return_info;
            for (int i = 1; (all_nodes < nodes_limit) && (i < 100); ++i)
            {
                all_nodes = 0;
                return_info = best_move_ab_with_order_and_hash(i, hashlist_ref, last_move_id);
            }
            return return_info;
        }

        void clear_hash()
        {
            better_hashtable.clear();
        }
    };
}
