#pragma once
#include "../types.hpp"
#include "evaltables.hpp"
#include "hashing.hpp"
namespace chess
{
    class Board
    {
    private:
        types::array<Piece, 64> table;
        Color turn = White;
        Square white_king_position, black_king_position;
        CastlingRights castl_rights = White_OO_OOO | Black_OO_OOO;

        Square en_passant = No_Square;

        int fifty_moves_rule = 0;
        int total_moves = 0;

        template <Color color>
        inline void do_en_passant_remove(const int id)
        {
            if constexpr (color)
                table[id - 8] = No_Piece;
            else
                table[id + 8] = No_Piece;
        }


        template <Color color>
        inline void do_rook_castling(const int id)
        {
            if (id == short_castling_king_to_square<color>())
            {
                table[short_castling_rook_from_square<color>()] = No_Piece;
                table[short_castling_rook_to_square<color>()] = Rook_with_color<color>();
            }
            else
            {
                table[long_castling_rook_from_square<color>()] = No_Piece;
                table[long_castling_rook_to_square<color>()] = Rook_with_color<color>();
            }
        }

        template <Color color>
        inline void undo_rook_castling(const int id)
        {
            if (id == short_castling_king_to_square<color>())
            {
                table[short_castling_rook_from_square<color>()] = Rook_with_color<color>();
                table[short_castling_rook_to_square<color>()] = No_Piece;
            }
            else
            {
                table[long_castling_rook_from_square<color>()] = Rook_with_color<color>();
                table[long_castling_rook_to_square<color>()] = No_Piece;
            }
        }

        template <Color color>
        inline void undo_en_passant_remove(const int id)
        {
            if constexpr (color)
                table[id - 8] = Pawn_with_color<change_color(color)>();
            else
                table[id + 8] = Pawn_with_color<change_color(color)>();
        }

       

        inline int eval_kings_positions(int my_king_id, int enemy_king_id)
        {
            const int my_rank = my_king_id / 8, my_file = my_king_id % 8, en_rank = enemy_king_id / 8, en_file = enemy_king_id % 8;
            int min_d = std::abs(my_rank - en_rank);
            int max_d = std::abs(my_file - en_file);
            if (min_d > max_d)
            {
                int buf = min_d;
                min_d = max_d;
                max_d = buf;
            }
            int enemy_d_to_centr_file = std::max(3 - en_file, en_file - 4);
            int enemy_d_to_centr_rank = std::max(3 - en_rank, en_rank - 4);
            return (enemy_d_to_centr_file + enemy_d_to_centr_rank) * 25 - min_d * 7 - max_d * 13;
        }

        template <Color clr>
        inline int eval_position()
        {
            evaluation eval;
            int midlegame_phase = 0;
            for (int i = 0; i < 64; ++i)
            {
                eval += materials_tables[table[i]][i];

                midlegame_phase += phase_table[table[i]];
            }

            eval.eg_eval += eval_kings_positions(get_right_king_position<clr>(), get_right_king_position<change_color(clr)>());

            if (midlegame_phase > 24)
                midlegame_phase = 24;

            return (eval.tapered_eval(midlegame_phase) * (100 - fifty_moves_rule)) / 100;
        }

        template <Color color>
        inline void do_rook_hashing(const int id, u64 &hash)
        {
            if (id == short_castling_king_to_square<color>())
            {
                hash ^= zobrist_hashtable[short_castling_rook_from_square<color>()][Rook_with_color<color>()] ^
                        zobrist_hashtable[short_castling_rook_to_square<color>()][Rook_with_color<color>()];
            }
            else
            {
                hash ^= zobrist_hashtable[long_castling_rook_from_square<color>()][Rook_with_color<color>()] ^
                        zobrist_hashtable[long_castling_rook_to_square<color>()][Rook_with_color<color>()];
            }
        }

        template <Color color>
        inline void do_en_passant_hashing(const int id, u64 &hash)
        {
            if constexpr (color)
                hash ^= zobrist_hashtable[id - 8][Pawn_with_color<change_color(color)>()];
            else
                hash ^= zobrist_hashtable[id + 8][Pawn_with_color<change_color(color)>()];
        }

