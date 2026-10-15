#pragma once
#include "../Board/board.hpp"
#include "../tables.hpp"

namespace chess
{

    class Movegen
    {
    private:
        Board &board;
        Movelist_ref &list;

    public:
        Movegen(Board &brd, Movelist_ref &_list) : board(brd), list(_list)
        {
        }

        Movelist_ref get_list() { return list; }

        template <Color color>
        PositionState gen_all_moves()
        {
            PositionState state;
            int attack_sq;
            Direction attack_dir;
            const int king_id = board.get_right_king_position<color>();
            std::tie(state, attack_sq, attack_dir) = get_attackers_information<color>(king_id);
            gen_all_moves<color>(king_id, state, attack_sq, attack_dir);
            return state;
        }

        template <Color clr>
        inline bool is_id_under_any_check(const int id)
        {
            return is_id_under_king_check<clr>(id) ||
                   is_id_under_pawn_check<clr>(id) ||
                   is_id_under_horizontal_vertical_check<clr>(id) ||
                   is_id_under_diagonal_check<clr>(id) ||
                   is_id_under_knight_check<clr>(id);
        }

        /// @param id is guaranteed that it`s no No_Square
        template <Color color>
        inline bool is_promotion_square(const int id)
        {
            if constexpr (color)
                return (id >= SQ_A8);
            else
                return (id <= SQ_H1);
        }

        template <Color clr>
        inline std::tuple<PositionState, Square, Direction> get_attackers_information(const int king_pos)
        {
            int attackers_counter = 0;
            int attacker_square = No_Square;
            Piece attacker = No_Piece;
            Direction attack_dir = No_Direction;
            int local_square = No_Square;

            const BishopNode bishop_squares = bishop_squares_moves[king_pos];

            if (bishop_squares.LeftUp != No_Square)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, bishop_squares.LeftUp, LeftUp);

            if (is_diagonal_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = LeftUp;
            }

            attacker = No_Piece;
            if (bishop_squares.RightUp != No_Square)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, bishop_squares.RightUp, RightUp);

            if (is_diagonal_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = RightUp;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            if (bishop_squares.LeftDown != No_Square)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, bishop_squares.LeftDown, LeftDown);

            if (is_diagonal_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = LeftDown;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            if (bishop_squares.RightDown != No_Square)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, bishop_squares.RightDown, RightDown);

            if (is_diagonal_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = RightDown;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            int to_id = king_pos & 0b000'111; // Down
            if (to_id != king_pos)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, to_id, Down);

            if (is_horizontal_vertical_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = Down;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            to_id = king_pos | 0b111'000; // Up
            if (to_id != king_pos)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, to_id, Up);

            if (is_horizontal_vertical_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = Up;
                attacker = No_Piece;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            to_id = king_pos & 0b111'000; // Left
            if (to_id != king_pos)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, to_id, Left);

