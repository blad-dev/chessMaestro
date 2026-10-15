#pragma once
#include <iostream>
#include "movegen.hpp"
#include "fenParser.hpp"
#include "../Graphic/graphic.hpp"
#include "../Board/board.hpp"
#include <fstream>
#include <string>

namespace chess
{

    class Realization
    {
    protected:
        Movegen &movegen;
        Board &board;
        fenParser &parser;
        Graphic &graphic;

        inline const static types::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

        bool is_correct_format(const char &file, const char &rank)
        {
            return tolower(file) >= 'a' && tolower(file) <= 'h' && rank >= '1' && rank <= '8';
        }

        bool find_move(const Move_full_info &move, Movelist_ref &list)
        {
            for (auto i = list.begin; i != list.end; ++i)
            {
                if (move == *i)
                {
                    return true;
                }
            }
            return false;
        }

        Square parse_str_to_sq(const char file, const char rank)
        {
            return static_cast<Square>((file - 'a') + (rank - '1') * 8);
        }


        template <Color clr>
        Move_full_info pars_to_move_full_info(types::string &move)
        {
            u8 from = No_Square, to = No_Square;

            if (move == "O-O")
            {
                from = castling_king_from_square<clr>();
                to = short_castling_king_to_square<clr>();
                return {from, to, No_promotion, SP_castling};
            }

            if (move == "O-O-O")
            {
                from = castling_king_from_square<clr>();
                to = long_castling_king_to_square<clr>();
                return {from, to, No_promotion, SP_castling};
            }

            if (is_correct_format(move[0], move[1]) && is_correct_format(move[2], move[3]))
            {
                from = parse_str_to_sq(move[0], move[1]);
                to = parse_str_to_sq(move[2], move[3]);
            }

            if (move.size() == 5)
            {
                if (!movegen.is_promotion_square<clr>(to))
                    return {};
                Promotion promotion = No_promotion;
                switch (move[4])
                {
                case 'q':
                    promotion = promote_to_queen;
                    break;
                case 'r':
                    promotion = promote_to_rook;
                    break;
                case 'b':
                    promotion = promote_to_bishop;
                    break;
                case 'n':
                    promotion = promote_to_knight;
                    break;
                default:
                    return {};
                }
                return {from, to, promotion, SP_Promotion};
            }

            if (board[from] == Pawn_with_color<clr>())
            {
                if (to == board.get_en_passant())
                {
                    return {from, to, No_promotion, SP_en_passant};
                }
            }

            return {from, to, No_promotion, No_special};
        }

        void save_position(const std::string &filename, const std::string &text)
        {
            std::ofstream file(filename);
            if (file.is_open())
            {
                file << text;
                file.close();
            }
            else
            {
                throw std::logic_error("File error 404");
            }
        }

        std::string read_position_from_file(const std::string &filename)
        {
            std::ifstream file(filename);
            std::string content;
            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    content += line;
                }
                file.close();
            }
            else
            {
                return START_POS;
            }
            return content;
        }

    public:
        Realization(Movegen &move, Board &brd, fenParser &pars, Graphic &graph) : movegen(move), board(brd), parser(pars), graphic(graph) {}

        void set_new_position(const std::string &pos)
        {
            parser.parse_from_FEN(pos, board);
        }

        void configure_last_position()
        {
            std::string fen = read_position_from_file("pos.txt");
            if (fen.size() == 0)
            {
                parser.parse_from_FEN(START_POS, board);
                return;
            }
            parser.parse_from_FEN(fen, board);
        }

        virtual void start_game() = 0;
    };

}