        void insertionsort(std::pair<int, Move_full_info> *l, std::pair<int, Move_full_info> *r)
        {
            for (auto *i = l + 1; i < r; ++i)
            {
                auto *j = i;
                auto buf = *i;
                while (j > l && (j - 1)->first > buf.first)
                {
                    *j = *(j - 1);
                    --j;
                }
                *j = buf;
            }
        }

        template <Color clr>
        inline int eval_quite_move(Move_full_info move, int mg_phase, int eg_phase)
        {
            if constexpr (clr)
            {
                const int from_eval = (mg_phase * square_eval_midlegame[table[move.from_square]][move.from_square ^ 56] + eg_phase * square_eval_endgame[table[move.from_square]][move.from_square ^ 56]);
                const int to_eval = (mg_phase * square_eval_midlegame[table[move.from_square]][move.to_square ^ 56] + eg_phase * square_eval_endgame[table[move.from_square]][move.to_square ^ 56]);
                return -(to_eval - from_eval) / 24;
            }
            else
            {
                const int from_eval = (mg_phase * square_eval_midlegame[table[move.from_square] - 6][move.from_square] + eg_phase * square_eval_endgame[table[move.from_square] - 6][move.from_square]);
                const int to_eval = (mg_phase * square_eval_midlegame[table[move.from_square] - 6][move.to_square] + eg_phase * square_eval_endgame[table[move.from_square] - 6][move.to_square]);
                return -(to_eval - from_eval) / 24;
            }
        }

        template <Color clr>
        int eval_move(Move_full_info move, const int last_moved_id, int mg_phase, int eg_phase)
        {
            if (table[move.to_square] == No_Piece)
                switch (move.special)
                {
                case SP_castling:
                    return -80;
                case No_special:
                    return eval_quite_move<clr>(move, mg_phase, eg_phase);
                default:
                    break;
                }
            constexpr static types::array<int, 13> matereal_eval_prey{100, 300, 310, 500, 900, 0, 100, 300, 310, 500, 900, 0, 0};
            constexpr static types::array<int, 12> matereal_eval_lion{-10, -30, -31, -50, -90, 0, -10, -30, -31, -50, -90, 0};

            const int eval = ((move.to_square == last_moved_id) * 175) + ((move.special == SP_en_passant) * 1101) + matereal_eval_lion[table[move.from_square]] + matereal_eval_prey[table[move.to_square]] +
                             ((move.special == SP_Promotion) * (matereal_eval_prey[move.promotion + static_cast<u8>(W_Knight)] + 100));

            return -eval;
        }

    public:
        Board() : turn(White), white_king_position(No_Square), black_king_position(No_Square), castl_rights(NO_Castling), en_passant(No_Square), fifty_moves_rule(0), total_moves(0) {}

        Board(const Board &board) : table(board.table), turn(board.turn), white_king_position(board.white_king_position),
                                    black_king_position(board.black_king_position), castl_rights(board.castl_rights), en_passant(board.en_passant),
                                    fifty_moves_rule(board.fifty_moves_rule), total_moves(board.total_moves) {}

        friend bool operator==(const Board &left, const Board &right)
        {
            return (left.table == right.table) && (left.turn == right.turn) && (left.white_king_position == right.white_king_position) &&
                   (left.black_king_position == right.black_king_position) && (left.castl_rights == right.castl_rights) &&
                   (left.en_passant == right.en_passant) &&
                   (left.fifty_moves_rule == right.fifty_moves_rule) && (left.total_moves == right.total_moves);
        }

        friend bool operator!=(const Board &left, const Board &right)
        {
            return (left.table != right.table) || (left.turn != right.turn) || (left.white_king_position != right.white_king_position) ||
                   (left.black_king_position != right.black_king_position) || (left.castl_rights != right.castl_rights) ||
                   (left.en_passant != right.en_passant) ||
                   (left.fifty_moves_rule != right.fifty_moves_rule) || (left.total_moves != right.total_moves);
        }

        constexpr Board& operator=(const Board& node) = default;

        template <Color color>
        void remove_short_castle()
        {
            if constexpr (color)
                castl_rights &= static_cast<CastlingRights>(~White_OO);
            else
                castl_rights &= static_cast<CastlingRights>(~Black_OO);
        }

