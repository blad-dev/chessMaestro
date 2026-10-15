#pragma once
#include "types.hpp"

namespace chess
{

    struct square_walker
    {
    public:
        int rank, file;

        constexpr square_walker() : rank(0), file(0){}
        constexpr square_walker(const int _rank, const int _file) : rank(_rank), file(_file) {}
        constexpr square_walker(const int id) : rank(id / 8), file(id % 8) {}
        constexpr u8 get_id()
        {
            return static_cast<u8>(rank * 8 + file);
        }
        constexpr void set_id(const int id)
        {
            rank = id / 8;
            file = id % 8;
        }
        constexpr bool is_legal()
        {
            return (file >= 0) && (file <= 7) && (rank >= 0) && (rank <= 7);
        }
        constexpr bool is_legal_and_not_on_border()
        {
            return (file >= 1) && (file <= 6) && (rank >= 1) && (rank <= 6);
        }
        constexpr bool is_on_border()
        {
            return (file == 0) || (file == 7) || (rank == 0) || (rank == 7);
        }
        constexpr void step_Up()
        {
            ++rank;
        }
        constexpr void step_Down()
        {
            --rank;
        }
        constexpr void step_Left()
        {
            --file;
        }
        constexpr void step_Right()
        {
            ++file;
        }
        constexpr void step_LeftUp()
        {
            ++rank;
            --file;
        }
        constexpr void step_RightUp()
        {
            ++rank;
            ++file;
        }
        constexpr void step_LeftDown()
        {
            --rank;
            --file;
        }
        constexpr void step_RightDown()
        {
            --rank;
            ++file;
        }
    };

    struct BishopNode
    {
        u8 LeftUp, LeftDown, RightUp, RightDown;

        constexpr BishopNode(const u8 _LeftUp, const u8 _LeftDown, const u8 _RightUp, const u8 _RightDown) : LeftUp(_LeftUp), LeftDown(_LeftDown), RightUp(_RightUp), RightDown(_RightDown) {}

        constexpr BishopNode(const BishopNode &squares) : LeftUp(squares.LeftUp), LeftDown(squares.LeftDown), RightUp(squares.RightUp), RightDown(squares.RightDown) {}

        constexpr BishopNode() : LeftUp(No_Square), LeftDown(No_Square), RightUp(No_Square), RightDown(No_Square) {}
    };

    struct KingNode
    {
        types::array<u8, 8> moves;

        constexpr KingNode(const u8 _left_sq, const u8 _right_sq, const u8 _up_sq, const u8 _down_sq,
                           const u8 _left_up_sq, const u8 _right_up_sq, const u8 _left_down_sq, const u8 _right_down_sq)
        {
            moves[0] = _left_up_sq;
            moves[1] = _up_sq;
            moves[2] = _right_up_sq;

            moves[3] = _left_sq;
            moves[4] = _right_sq;

            moves[5] = _left_down_sq;
            moves[6] = _down_sq;
            moves[7] = _right_down_sq;
        }
        constexpr KingNode(const KingNode &node)
        {
            moves[0] = node.moves[0];
            moves[1] = node.moves[1];
            moves[2] = node.moves[2];

            moves[3] = node.moves[3];
            // moves[4] = node.moves[4];
            moves[4] = node.moves[4];

            moves[5] = node.moves[5];
            moves[6] = node.moves[6];
            moves[7] = node.moves[7];
        }

        constexpr KingNode()
        {
            init();
        }

        constexpr KingNode& operator=(const KingNode& node) = default;
        

    private:
        constexpr void init()
        {

            moves[0] = No_Square; // left up
            moves[1] = No_Square; // up
            moves[2] = No_Square; // right up

            moves[3] = No_Square; // left
            // moves[4] = No_Square; // king pos
            moves[4] = No_Square; // right

            moves[5] = No_Square; // left down
            moves[6] = No_Square; // down
            moves[7] = No_Square; // right down
        }
    };

    struct KnightNode
    {
        types::array<u8, 8> moves;

        constexpr KnightNode(const KingNode &node)
        {
            moves[0] = node.moves[0];
            moves[1] = node.moves[1];
            moves[2] = node.moves[2];

            moves[3] = node.moves[3];
            moves[4] = node.moves[4];

            moves[5] = node.moves[5];
            moves[6] = node.moves[6];
            moves[7] = node.moves[7];
        }

        constexpr KnightNode()
        {
            init();
        }

    private:
        constexpr void init()
        {

            moves[0] = No_Square;
            moves[1] = No_Square;
            moves[2] = No_Square;
            moves[3] = No_Square;
            moves[4] = No_Square;
            moves[5] = No_Square;
            moves[6] = No_Square;
            moves[7] = No_Square;
        }
    };

    struct PawnNode
    {
        u8 Left, Right;
        constexpr PawnNode() : Left(No_Square), Right(No_Square) {}
        constexpr PawnNode(const PawnNode &node) : Left(node.Left), Right(node.Right) {}
    };

