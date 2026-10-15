#pragma once

#include <iostream>
#include "movegen.hpp"
#include "fenParser.hpp"
#include "realization.hpp"
#include "../Graphic/graphic.hpp"
#include "../Board/board.hpp"
#include "../ai.hpp"
#include <cstdlib>
#include <ctime>
namespace chess
{
    enum Difficulty : int
    {
        Easy = 4'000,
        Normal = 50'000,
        Hard = 800'000
    };

    class PlayerVComputer : public Realization
    {
    private:
        Hashlist_ref hashlist_ref;
        Difficulty difficlut = Easy;
        Color first_player = White;

        uint64_t rand_limit(int limit)
        {
            int rand_factor = limit / 2;
            return limit + (rand() % rand_factor);
        }

    public:
        PlayerVComputer(Movegen &move, Board &brd, fenParser &pars, Graphic &graph, Hashlist_ref &hashlist) : Realization(move, brd, pars, graph), hashlist_ref(hashlist)
        {
            parser.parse_from_FEN(START_POS, board);
        }

        void set_difficult(Difficulty dif)
        {
            difficlut = dif;
        }

        void set_first_player(Color color){
            first_player = color;
        }
        
        void start_game()
        {
            srand(time(0));
            Movelist_ref list_ref = movegen.get_list();
            const int limit = difficlut;
            Boardlist<103> boardlist;
            hashlist_ref.clear_hash();
            AI bot(board, list_ref);
            Move_full_info move;
            int last_move_id = No_Square;
            while (true)
            {
                
                system("clear");
                graphic.draw_board(board.get_turn());

                types::string move_input = "";

                PositionState state = board.get_turn() ? movegen.gen_all_moves<White>() : movegen.gen_all_moves<Black>();

                if (board.get_turn() != first_player)
                    goto AI_TURN;
                list_ref = movegen.get_list();
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
                    std::cout << "\t\t\t\t\tIt's a draw by fifty moves rule!\n";
                    return;
                }
                if (boardlist.contains_count(board) >= 2)
                {
                    std::cout << "\t\t\t\t\tIt's a draw by three repetitions rule!\n";
                    return;
                }

                boardlist.push(board);
                hashlist_ref.add(board.get_hash());
                if (state == check)
                    std::cout << "You are in a check. Protect your king.\n";
                else if (state == double_check)
                    std::cout << "You are in a double check. Move your king.\n";
                while (true)
                {
                    std::cout << "Last bot move: " << move << "\n";
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
                        {
                            boardlist.clear_list();
                            hashlist_ref.clear_hash();
                        }
                        board.get_turn() ? board.make_move<White>(move) : board.make_move<Black>(move);
                        last_move_id = move.to_square;
                        break;
                    }
                    std::cout << "\t\t\t\t\tThe move you entered is illegal! Try again!\n";
                }
            AI_TURN:
                system("clear");
                graphic.draw_board(static_cast<Color>(!board.get_turn()));

                state = board.get_turn() ? movegen.gen_all_moves<White>() : movegen.gen_all_moves<Black>();
                list_ref = movegen.get_list();
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
                    std::cout << "\t\t\t\t\tIt's a draw by fifty moves rule!\n";
                    return;
                }
                if (boardlist.contains_count(board) >= 2)
                {
                    std::cout << "\t\t\t\t\tIt's a draw by three repetitions rule!\n";
                    return;
                }

                boardlist.push(board);
                hashlist_ref.add(board.get_hash());
                int eval;
                std::tie(move, eval) = bot.best_move_nodes_limit(rand_limit(limit), hashlist_ref, last_move_id);
                board.get_turn() ? board.make_move<White>(move) : board.make_move<Black>(move);
                bot.hash_aging(2);
            }
        }
    };

}