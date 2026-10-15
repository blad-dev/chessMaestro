#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <tuple>

#ifdef MAESTRO_DEBUG
#include <debug/string>
#include <debug/array>
#include <debug/map>
#else
#include <string>
#include <array>
#include <map>
#endif

namespace chess
{
    namespace types
    {
#ifdef MAESTRO_DEBUG
        using string = __gnu_debug::string;
        template <typename _Tp, std::size_t _Nm>
        using array = std::__debug::array<_Tp, _Nm>;
        template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
                  typename _Allocator = std::allocator<std::pair<const _Key, _Tp>>>
        using map = std::__debug::map<_Key, _Tp, _Compare, _Allocator>;
#else
        using string = std::string;
        template <typename _Tp, std::size_t _Nm>
        using array = std::array<_Tp, _Nm>;
        template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
                  typename _Allocator = std::allocator<std::pair<const _Key, _Tp>>>
        using map = std::map<_Key, _Tp, _Compare, _Allocator>;
#endif
    }

    using uint = unsigned int;
    using u8 = uint8_t;
    using i8 = int8_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using Move = uint16_t;

    enum Color : bool
    {
        White = true,
        Black = false
    };

    enum Direction : int
    {
        Up = 8,
        Down = -8,
        Left = -1,
        Right = 1,
        LeftUp = 7,
        RightUp = 9,
        LeftDown = -9,
        RightDown = -7,

        No_Direction = 0
    };

    constexpr Direction operator-(const Direction direction)
    {
        return static_cast<Direction>(-static_cast<int>(direction));
    }

    enum PositionState : int
    {
        quite = 0,
        check = 1,
        double_check = 2
    };

    enum CastlingRights : int
    {
        NO_Castling = 0,
        White_OO = 1,
        White_OOO = 2,
        Black_OO = 4,
        Black_OOO = 8,

        White_OO_OOO = White_OO | White_OOO,
        Black_OO_OOO = Black_OO | Black_OOO
    };

    constexpr CastlingRights &operator|=(CastlingRights &rights, const CastlingRights &add)
    {
        rights = static_cast<CastlingRights>(rights | add);
        return rights;
    }
    constexpr CastlingRights &operator&=(CastlingRights &rights, const CastlingRights &add)
    {
        rights = static_cast<CastlingRights>(rights & add);
        return rights;
    }

    constexpr CastlingRights operator|(const CastlingRights &right, const CastlingRights &left)
    {
        return static_cast<CastlingRights>(static_cast<int>(right) | static_cast<int>(left));
    }

    constexpr int real_board_id(const int id)
    {
        return ((7 - (id / 8)) * 8) + (id % 8);
    }

    enum Piece : int
    {
        W_Pawn = 0,
        W_Knight = 1,
        W_Bishop = 2,
        W_Rook = 3,
        W_Queen = 4,
        W_King = 5,

        B_Pawn = 6,
        B_Knight = 7,
        B_Bishop = 8,
        B_Rook = 9,
        B_Queen = 10,
        B_King = 11,

        No_Piece = 12
    };

    enum Square : int
    {
        SQ_A1 = 0,
        SQ_B1,
        SQ_C1,
        SQ_D1,
        SQ_E1,
        SQ_F1,
        SQ_G1,
        SQ_H1,
        SQ_A2,
        SQ_B2,
        SQ_C2,
        SQ_D2,
        SQ_E2,
        SQ_F2,
        SQ_G2,
        SQ_H2,
        SQ_A3,
        SQ_B3,
        SQ_C3,
        SQ_D3,
        SQ_E3,
        SQ_F3,
        SQ_G3,
        SQ_H3,
        SQ_A4,
        SQ_B4,
        SQ_C4,
        SQ_D4,
        SQ_E4,
        SQ_F4,
        SQ_G4,
        SQ_H4,
        SQ_A5,
        SQ_B5,
        SQ_C5,
        SQ_D5,
        SQ_E5,
        SQ_F5,
        SQ_G5,
        SQ_H5,
        SQ_A6,
        SQ_B6,
        SQ_C6,
        SQ_D6,
        SQ_E6,
        SQ_F6,
        SQ_G6,
        SQ_H6,
        SQ_A7,
        SQ_B7,
        SQ_C7,
        SQ_D7,
        SQ_E7,
        SQ_F7,
        SQ_G7,
        SQ_H7,
        SQ_A8,
        SQ_B8,
        SQ_C8,
        SQ_D8,
        SQ_E8,
        SQ_F8,
        SQ_G8,
        SQ_H8,