    consteval types::array<BishopNode, 64> gen_bishop_to_squares()
    {
        types::array<BishopNode, 64> bishops;
        for (int i = 0; i < 64; ++i)
        {
            square_walker walker(i);

            while (true)
            {
                walker.step_LeftUp();
                if (walker.is_legal())
                {
                    if (walker.is_on_border())
                    {
                        bishops[i].LeftUp = walker.get_id();
                        break;
                    }
                }
                else
                {
                    bishops[i].LeftUp = No_Square;
                    break;
                }
            }

            walker.set_id(i);
            while (true)
            {
                walker.step_RightUp();
                if (walker.is_legal())
                {
                    if (walker.is_on_border())
                    {
                        bishops[i].RightUp = walker.get_id();
                        break;
                    }
                }
                else
                {
                    bishops[i].RightUp = No_Square;
                    break;
                }
            }

            walker.set_id(i);
            while (true)
            {
                walker.step_LeftDown();
                if (walker.is_legal())
                {
                    if (walker.is_on_border())
                    {
                        bishops[i].LeftDown = walker.get_id();
                        break;
                    }
                }
                else
                {
                    bishops[i].LeftDown = No_Square;
                    break;
                }
            }

            walker.set_id(i);
            while (true)
            {
                walker.step_RightDown();
                if (walker.is_legal())
                {
                    if (walker.is_on_border())
                    {
                        bishops[i].RightDown = walker.get_id();
                        break;
                    }
                }
                else
                {
                    bishops[i].RightDown = No_Square;
                    break;
                }
            }
        }
        return bishops;
    }

    consteval types::array<KingNode, 64> gen_king_squares()
    {
        types::array<KingNode, 64> king_moves;

        for (int i = 0; i < 64; i++)
        {
            square_walker walker(i);

            KingNode king_node;

            walker.step_Up();

            if (walker.is_legal())
            {
                king_node.moves[0] = walker.get_id();
            }
            else
            {
                king_node.moves[0] = No_Square;
            }

            walker.set_id(i);

            walker.step_Down();

            if (walker.is_legal())
            {
                king_node.moves[1] = walker.get_id();
            }
            else
            {
                king_node.moves[1] = No_Square;
            }

            walker.set_id(i);

            walker.step_Left();

            if (walker.is_legal())
            {
                king_node.moves[2] = walker.get_id();
            }
            else
            {
                king_node.moves[2] = No_Square;
            }

            walker.set_id(i);

            walker.step_Right();

            if (walker.is_legal())
            {
                king_node.moves[3] = walker.get_id();
            }
            else
            {
                king_node.moves[3] = No_Square;
            }

            walker.set_id(i);

            walker.step_LeftUp();

            if (walker.is_legal())
            {
                king_node.moves[4] = walker.get_id();
            }
            else
            {
                king_node.moves[4] = No_Square;
            }

            walker.set_id(i);

            walker.step_LeftDown();

            if (walker.is_legal())
            {
                king_node.moves[5] = walker.get_id();
            }
            else
            {
                king_node.moves[5] = No_Square;
            }

            walker.set_id(i);

            walker.step_RightUp();

            if (walker.is_legal())
            {
                king_node.moves[6] = walker.get_id();
            }
            else
            {
                king_node.moves[6] = No_Square;
            }

            walker.set_id(i);

            walker.step_RightDown();

            if (walker.is_legal())
            {
                king_node.moves[7] = walker.get_id();
            }
            else
            {
                king_node.moves[7] = No_Square;
            }

            king_moves[i] = king_node;
        }
        return king_moves;
    }

    constexpr int id_at_ij(const int i, const int j)
    {
        return (i * 8 + j);
    }

    constexpr u64 mask_at_ij(const int i, const int j)
    {
        return (static_cast<u64>(1) << (i * 8 + j));
    }

    constexpr bool is_legal(const int i, const int j)
    {
        return (i >= 0) && (i < 8) && (j >= 0) && (j < 8);
    }

    constexpr u64 mask_at_ij_if_legal(const int i, const int j)
    {
        if (is_legal(i, j))
            return mask_at_ij(i, j);
        return 0;
    }

   

    consteval u64 all_knights_masks_at_ij(const int i, const int j)
    {
        u64 mask = mask_at_ij_if_legal(i + 2, j + 1) |
         mask_at_ij_if_legal(i - 2, j + 1) |
          mask_at_ij_if_legal(i + 2, j - 1) |
           mask_at_ij_if_legal(i - 2, j - 1) |
            mask_at_ij_if_legal(i + 1, j + 2) |
             mask_at_ij_if_legal(i - 1, j + 2) |
              mask_at_ij_if_legal(i + 1, j - 2) |
               mask_at_ij_if_legal(i - 1, j - 2);
        return mask;
    }