        template <Color color>
        void remove_long_castle()
        {
            if constexpr (color)
                castl_rights &= static_cast<CastlingRights>(~White_OOO);
            else
                castl_rights &= static_cast<CastlingRights>(~Black_OOO);
        }

        template <Color color>
        void remove_all_castle()
        {
            if constexpr (color)
                castl_rights &= static_cast<CastlingRights>(~White_OO_OOO);
            else
                castl_rights &= static_cast<CastlingRights>(~Black_OO_OOO);
        }

        template <Color color>
        bool can_castle_long()
        {
            if constexpr (color)
            {
                return castl_rights & White_OOO;
            }
            else
                return castl_rights & Black_OOO;
        }

        template <Color color>
        bool can_castle_short()
        {
            if constexpr (color)
            {
                return castl_rights & White_OO;
            }
            else
                return castl_rights & Black_OO;
        }

        inline Piece &operator[](const uint id)
        {
            return table[id];
        }
        

        inline const Piece &operator[](const uint id) const
        {
#ifdef MAESTRO_DEBUG
            if (id >= 64)
                throw std::out_of_range("Invalid id: " + id);
#endif
            return table[id];
        }
        void set_turn(Color color)
        {
            turn = color;
        }

        void set_castling(const CastlingRights rights)
        {
            castl_rights = rights;
        }

        void add_castling(const CastlingRights rights)
        {
            castl_rights |= rights;
        }

        bool has_castling(const CastlingRights rights) const
        {
            return (castl_rights & rights) == rights;
        }

        inline void set_en_passant(chess::Square sq)
        {
#ifdef MAESTRO_DEBUG
            if (((sq > SQ_H3) && (sq < SQ_A3)) || ((sq > SQ_H6) && (sq < SQ_A6)))
                throw std::invalid_argument("Invalid en passant square: " + sq);
#endif
            en_passant = sq;
        }

        void set_fifty_rule(int move)
        {
#ifdef MAESTRO_DEBUG
            if (move >= 50)
                throw std::invalid_argument("Invalid fifty moves: " + move);
#endif
            fifty_moves_rule = move;
        }

        void set_total_moves(int moves)
        {
#ifdef MAESTRO_DEBUG
            if (moves < 0)
                throw std::invalid_argument("Invalid moves passed: " + moves);
#endif
            total_moves = moves;
        }

        inline Color get_turn() const { return turn; }
        inline CastlingRights get_castling() const { return castl_rights; }
        inline Square get_en_passant() const { return en_passant; }
        inline int get_fifty_rule() const { return fifty_moves_rule; }
        inline int get_total_moves() const { return total_moves; }
        inline Accumulator get_accumulator() const
        {
            return Accumulator{static_cast<u8>(castl_rights), static_cast<u8>(white_king_position), static_cast<u8>(black_king_position), static_cast<u8>(en_passant)};
        }
        void clear_board()
        {
            en_passant = No_Square;
            for (Piece &piece : table)
            {
                piece = No_Piece;
            }
        }

        template <Color color>
        constexpr Square &get_right_king_position()
        {
            if constexpr (color)
                return white_king_position;
            else
                return black_king_position;
        }

