#pragma once
#include <iostream>
#include "movegen.hpp"
#include "fenParser.hpp"
#include "realization.hpp"
#include "../Graphic/graphic.hpp"
#include "../Board/board.hpp"
#include <fstream>
#include <string>

namespace chess
{
    class PlayerVPlayer : public Realization
    {

    public:
        PlayerVPlayer(Movegen &move, Board &brd, fenParser &pars, Graphic &graph) : Realization(move, brd, pars, graph)
        {
            parser.parse_from_FEN(START_POS, board);
        }

        void start_game()
        {
            std::string last_move = "";
            types::string move_input = "";
            Boardlist<103> boardlist(board.get_fifty_rule());

            while (true)
            {
                system("clear");
                graphic.draw_board(board.get_turn());
                
                std::cout << "Last move: " << move_input << "\n";

                PositionState state = board.get_turn() ? movegen.gen_all_moves<White>() : movegen.gen_all_moves<Black>();
                Movelist_ref list_ref = movegen.get_list();
                if (list_ref.no_moves())
                {
                    if (state != quite)
                        std::cout << "\t\t\t\t\tIt's checkmate, " << (board.get_turn() ? "Black" : "White") << " player won!\n";
                    else
                        std::cout << "\t\t\t\t\tIt's stalemate, it's a draw!\n";
                    return;
                }
                if (boardlist.size() >= 100)
                {
                    std::cout << "It's a draw by fifty moves rule!\n";
                    return;
                }
                if (boardlist.contains_count(board) >= 2)
                {
                    std::cout << "It's a draw by three repetitions rule!\n";
                    return;
                }
                boardlist.push(board);
                if (state == check)
                    std::cout << "You are in a check. Protect your king.\n";
                else if (state == double_check)
                    std::cout << "You are in a double check. Move your king.\n";
                while (true)
                {

                    std::cout << (board.get_turn() ? "White move:\n" : "Black move:\n");
                    std::cin >> move_input;

                    if (move_input == "exit")
                    {
                        save_position("pos.txt", parser.parse_to_Fen(board));
                        return;
                    }

                    if (move_input == "surr")
                    {
                        std::cout << (board.get_turn() ? "Black won. White surrendered." : "White won. Black surrendered.");
                        return;
                    }

                    Move_full_info move = board.get_turn() ? pars_to_move_full_info<White>(move_input) : pars_to_move_full_info<Black>(move_input);

                    if (find_move(move, list_ref))
                    {
                        if ((board[move.from_square] == W_Pawn) || (board[move.from_square] == B_Pawn) || (board[move.to_square] != No_Piece))
                            boardlist.clear_list();
                        board.get_turn() ? board.make_move<White>(move) : board.make_move<Black>(move);
                        break;
                    }
                    std::cout << "\t\t\t\t\tThe move you entered is illegal! Try again!\n";
                }
            }
        }
    };

}