    consteval types::array<u64, 64> gen_knights_move_mask()
    {
        types::array<u64, 64> knights;
        knights.fill(0);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                int id = id_at_ij(i, j);
                knights[id] = all_knights_masks_at_ij(i, j);
            }
        }
        return knights;
    }

    consteval u64 gen_king_move_mask_at_ij(const int i, const int j)
    {
        u64 mask =
            mask_at_ij_if_legal(i + 1, j + 1) |
             mask_at_ij_if_legal(i + 1, j) |
              mask_at_ij_if_legal(i + 1, j - 1) |
               mask_at_ij_if_legal(i, j + 1) |
                mask_at_ij_if_legal(i, j - 1) |
                 mask_at_ij_if_legal(i - 1, j + 1) |
                  mask_at_ij_if_legal(i - 1, j) |
                   mask_at_ij_if_legal(i - 1, j - 1);
        return mask;
    }

    consteval types::array<u64, 64> gen_king_move_mask()
    {
        types::array<u64, 64> king;
        king.fill(0);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                king[id_at_ij(i, j)] = gen_king_move_mask_at_ij(i, j);
            }
        }
        return king;
    }

    consteval u8 do_jump(const int vertical, const int horizontal, const int start_pos)
    {
        square_walker walker(start_pos);
        int abs_vert = abs(vertical);
        int abs_hor = abs(horizontal);

        if (vertical > 0)
        {
            for (int i = 0; i < abs_vert; i++)
                walker.step_Up();
        }
        if (vertical < 0)
        {
            for (int i = 0; i < abs_vert; i++)
                walker.step_Down();
        }

        if (horizontal > 0)
        {
            for (int i = 0; i < abs_hor; i++)
                walker.step_Right();
        }
        if (horizontal < 0)
        {
            for (int i = 0; i < abs_hor; i++)
                walker.step_Left();
        }

        return walker.is_legal() ? walker.get_id() : (u8)No_Square;
    }

    consteval types::array<KnightNode, 64> gen_knights_squares()
    {
        types::array<KnightNode, 64> knight_squares;

        for (int i = 0; i < 64; i++)
        {
            KnightNode node;

            square_walker walker;
            walker.set_id((int)do_jump(2, 1, i));

            knight_squares[i].moves[0] = walker.get_id();

            walker.set_id((int)do_jump(2, -1, i));

            knight_squares[i].moves[1] = walker.get_id();

            walker.set_id((int)do_jump(1, 2, i));

            knight_squares[i].moves[2] = walker.get_id();

            walker.set_id((int)do_jump(-1, 2, i));

            knight_squares[i].moves[3] = walker.get_id();

            walker.set_id((int)do_jump(-2, 1, i));

            knight_squares[i].moves[4] = walker.get_id();

            walker.set_id((int)do_jump(-2, -1, i));

            knight_squares[i].moves[5] = walker.get_id();

            walker.set_id((int)do_jump(-1, -2, i));

            knight_squares[i].moves[6] = walker.get_id();

            walker.set_id((int)do_jump(1, -2, i));

            knight_squares[i].moves[7] = walker.get_id();
        }

        return knight_squares;
    }

    consteval types::array<PawnNode, 64> gen_white_pawns_attacks()
    {
        types::array<PawnNode, 64> pawns;

        for (int i = 0; i < 64; ++i)
        {
            square_walker walker;
            walker.set_id(i);
            walker.step_LeftUp();
            if (walker.is_legal())
            {
                pawns[i].Left = walker.get_id();
            }
            walker.set_id(i);
            walker.step_RightUp();
            if (walker.is_legal())
            {
                pawns[i].Right = walker.get_id();
            }
        }
        return pawns;
    }

    consteval types::array<PawnNode, 64> gen_black_pawns_attacks()
    {
        types::array<PawnNode, 64> pawns;

        for (int i = 0; i < 64; ++i)
        {
            square_walker walker;
            walker.set_id(i);
            walker.step_LeftDown();
            if (walker.is_legal())
            {
                pawns[i].Left = walker.get_id();
            }
            walker.set_id(i);
            walker.step_RightDown();
            if (walker.is_legal())
            {
                pawns[i].Right = walker.get_id();
            }
        }
        return pawns;
    }

    constexpr types::array<PawnNode, 64> white_pawns_attacks{gen_white_pawns_attacks()};

    constexpr types::array<PawnNode, 64> black_pawns_attacks{gen_black_pawns_attacks()};

    constexpr types::array<BishopNode, 64> bishop_squares_moves{gen_bishop_to_squares()};

    constexpr types::array<KingNode, 64> king_squares_moves{gen_king_squares()};

    constexpr types::array<KnightNode, 64> knight_squares_moves{gen_knights_squares()};


    constexpr types::array<u64, 64> knights_moves{gen_knights_move_mask()};

    constexpr types::array<u64, 64> king_moves{gen_king_move_mask()};

    template <Color clr>
    inline PawnNode colored_pawn_attack_on_id(const int id)
    {
        if constexpr (clr)
            return white_pawns_attacks[id];
        else
            return black_pawns_attacks[id];
    }


}