        template <Color color>
        inline Accumulator make_move(const Move_full_info move)
        {
            turn = static_cast<Color>(!turn);
            const Accumulator accumulator{static_cast<u8>(castl_rights), static_cast<u8>(white_king_position), static_cast<u8>(black_king_position), static_cast<u8>(en_passant), static_cast<u8>(table[move.to_square]), static_cast<u8>(fifty_moves_rule)};
            ++fifty_moves_rule;
            ++total_moves;

            if (table[move.from_square] == King_with_color<color>())
            {
                get_right_king_position<color>() = static_cast<Square>(move.to_square);
                remove_all_castle<color>();
            }
            switch (move.from_square)
            {
            case short_castling_rook_from_square<color>():
                remove_short_castle<color>();
                break;
            case long_castling_rook_from_square<color>():
                remove_long_castle<color>();
            }

            switch (move.to_square)
            {
            case short_castling_rook_from_square<change_color(color)>():
                remove_short_castle<change_color(color)>();
                break;
            case long_castling_rook_from_square<change_color(color)>():
                remove_long_castle<change_color(color)>();
            }

            en_passant = No_Square;
            if (table[move.from_square] == Pawn_with_color<color>())
            {
                fifty_moves_rule = 0;
                if ((move.to_square - move.from_square) == pawn_double_move_distance<color>())
                    en_passant = static_cast<Square>(move.from_square + pawn_en_passant_direction_from<color>());
            }
            else if (table[move.to_square] != No_Piece)
            {
                fifty_moves_rule = 0;
            }

            switch (move.special)
            {
            case No_special:
                table[move.to_square] = table[move.from_square];
                break;

            case SP_Promotion:
                table[move.to_square] = static_cast<Piece>(move.promotion + static_cast<u8>(Knight_with_color<color>()));
                break;
            case SP_castling:
                do_rook_castling<color>(move.to_square);
                table[move.to_square] = King_with_color<color>();
                break;
            case SP_en_passant:
                do_en_passant_remove<color>(move.to_square);
                table[move.to_square] = Pawn_with_color<color>();
            }
            table[move.from_square] = No_Piece;
            return accumulator;
        }

        template <Color color>
        inline void undo_move(const Move_full_info move, const Accumulator accumulator)
        {
            turn = static_cast<Color>(!turn);
            --total_moves;
            table[move.from_square] = table[move.to_square];
            switch (move.special)
            {
            case SP_castling:
                undo_rook_castling<color>(move.to_square);
                table[move.to_square] = King_with_color<color>();
                break;
            case SP_en_passant:
                undo_en_passant_remove<color>(move.to_square);
                // table[move.to_square] = Pawn_with_color<color>();
                break;
            case SP_Promotion:
                table[move.from_square] = Pawn_with_color<color>();
            default:
                break;
            }
            table[move.to_square] = static_cast<Piece>(accumulator.piece_to_revive);
            restore_info(accumulator);
        }

        void find_kings_and_check_pawns()
        {
            white_king_position = No_Square;
            black_king_position = No_Square;
            for (int i = 0; i < 64; ++i)
            {
                switch (table[i])
                {
                case W_King:
                    // #ifdef MAESTRO_DEBUG
                    if (white_king_position != No_Square)
                        throw std::logic_error("Too many white kings!");
                    // #endif
                    white_king_position = static_cast<Square>(i);

                    break;

                case B_King:
                    // #ifdef MAESTRO_DEBUG
                    if (black_king_position != No_Square)
                        throw std::logic_error("Too many black kings!");
                    // #endif
                    black_king_position = static_cast<Square>(i);
                    break;

                case W_Pawn:
                case B_Pawn:
                    if (!is_legal_pawn_square(i))
                        throw std::logic_error("Illegal pawns position!");
                default:
                    break;
                }
            }

            if (white_king_position == No_Square)
                throw std::logic_error("No white king!");
            if (black_king_position == No_Square)
                throw std::logic_error("No black king!");
        }

        template <Color color>
        inline int eval()
        {
            if constexpr (color)
                return eval_position<color>();
            else
                return -eval_position<color>();
        }

        u64 get_specials_hash()
        {
            u64 hash = 0;

            if (contains(castl_rights, White_OO))
                hash ^= White_OO_hash;
            if (contains(castl_rights, White_OOO))
                hash ^= White_OOO_hash;

            if (contains(castl_rights, Black_OO))
                hash ^= Black_OO_hash;
            if (contains(castl_rights, Black_OOO))
                hash ^= Black_OOO_hash;

            if (en_passant != No_Square)
                hash ^= en_passant_files_hash[en_passant % 8];

            return hash;
        }

        u64 get_hash()
        {
            u64 hash = 0;

            for (int i = 0; i < 64; ++i)
            {
                if (table[i] != No_Piece)
                    hash ^= zobrist_hashtable[i][table[i]];
            }

            if (!turn)
                hash ^= black_side_to_move_hash;

            hash ^= get_specials_hash();

            return hash;
        }