        No_Square
    };

    enum Promotion : u8
    {
        No_promotion = 0,
        promote_to_knight = 0,
        promote_to_bishop,
        promote_to_rook,
        promote_to_queen
    };

    enum Special : u8
    {
        No_special = 0,
        SP_Promotion,
        SP_en_passant,
        SP_castling
    };

    template <Color color>
    consteval Square long_castling_rook_from_square()
    {
        if constexpr (color)
            return SQ_A1;
        else
            return SQ_A8;
    }

    template <Color color>
    consteval Square short_castling_rook_from_square()
    {
        if constexpr (color)
            return SQ_H1;
        else
            return SQ_H8;
    }

    template <Color color>
    consteval Square long_castling_rook_to_square()
    {
        if constexpr (color)
            return SQ_D1;
        else
            return SQ_D8;
    }

    template <Color color>
    consteval Square short_castling_rook_to_square()
    {
        if constexpr (color)
            return SQ_F1;
        else
            return SQ_F8;
    }

    template <Color color>
    consteval Square castling_king_from_square()
    {
        if constexpr (color)
            return SQ_E1;
        else
            return SQ_E8;
    }

    template <Color color>
    consteval Square short_castling_king_to_square()
    {
        if constexpr (color)
            return SQ_G1;
        else
            return SQ_G8;
    }

    template <Color color>
    consteval Square long_castling_king_to_square()
    {
        if constexpr (color)
            return SQ_C1;
        else
            return SQ_C8;
    }

    template <Color color>
    consteval Direction pawn_move_dir()
    {
        if constexpr (color)
            return Up;
        else
            return Down;
    }

    constexpr uint64_t bit_at(const uint i)
    {
        return static_cast<uint64_t>(1) << i;
    }

    constexpr bool contains(const auto source, const auto mask)
    {
        return (source & mask);
    }

    inline types::string square_to_str(const Square sq)
    {
        types::string str;
        str.reserve(2);
        str.push_back(static_cast<char>((sq % 8) + 'a'));
        str.push_back(static_cast<char>((sq / 8) + '1'));
        return str;
    }

    consteval Color change_color(Color color)
    {
        return static_cast<Color>(!color);
    }

    template <Color color>
    consteval Piece Pawn_with_color()
    {
        if constexpr (color)
        {
            return W_Pawn;
        }
        else
        {
            return B_Pawn;
        }
    }

    template <Color color>
    consteval Piece Knight_with_color()
    {
        if constexpr (color)
        {
            return W_Knight;
        }
        else
        {
            return B_Knight;
        }
    }

    template <Color color>
    consteval Piece Bishop_with_color()
    {
        if constexpr (color)
        {
            return W_Bishop;
        }
        else
        {
            return B_Bishop;
        }
    }

    template <Color color>
    consteval Piece Rook_with_color()
    {
        if constexpr (color)
        {
            return W_Rook;
        }
        else
        {
            return B_Rook;
        }
    }

    template <Color color>
    consteval Piece Queen_with_color()
    {
        if constexpr (color)
        {
            return W_Queen;
        }
        else
        {
            return B_Queen;
        }
    }

    template <Color color>
    consteval Piece King_with_color()
    {
        if constexpr (color)
        {
            return W_King;
        }
        else
        {
            return B_King;
        }
    }

    template <Color color>
    consteval int pawn_double_move_distance()
    {
        if constexpr (color)
            return Direction::Up * 2;
        else
            return Direction::Down * 2;
    }