            if (is_horizontal_vertical_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = Left;
                attacker = No_Piece;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            to_id = king_pos | 0b000'111; // Right
            if (to_id != king_pos)
                std::tie(local_square, attacker) = first_piece_in_direction_and_id(king_pos, to_id, Right);

            if (is_horizontal_vertical_checker<clr>(attacker))
            {
                attacker_square = local_square;
                ++attackers_counter;
                attack_dir = Right;
                attacker = No_Piece;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            attacker = No_Piece;

            const KnightNode knight = knight_squares_moves[king_pos];
            for (const int to : knight.moves)
            {
                if ((to != No_Square) && (board[to] == Knight_with_color<change_color(clr)>()))
                {
                    ++attackers_counter;
                    attacker_square = to;
                    attack_dir = No_Direction;
                }
                if (attackers_counter == 2)
                    return {double_check, No_Square, No_Direction};
            }

            const PawnNode pawn_attack = colored_pawn_attack_on_id<clr>(king_pos);

            if ((pawn_attack.Left != No_Square) && board[pawn_attack.Left] == Pawn_with_color<change_color(clr)>())
            {
                attacker_square = pawn_attack.Left;
                ++attackers_counter;
                attack_dir = No_Direction;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            if ((pawn_attack.Right != No_Square) && board[pawn_attack.Right] == Pawn_with_color<change_color(clr)>())
            {
                attacker_square = pawn_attack.Right;
                ++attackers_counter;
                attack_dir = No_Direction;
            }

            if (attackers_counter == 2)
                return {double_check, No_Square, No_Direction};

            return {static_cast<PositionState>(attackers_counter), static_cast<Square>(attacker_square), static_cast<Direction>(attack_dir)};
        }

        template <Color color>
        inline void gen_all_moves(int king_id, PositionState state, int attack_sq, Direction attack_dir)
        {
            list.clear_moves();
            switch (state)
            {
            case double_check:
                all_king_rays<color>(king_id);
                break;
            case check:
                gen_all_moves_with_check<color>(generate_check_mask<White>(king_id, attack_sq, attack_dir), king_id);
                break;
            case quite:
                gen_all_quiet_moves<color>(king_id);
            }
        }

    private:
        template <Color color>
        void gen_all_moves_with_check(const u64 check_mask, const int king_id)
        {
            const u64 pin_mask = gen_only_pin_mask_when_check<color>(king_id);

            for (int i = 0; i < 64; ++i)
            {
                if (contains(pin_mask, bit_at(i)))
                    continue;

                switch (board[i])
                {
                case Pawn_with_color<color>():
                    all_pawn_moves_with_checkmask<color>(i, check_mask);
                    break;
                case Knight_with_color<color>():
                    all_knight_rays_with_mask<color>(i, check_mask);
                    break;
                case Bishop_with_color<color>():
                    all_bishop_rays_with_mask<color>(i, check_mask);
                    break;
                case Rook_with_color<color>():
                    all_rook_rays_with_mask<color>(i, check_mask);
                    break;
                case Queen_with_color<color>():
                    all_queen_rays_with_mask<color>(i, check_mask);
                    break;
                default:
                    break;
                }
            }

            gen_en_passant<color>();
            all_king_rays<color>(king_id);
        }

        template <Color color>
        void gen_all_quiet_moves(const int king_id)
        {
            const u64 pin_mask = gen_all_pinned_moves<color>(king_id);

            for (int i = 0; i < 64; ++i)
            {
                if (contains(pin_mask, bit_at(i)))
                    continue;

                switch (board[i])
                {
                case Pawn_with_color<color>():
                    all_pawn_moves<color>(i);
                    break;
                case Knight_with_color<color>():
                    all_knight_rays<color>(i);
                    break;
                case Bishop_with_color<color>():
                    all_bishop_rays<color>(i);
                    break;
                case Rook_with_color<color>():
                    all_rook_rays<color>(i);
                    break;
                case Queen_with_color<color>():
                    all_queen_rays<color>(i);
                    break;
                default:
                    break;
                }
            }

            gen_en_passant<color>();
            all_king_rays<color>(king_id);
            gen_castling<color>();
        }

        inline bool is_piece(const int id)
        {
            return board[id] != No_Piece;
        }

        inline Color piece_color(const int id)
        {
            return static_cast<Color>(board[id] < 6);
        }

        inline Color piece_color(const Piece piece)
        {
            return static_cast<Color>(piece < 6);
        }

        template <Color clr>
        inline bool not_same_color_or_empty(const int id)
        {
            return (board[id] == No_Piece) || (piece_color(id) != clr);
        }

        template <Color clr>
        inline bool piece_and_not_same_color(const int id)
        {
            return (board[id] != No_Piece) && (piece_color(id) != clr);
        }

        inline bool is_empty(const int id)
        {
            return (board[id] == No_Piece);
        }

        template <Color clr>
        inline void rook_bishop_gen_ray(int from_id, int to_id, Direction direction)
        {
            int cur_id = from_id + direction;
            to_id += direction;
            while (cur_id != to_id)
            {
                if (is_piece(cur_id))
                {
                    if (piece_color(cur_id) != clr)
                    {
                        list.add(Move_full_info(from_id, cur_id, No_promotion, No_special));
                    }
                    break;
                }
                list.add(Move_full_info(from_id, cur_id, No_promotion, No_special));
                cur_id += direction;
            }
        }

        template <bool into_enemy>
        inline void rook_bishop_gen_ray_guaranteed_into_piece(const int from_id, const Direction direction)
        {
            int cur_id = from_id + direction;
            while (true)
            {
                if (is_piece(cur_id))
                {
                    if constexpr (into_enemy)
                    {
                        list.add(Move_full_info(from_id, cur_id, No_promotion, No_special));
                    }
                    return;
                }
                list.add(Move_full_info(from_id, cur_id, No_promotion, No_special));
                cur_id += direction;
            }
        }

        template <Color clr>
        inline void rook_bishop_gen_ray_with_mask(int from_id, int to_id, Direction direction, const u64 check_mask)
        {
            int cur_id = from_id + direction;
            to_id += direction;

            while (cur_id != to_id)
            {
                if (contains(check_mask, bit_at(cur_id)))
                {
                    list.add(Move_full_info(from_id, cur_id, No_promotion, No_special));
                }
                if (is_piece(cur_id))
                    return;
                cur_id += direction;
            }
        }

        template <Color clr>
        inline void all_knight_rays(const int from)
        {
            const KnightNode knight(knight_squares_moves[from]);
            for (const int to : knight.moves)
            {

                if ((to != No_Square))
                {
                    if (piece_color(to) != clr || is_empty(to))
                        list.add(Move_full_info(from, to, No_promotion, No_special));
                }
            }
        }

        template <Color clr>
        inline void all_knight_rays_with_mask(const int from, const u64 check_mask)
        {
            const KnightNode knight = knight_squares_moves[from];

            for (const int to : knight.moves)
            {
                if ((to != No_Square) && contains(check_mask, bit_at(to)))
                {
                    list.add(Move_full_info(from, to, No_promotion, No_special));
                }
            }
        }

        template <Color clr>
        inline void all_queen_rays(const int from)
        {
            all_rook_rays<clr>(from);
            all_bishop_rays<clr>(from);
        }

        template <Color clr>
        inline void all_queen_rays_with_mask(const int from, const u64 check_mask)
        {
            all_rook_rays_with_mask<clr>(from, check_mask);
            all_bishop_rays_with_mask<clr>(from, check_mask);
        }

        template <Color clr>
        inline bool is_diagonal_checker(const Piece piece)
        {
            return (piece == Queen_with_color<change_color(clr)>()) || (piece == Bishop_with_color<change_color(clr)>());
        }

        template <Color clr>
        inline bool is_horizontal_vertical_checker(const Piece piece)
        {
            return (piece == Queen_with_color<change_color(clr)>()) || (piece == Rook_with_color<change_color(clr)>());
        }

        inline Piece first_piece_in_direction(const int from, const int to, const Direction direction)
        {
            int cur_id = from;
            while (cur_id != to)
            {
                cur_id += direction;
                if (board[cur_id] != No_Piece)
                {
                    return board[cur_id];
                }
            }
            return No_Piece;
        }

        template <Color clr>
        inline bool is_id_under_diagonal_check(const int id)
        {
            const BishopNode bishop_squares = bishop_squares_moves[id];

            if ((bishop_squares.LeftUp != No_Square) && is_diagonal_checker<clr>(first_piece_in_direction(id, bishop_squares.LeftUp, LeftUp)))
                return true;

            if ((bishop_squares.RightUp != No_Square) && is_diagonal_checker<clr>(first_piece_in_direction(id, bishop_squares.RightUp, RightUp)))
                return true;

            if ((bishop_squares.LeftDown != No_Square) && is_diagonal_checker<clr>(first_piece_in_direction(id, bishop_squares.LeftDown, LeftDown)))
                return true;

            if ((bishop_squares.RightDown != No_Square) && is_diagonal_checker<clr>(first_piece_in_direction(id, bishop_squares.RightDown, RightDown)))
                return true;

            return false;
        }

        template <Color clr>
        inline bool is_id_under_horizontal_vertical_check(const int id)
        {

            int to_id = id & 0b000'111; // Down
            if ((to_id != id) && is_horizontal_vertical_checker<clr>(first_piece_in_direction(id, to_id, Down)))
                return true;

            to_id = id | 0b111'000; // Up
            if ((to_id != id) && is_horizontal_vertical_checker<clr>(first_piece_in_direction(id, to_id, Up)))
                return true;

            to_id = id & 0b111'000; // Left
            if ((to_id != id) && is_horizontal_vertical_checker<clr>(first_piece_in_direction(id, to_id, Left)))
                return true;

            to_id = id | 0b000'111; // Right
            if ((to_id != id) && is_horizontal_vertical_checker<clr>(first_piece_in_direction(id, to_id, Right)))
                return true;

            return false;
        }

        template <Color clr>
        inline bool is_id_under_knight_check(const int id)
        {
            const KnightNode knight = knight_squares_moves[id];
            for (const int to : knight.moves)
            {
                if ((to != No_Square) && (board[to] == Knight_with_color<change_color(clr)>()))
                    return true;
            }
            return false;
        }

        template <Color clr>
        inline bool is_id_under_king_check(const int id)
        {
            return king_moves[board.get_right_king_position<change_color(clr)>()] & (1ULL << id);
        }

        template <Color clr>
        inline bool is_id_under_pawn_check(const int id)
        {
            const PawnNode pawn{colored_pawn_attack_on_id<clr>(id)};
            if ((pawn.Left != No_Square) &&
                (board[pawn.Left] == Pawn_with_color<change_color(clr)>()))
                return true;
            if ((pawn.Right != No_Square) &&
                (board[pawn.Right] == Pawn_with_color<change_color(clr)>()))
                return true;

            return false;
        }

        inline u64 mask_in_dir(const int from_id, int to_id, const Direction direction)
        {
            int cur_id = from_id + direction;
            to_id += direction;
            u64 mask = 0;
            while (cur_id != to_id)
            {
                mask |= bit_at(cur_id);
                if (is_piece(cur_id))
                {
                    return mask;
                }
                cur_id += direction;
            }
            return mask;
        }

        template <Color clr>
        inline void all_king_rays(const int from)
        {

            const KingNode king = king_squares_moves[from];
            board[from] = No_Piece;

            for (int to : king.moves)
            {
                if ((to != No_Square) && not_same_color_or_empty<clr>(to) && (!is_id_under_any_check<clr>(to)))
                    list.add(Move_full_info(from, to, No_promotion, No_special));
            }
            board[from] = King_with_color<clr>();
        }

        template <Color clr>
        inline void all_rook_rays(const int id)
        {
            int to_id = id & 0b000'111; // Down
            if (to_id != id)
                rook_bishop_gen_ray<clr>(id, to_id, Down);

            to_id = id | 0b111'000; // Up
            if (to_id != id)
                rook_bishop_gen_ray<clr>(id, to_id, Up);

            to_id = id & 0b111'000; // Left
            if (to_id != id)
                rook_bishop_gen_ray<clr>(id, to_id, Left);

            to_id = id | 0b000'111; // Right
            if (to_id != id)
                rook_bishop_gen_ray<clr>(id, to_id, Right);
        }

        template <Color clr>
        inline void all_rook_rays_with_mask(const int id, const u64 check_mask)
        {

            int to_id = id & 0b000'111; // Down
            if (to_id != id)
                rook_bishop_gen_ray_with_mask<clr>(id, to_id, Down, check_mask);

            to_id = id | 0b111'000; // Up
            if (to_id != id)
                rook_bishop_gen_ray_with_mask<clr>(id, to_id, Up, check_mask);

            to_id = id & 0b111'000; // Left
            if (to_id != id)
                rook_bishop_gen_ray_with_mask<clr>(id, to_id, Left, check_mask);

            to_id = id | 0b000'111; // Right
            if (to_id != id)
                rook_bishop_gen_ray_with_mask<clr>(id, to_id, Right, check_mask);
        }

        template <Color clr>
        inline void all_bishop_rays(const int id)
        {
            // Gen tables
            const BishopNode edge_squares(bishop_squares_moves[id]);

            if (edge_squares.LeftUp != No_Square)
                rook_bishop_gen_ray<clr>(id, edge_squares.LeftUp, LeftUp);

            if (edge_squares.LeftDown != No_Square)
                rook_bishop_gen_ray<clr>(id, edge_squares.LeftDown, LeftDown);

            if (edge_squares.RightUp != No_Square)
                rook_bishop_gen_ray<clr>(id, edge_squares.RightUp, RightUp);

            if (edge_squares.RightDown != No_Square)
                rook_bishop_gen_ray<clr>(id, edge_squares.RightDown, RightDown);
        }

        template <Color clr>
        inline void all_bishop_rays_with_mask(const int id, const u64 mask)
        {

            // Gen tables
            const BishopNode edge_squares(bishop_squares_moves[id]);

            if (edge_squares.LeftUp != No_Square)
                rook_bishop_gen_ray_with_mask<clr>(id, edge_squares.LeftUp, LeftUp, mask);

            if (edge_squares.LeftDown != No_Square)
                rook_bishop_gen_ray_with_mask<clr>(id, edge_squares.LeftDown, LeftDown, mask);

            if (edge_squares.RightUp != No_Square)
                rook_bishop_gen_ray_with_mask<clr>(id, edge_squares.RightUp, RightUp, mask);

            if (edge_squares.RightDown != No_Square)
                rook_bishop_gen_ray_with_mask<clr>(id, edge_squares.RightDown, RightDown, mask);
        }

        inline std::tuple<int, Piece> first_piece_in_direction_and_id(int from_id, int to_id, Direction direction)
        {
            to_id += direction;
            int cur_id = from_id + direction;
            while (cur_id != to_id)
            {
                if (board[cur_id] != No_Piece)
                    return {cur_id, board[cur_id]};
                cur_id += direction;
            }
            return {No_Square, No_Piece};
        }

        inline int get_end_square_from_bishop(const int id, const Direction dir)
        {
            switch (dir)
            {
            case LeftUp:
                return bishop_squares_moves[id].LeftUp;
            case RightUp:
                return bishop_squares_moves[id].RightUp;
            case LeftDown:
                return bishop_squares_moves[id].LeftDown;
            case RightDown:
                return bishop_squares_moves[id].RightDown;
            default:
                return No_Square;
            }
        }

        inline int get_end_square_from_rook(const int id, const Direction dir)
        {
            switch (dir)
            {
            case Down:
                return id & 0b000'111; // Down
            case Up:
                return id | 0b111'000; // Up
            case Left:
                return id & 0b111'000; // Left
            case Right:
                return id | 0b000'111; // Right
            default:
                return No_Square;
            }
        }

        template <Color clr>
        inline u64 generate_check_mask(const int from, int to, const Direction dir)
        {
            if (dir == No_Direction)
                return bit_at(to);
            u64 mask = 0;
            to += dir;
            int cur_id = from + dir;

            while (cur_id != to)
            {
                mask |= bit_at(cur_id);
                cur_id += dir;
            }
            return mask;
        }

        template <Color color>
        inline bool is_pawn_moved(const int id)
        {
            if constexpr (color)
                return (id >= SQ_A2) && (id <= SQ_H2);
            else
                return (id >= SQ_A7) && (id <= SQ_H7);
        }

        template <Color color>
        inline void pawn_take_gen(const int id, const int to_id)
        {
            if (piece_and_not_same_color<color>(to_id))
            {
                if (is_promotion_square<color>(to_id))
                {
                    list.add(Move_full_info(id, to_id, promote_to_queen, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_rook, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_bishop, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_knight, SP_Promotion));
                }
                else
                {
                    list.add(Move_full_info(id, to_id, No_promotion, No_special));
                }
            }
        }

        template <Color color>
        inline void pawn_not_take_gen(const int id, const Direction direction)
        {
            const int to_id = id + direction;
            if (is_empty(to_id))
            {
                if (is_promotion_square<color>(to_id))
                {
                    list.add(Move_full_info(id, to_id, promote_to_queen, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_rook, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_bishop, SP_Promotion));
                    list.add(Move_full_info(id, to_id, promote_to_knight, SP_Promotion));
                    return;
                }
                list.add(Move_full_info(id, to_id, No_promotion, No_special));

                if (is_pawn_moved<color>(id) && is_empty(to_id + direction))
                {
                    list.add(Move_full_info(id, to_id + direction, No_promotion, No_special));
                }
            }
        }

        template <Color color>
        inline void all_pawn_moves(const int id)
        {
            const PawnNode pawn = colored_pawn_attack_on_id<color>(id);

            if (pawn.Left != No_Square)
                pawn_take_gen<color>(id, pawn.Left);
            if (pawn.Right != No_Square)
                pawn_take_gen<color>(id, pawn.Right);

            constexpr Direction move_direction = pawn_move_dir<color>();

            pawn_not_take_gen<color>(id, move_direction);
        }

        template <Color color>
        inline void pawn_not_take_gen_with_checkmask(const int id, const Direction direction, const u64 mask)
        {
            const int to_id = id + direction;
            if (is_empty(to_id))
            {
                if (contains(mask, bit_at(to_id)))
                {
                    if (is_promotion_square<color>(to_id))
                    {
                        list.add(Move_full_info(id, to_id, promote_to_queen, SP_Promotion));
                        list.add(Move_full_info(id, to_id, promote_to_rook, SP_Promotion));
                        list.add(Move_full_info(id, to_id, promote_to_bishop, SP_Promotion));
                        list.add(Move_full_info(id, to_id, promote_to_knight, SP_Promotion));
                        return;
                    }
                    list.add(Move_full_info(id, to_id, No_promotion, No_special));
                }
                if (is_pawn_moved<color>(id) && is_empty(to_id + direction) && contains(mask, bit_at(to_id + direction)))
                {
                    list.add(Move_full_info(id, to_id + direction, No_promotion, No_special));
                }
            }
        }

        template <Color color>
        inline void all_pawn_moves_with_checkmask(const int id, const u64 mask)
        {
            const PawnNode pawn = colored_pawn_attack_on_id<color>(id);

            if ((pawn.Left != No_Square) && contains(mask, bit_at(pawn.Left)))
                pawn_take_gen<color>(id, pawn.Left);
            if ((pawn.Right != No_Square) && contains(mask, bit_at(pawn.Right)))
                pawn_take_gen<color>(id, pawn.Right);

            constexpr Direction move_direction = pawn_move_dir<color>();

            pawn_not_take_gen_with_checkmask<color>(id, move_direction, mask);
        }

        template <Color color>
        inline void pawn_pinned_move_gen(const int id, const Direction direction)
        {
            if ((direction != Up) && (direction != Down))
                return;
            constexpr Direction move_direction = pawn_move_dir<color>();
            pawn_not_take_gen<color>(id, move_direction);
        }

        template <Color color>
        inline void horizontal_vertical_pinned_gen(const int king_id, const int to_id, const Direction direction, u64 &mask)
        {
            int piece_id;
            Piece piece;
            std::tie(piece_id, piece) = first_piece_in_direction_and_id(king_id, to_id, direction);

            if ((piece != No_Piece) && (piece_color(piece) == color) && (piece_id != to_id) &&
                is_horizontal_vertical_checker<color>(first_piece_in_direction(piece_id, to_id, direction)))
            {
                mask |= bit_at(piece_id);
                switch (piece)
                {
                case Queen_with_color<color>():
                case Rook_with_color<color>():
                    rook_bishop_gen_ray_guaranteed_into_piece<true>(piece_id, direction);
                    rook_bishop_gen_ray_guaranteed_into_piece<false>(piece_id, -direction);
                    break;
                case Pawn_with_color<color>():
                    pawn_pinned_move_gen<color>(piece_id, direction);
                default:
                    return;
                }
            }
        }

        template <Color color>
        inline bool is_pawn_take_direction_legal(const Direction direction)
        {
            if constexpr (color)
                return (direction == LeftUp) || (direction == RightUp);
            else
                return (direction == LeftDown) || (direction == RightDown);
        }

        template <Color color>
        inline void diagonal_pinned_gen(const int king_id, const int to_id, const Direction direction, u64 &mask)
        {
            int piece_id;
            Piece piece;
            std::tie(piece_id, piece) = first_piece_in_direction_and_id(king_id, to_id, direction);

            if ((piece != No_Piece) && (piece_color(piece) == color) && (piece_id != to_id) &&
                is_diagonal_checker<color>(first_piece_in_direction(piece_id, to_id, direction)))
            {
                mask |= bit_at(piece_id);
                switch (piece)
                {
                case Queen_with_color<color>():
                case Bishop_with_color<color>():
                    rook_bishop_gen_ray_guaranteed_into_piece<true>(piece_id, direction);
                    rook_bishop_gen_ray_guaranteed_into_piece<false>(piece_id, -direction);
                    break;
                case Pawn_with_color<color>():
                    if (is_pawn_take_direction_legal<color>(direction))
                        pawn_take_gen<color>(piece_id, piece_id + direction);
                default:
                    return;
                }
            }
        }

        template <Color color>
        inline void update_pin_mask_horizontal_vertical(const int king_id, const int to_id, const Direction direction, u64 &mask)
        {
            int piece_id;
            Piece piece;
            std::tie(piece_id, piece) = first_piece_in_direction_and_id(king_id, to_id, direction);

            if ((piece != No_Piece) && (piece_color(piece) == color) && (piece_id != to_id) &&
                is_horizontal_vertical_checker<color>(first_piece_in_direction(piece_id, to_id, direction)))
                mask |= bit_at(piece_id);
        }

        template <Color color>
        inline void update_pin_mask_diagonal(const int king_id, const int to_id, const Direction direction, u64 &mask)
        {
            int piece_id;
            Piece piece;
            std::tie(piece_id, piece) = first_piece_in_direction_and_id(king_id, to_id, direction);

            if ((piece != No_Piece) && (piece_color(piece) == color) && (piece_id != to_id) &&
                is_diagonal_checker<color>(first_piece_in_direction(piece_id, to_id, direction)))
                mask |= bit_at(piece_id);
        }

        template <Color color>
        inline u64 gen_only_pin_mask_when_check(const int id)
        {
            u64 mask = 0;
            int to_id = id & 0b000'111; // Down
            if (to_id != id)
                update_pin_mask_horizontal_vertical<color>(id, to_id, Down, mask);

            to_id = id | 0b111'000; // Up
            if (to_id != id)
                update_pin_mask_horizontal_vertical<color>(id, to_id, Up, mask);

            to_id = id & 0b111'000; // Left
            if (to_id != id)
                update_pin_mask_horizontal_vertical<color>(id, to_id, Left, mask);

            to_id = id | 0b000'111; // Right
            if (to_id != id)
                update_pin_mask_horizontal_vertical<color>(id, to_id, Right, mask);

            const BishopNode bishop = bishop_squares_moves[id];

            if (bishop.LeftUp != No_Square)
                update_pin_mask_diagonal<color>(id, bishop.LeftUp, LeftUp, mask);

            if (bishop.RightUp != No_Square)
                update_pin_mask_diagonal<color>(id, bishop.RightUp, RightUp, mask);

            if (bishop.LeftDown != No_Square)
                update_pin_mask_diagonal<color>(id, bishop.LeftDown, LeftDown, mask);

            if (bishop.RightDown != No_Square)
                update_pin_mask_diagonal<color>(id, bishop.RightDown, RightDown, mask);

            return mask;
        }

        template <Color color>
        inline u64 gen_all_pinned_moves(const int id)
        {
            u64 mask = 0;
            int to_id = id & 0b000'111; // Down
            if (to_id != id)
                horizontal_vertical_pinned_gen<color>(id, to_id, Down, mask);

            to_id = id | 0b111'000; // Up
            if (to_id != id)
                horizontal_vertical_pinned_gen<color>(id, to_id, Up, mask);

            to_id = id & 0b111'000; // Left
            if (to_id != id)
                horizontal_vertical_pinned_gen<color>(id, to_id, Left, mask);

            to_id = id | 0b000'111; // Right
            if (to_id != id)
                horizontal_vertical_pinned_gen<color>(id, to_id, Right, mask);

            const BishopNode bishop = bishop_squares_moves[id];

            if (bishop.LeftUp != No_Square)
                diagonal_pinned_gen<color>(id, bishop.LeftUp, LeftUp, mask);

            if (bishop.RightUp != No_Square)
                diagonal_pinned_gen<color>(id, bishop.RightUp, RightUp, mask);

            if (bishop.LeftDown != No_Square)
                diagonal_pinned_gen<color>(id, bishop.LeftDown, LeftDown, mask);

            if (bishop.RightDown != No_Square)
                diagonal_pinned_gen<color>(id, bishop.RightDown, RightDown, mask);

            return mask;
        }

        template <Color color>
        inline void gen_castling()
        {
            if (can_long_castle<color>())
            {
                list.add(Move_full_info(castling_king_from_square<color>(), long_castling_king_to_square<color>(),
                                        No_promotion, SP_castling));
            }
            if (can_short_castle<color>())
            {
                list.add(Move_full_info(castling_king_from_square<color>(), short_castling_king_to_square<color>(),
                                        No_promotion, SP_castling));
            }
        }

        template <Color clr>
        inline bool can_long_castle()
        {
            board[board.get_right_king_position<clr>()] = No_Piece;
            bool can_castle;
            if constexpr (clr)
            {
                can_castle = board.can_castle_long<clr>() && is_empty(SQ_B1) && is_empty(SQ_C1) && is_empty(SQ_D1) && (!is_id_under_any_check<clr>(SQ_C1)) && (!is_id_under_any_check<clr>(SQ_D1));
            }
            else
            {
                can_castle = board.can_castle_long<clr>() && is_empty(SQ_B8) && is_empty(SQ_C8) && is_empty(SQ_D8) &&
                             (!is_id_under_any_check<clr>(SQ_C8)) && (!is_id_under_any_check<clr>(SQ_D8));
            }

            board[board.get_right_king_position<clr>()] = King_with_color<clr>();
            return can_castle;
        }

        template <Color clr>
        inline bool can_short_castle()
        {
            board[board.get_right_king_position<clr>()] = No_Piece;
            bool can_castle;
            if constexpr (clr)
            {
                can_castle = board.can_castle_short<clr>() &&
                             is_empty(SQ_F1) &&
                             is_empty(SQ_G1) &&
                             (!is_id_under_any_check<clr>(SQ_F1)) && (!is_id_under_any_check<clr>(SQ_G1));
            }
            else
            {
                can_castle = board.can_castle_short<clr>() && is_empty(SQ_F8) && is_empty(SQ_G8) &&
                             (!is_id_under_any_check<clr>(SQ_F8)) && (!is_id_under_any_check<clr>(SQ_G8));
            }
            board[board.get_right_king_position<clr>()] = King_with_color<clr>();
            return can_castle;
        }

        template <Color clr>
        inline void gen_en_passant()
        {
            const Square en_passant = board.get_en_passant();
            if (en_passant == No_Square)
                return;
            const PawnNode pawn = colored_pawn_attack_on_id<change_color(clr)>(en_passant);

            if ((pawn.Left != No_Square) && (board[pawn.Left] == Pawn_with_color<clr>()))
            {
                board[pawn.Left] = No_Piece;
                board[en_passant] = Pawn_with_color<clr>();
                board[en_passant + pawn_en_passant_direction_to<clr>()] = No_Piece;
                if (!is_id_under_any_check<clr>(board.get_right_king_position<clr>()))
                {
                    list.add(Move_full_info(pawn.Left, en_passant, No_promotion, SP_en_passant));
                }
                board[pawn.Left] = Pawn_with_color<clr>();
                board[en_passant] = No_Piece;
                board[en_passant + pawn_en_passant_direction_to<clr>()] = Pawn_with_color<change_color(clr)>();
            }

            if ((pawn.Right != No_Square) && (board[pawn.Right] == Pawn_with_color<clr>()))
            {
                board[pawn.Right] = No_Piece;
                board[en_passant] = Pawn_with_color<clr>();
                board[en_passant + pawn_en_passant_direction_to<clr>()] = No_Piece;
                if (!is_id_under_any_check<clr>(board.get_right_king_position<clr>()))
                {
                    list.add(Move_full_info(pawn.Right, en_passant, No_promotion, SP_en_passant));
                }
                board[pawn.Right] = Pawn_with_color<clr>();
                board[en_passant] = No_Piece;
                board[en_passant + pawn_en_passant_direction_to<clr>()] = Pawn_with_color<change_color(clr)>();
            }
        }
    };
}