        template <Color color>
        u64 get_hash(u64 hash, Move_full_info move)
        {
            hash ^= black_side_to_move_hash;
            if (table[move.to_square] != No_Piece)
                hash ^= zobrist_hashtable[move.to_square][table[move.to_square]];

            hash ^= zobrist_hashtable[move.from_square][table[move.from_square]];

            switch (move.special)
            {
            case No_special:
                hash ^= zobrist_hashtable[move.to_square][table[move.from_square]];
                break;
            case SP_Promotion:
                hash ^= zobrist_hashtable[move.to_square][move.promotion + static_cast<u8>(Knight_with_color<color>())];
                break;
            case SP_castling:
                do_rook_hashing<color>(move.to_square, hash);
                hash ^= zobrist_hashtable[move.to_square][King_with_color<color>()];
                break;
            case SP_en_passant:
                do_en_passant_hashing<color>(move.to_square, hash);
                hash ^= zobrist_hashtable[move.to_square][Pawn_with_color<color>()];
            }

            return hash;
        }

       

        template <Color clr>
        void sort_moves(Movelist_ref moves, const int last_moved_id)
        {
            int midlegame_phase = 0;
            for (int i = 0; i != 64; ++i)
            {
                midlegame_phase += phase_table[table[i]];
            }

            if (midlegame_phase > 24)
                midlegame_phase = 24;

            const int endgame_phase = 24 - midlegame_phase;

            std::pair<int, Move_full_info> sorting_bucket[350];
            int size = moves.count();
            for (int i = 0; i < size; ++i)
            {
                sorting_bucket[i] = std::make_pair(eval_move<clr>(moves[i], last_moved_id, midlegame_phase, endgame_phase), moves[i]);
            }

            // insertionsort(sorting_bucket, sorting_bucket + size);
            std::sort(sorting_bucket, sorting_bucket + size);
            for (int i = 0; i < size; ++i)
            {
                moves[i] = sorting_bucket[i].second;
            }


        }

        template <Color clr>
        void sort_moves_with_hashed_move(Movelist_ref moves, const int last_moved_id, const Move_full_info move)
        {
            int midlegame_phase = 0;
            for (int i = 0; i != 64; ++i)
            {
                midlegame_phase += phase_table[table[i]];
            }

            if (midlegame_phase > 24)
                midlegame_phase = 24;

            const int endgame_phase = 24 - midlegame_phase;

            std::pair<int, Move_full_info> sorting_bucket[350];
            int size = moves.count();
            for (int i = 0; i < size; ++i)
            {
                const int move_eval = (moves[i] == move) ? -10000
                                                         : eval_move<clr>(moves[i], last_moved_id, midlegame_phase, endgame_phase);

                sorting_bucket[i] = std::make_pair(move_eval, moves[i]);
            }

            insertionsort(sorting_bucket, sorting_bucket + size);
            for (int i = 0; i < size; ++i)
            {
                moves[i] = sorting_bucket[i].second;
            }
        }

        inline void restore_info(const Accumulator &accumulator)
        {
            castl_rights = static_cast<CastlingRights>(accumulator.castling_rights);
            white_king_position = static_cast<Square>(accumulator.white_king_position);
            black_king_position = static_cast<Square>(accumulator.black_king_position);
            en_passant = static_cast<Square>(accumulator.en_passant);
            fifty_moves_rule = accumulator.fifty_moves;
        }
    };

    template <uint _size>
    class Boardlist
    {
    public:
        types::array<Board, _size> arr;
        int count;
        constexpr Boardlist() : count(0) {}
        constexpr Boardlist(uint size) : count(size) {}
        constexpr void push(const Board &move)
        {
            arr[count++] = move;
        }
        constexpr const Boardlist &peek()
        {
            return arr[count];
        }
        constexpr void pop()
        {
            --count;
        }
        constexpr uint size()
        {
            return count;
        }
        constexpr void clear_list()
        {
            count = 0;
        }
        constexpr void deep_clear_list()
        {
            count = 0;
            for (auto &i : arr)
            {
                i = Board();
            }
        }
        constexpr void set_size(uint size)
        {
            count = size;
        }
        constexpr int contains_count(const Board &board)
        {
            int count_contains = 0;
            for (int i = 0; i < count; ++i)
            {
                count_contains += (board == arr[i]);
            }
            return count_contains;
        }
    };

}