    template <Color color>
    consteval int pawn_en_passant_direction_from()
    {
        if constexpr (color)
            return Direction::Up;
        else
            return Direction::Down;
    }

    template <Color color>
    consteval int pawn_en_passant_direction_to()
    {
        if constexpr (color)
            return Direction::Down;
        else
            return Direction::Up;
    }

    inline Color piece_color(const Piece piece)
    {
        return static_cast<Color>(piece < 6);
    }

    inline bool is_legal_pawn_square(int i)
    {
        return (i >= SQ_A2) && (i <= SQ_H7);
    }

    inline bool is_legal_rank(char rank)
    {
        return (rank >= '1') && (rank <= '8');
    }
    inline bool is_legal_file(char file)
    {
        return (file >= 'a') && (file <= 'h');
    }

    struct Move_full_info
    {
        u8 from_square, to_square;
        Promotion promotion;
        Special special;

        constexpr Move_full_info() : from_square(No_Square), to_square(No_Square), promotion(No_promotion), special(No_special) {}
        constexpr Move_full_info(u8 _from_square, u8 _to_square, Promotion _promotion, Special _special) noexcept : from_square(_from_square),
                                                                                                                    to_square(_to_square),
                                                                                                                    promotion(_promotion),
                                                                                                                    special(_special) {}
        constexpr Move_full_info(Move move)
        {
            from_square = static_cast<u8>(move & 0b111'111);
            move >>= 6;
            to_square = static_cast<u8>(move & 0b111'111);
            move >>= 6;
            promotion = static_cast<Promotion>(move & 0b11);
            move >>= 2;
            special = static_cast<Special>(move);
        }

        constexpr Move to_move() const noexcept
        {
            Move move = static_cast<Move>(special);
            move <<= 2;
            move |= static_cast<Move>(promotion);
            move <<= 6;
            move |= static_cast<Move>(to_square);
            move <<= 6;
            move |= static_cast<Move>(from_square);
            return move;
        }

        friend constexpr bool operator<(const Move_full_info &left, const Move_full_info &right)
        {
            return false;
        }

        friend constexpr bool operator==(const Move_full_info &left, const Move_full_info &right)
        {
            return (left.from_square == right.from_square) && (left.to_square == right.to_square) && (left.promotion == right.promotion) && (left.special == right.special);
        }

        constexpr bool is_no_move()
        {
            return from_square == No_Square;
        }

        friend std::ostream &operator<<(std::ostream &out, const Move_full_info &move)
        {

            if (move == Move_full_info{})
            {
                out << "No previous move.";
                return out;
            }

            if (move.special == SP_castling)
            {
                switch (move.to_square)
                {
                case short_castling_king_to_square<White>():
                case short_castling_king_to_square<Black>():
                    out << "O-O";
                    return out;
                case long_castling_king_to_square<White>():
                case long_castling_king_to_square<Black>():
                    out << "O-O-O";
                    return out;
                }
            }

            out << square_to_str(static_cast<Square>(move.from_square));
            out << square_to_str(static_cast<Square>(move.to_square));
            if (move.special == SP_Promotion)
                switch (move.promotion)
                {
                case promote_to_knight:
                    out << 'k';
                    break;
                case promote_to_bishop:
                    out << 'b';
                    break;
                case promote_to_rook:
                    out << 'r';
                    break;
                case promote_to_queen:
                    out << 'q';
                }
            return out;
        }
    };

    template <uint _size>
    struct Movelist
    {
    public:
        types::array<Move_full_info, _size> arr;
        int count;
        constexpr Movelist() : count(0) {}
        constexpr void push(const Move_full_info &move)
        {
            arr[count++] = move;
        }
        constexpr Move_full_info peek()
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
    };

    struct Accumulator
    {
        u8 castling_rights = 0, white_king_position = 0, black_king_position = 0, en_passant = 0, piece_to_revive = 0, fifty_moves = 0;
    };

    struct Movelist_ref
    {
    public:
        Move_full_info *begin;
        Move_full_info *end;
        constexpr Movelist_ref(Move_full_info *arr) : begin(arr), end(arr) {}
        template <uint _size>
        constexpr Movelist_ref(Movelist<_size> &_list) : begin(_list.arr.begin()), end(_list.arr.begin()) {}

        constexpr const Move_full_info &operator[](int i) const
        {
            return begin[i];
        }

        constexpr Move_full_info &operator[](int i)
        {
            return begin[i];
        }

        constexpr void add(const Move_full_info &move)
        {
            *(end++) = move;
        }
        constexpr Move_full_info peek() const
        {
            return *end;
        }
        constexpr void pop()
        {
            --end;
        }
        constexpr void pop(Move_full_info &move)
        {
            --end;
            move = *end;
        }
        // constexpr void
        constexpr bool no_moves() const
        {
            return begin == end;
        }
        constexpr Movelist_ref get_ref()
        {
            return Movelist_ref(end);
        }

        constexpr int count()
        {
            return end - begin;
        }

        constexpr void clear_moves()
        {
            end = begin;
        }
    };

    template <uint _size>
    struct Hashlist
    {
    public:
        types::array<u64, _size> arr;
        int count;
        constexpr Hashlist() : count(0) {}
        constexpr Hashlist(uint size) : count(size) {}
        constexpr void push(const u64 &hash)
        {
            arr[count++] = hash;
        }
        constexpr u64 peek()
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
    };

    struct Hashlist_ref
    {
    public:
        u64 *begin;
        u64 *end;
        constexpr Hashlist_ref() {}
        constexpr Hashlist_ref(u64 *arr) : begin(arr), end(arr) {}
        template <uint _size>
        constexpr Hashlist_ref(Hashlist<_size> &_list) : begin(_list.arr.begin()), end(_list.arr.begin()) {}

        constexpr const u64 &operator[](int i) const
        {
            return begin[i];
        }

        constexpr u64 &operator[](int i)
        {
            return begin[i];
        }

        constexpr void add(const u64 &hash)
        {
            *(end++) = hash;
        }
        constexpr u64 peek() const
        {
            return *end;
        }
        constexpr void pop()
        {
            --end;
        }
        // constexpr void
        constexpr bool no_hash() const
        {
            return begin == end;
        }
        constexpr Hashlist_ref get_ref_from_end()
        {
            return Hashlist_ref(end);
        }

        constexpr int count()
        {
            return end - begin;
        }

        constexpr void clear_hash()
        {
            end = begin;
        }

        constexpr int contains_count(const u64 hash)
        {
            int count = 0;
            for (auto *i = begin; i != end; ++i)
            {
                count += (hash == *i);
            }
            return count;
        }
    };

    struct evaluation
    {
        int mg_eval, eg_eval;

        constexpr evaluation() : mg_eval(0), eg_eval(0) {}
        constexpr evaluation(int _mg_eval, int _eg_eval) : mg_eval(_mg_eval), eg_eval(_eg_eval) {}

        constexpr int tapered_eval(const int mg_phase) const
        {
            return (mg_phase * mg_eval + (24 - mg_phase) * eg_eval) / 24;
        }
        constexpr void operator+=(const evaluation &another_eval)
        {
            mg_eval += another_eval.mg_eval;
            eg_eval += another_eval.eg_eval;
        }
        constexpr void operator-=(const evaluation &another_eval)
        {
            mg_eval -= another_eval.mg_eval;
            eg_eval -= another_eval.eg_eval;
        }
        constexpr evaluation operator-() const
        {
            return {-mg_eval, -eg_eval};
        }
        constexpr void operator=(const evaluation &another_eval)
        {
            mg_eval = another_eval.mg_eval;
            eg_eval = another_eval.eg_eval;
        }
        friend constexpr evaluation operator+(const evaluation &left, const evaluation &right)
        {
            return {left.mg_eval + right.mg_eval, left.eg_eval + right.eg_eval};
        }
        friend constexpr evaluation operator-(const evaluation &left, const evaluation &right)
        {
            return {left.mg_eval - right.mg_eval, left.eg_eval - right.eg_eval};
        }
    };
}