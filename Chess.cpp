#include <iostream>
#include "MainLogic/fenParser.hpp"
#include "MainLogic/PlayerVPlayer.hpp"
#include "MainLogic/PlayerVComputer.hpp"
#include "Graphic/graphic.hpp"
#include "Board/board.hpp"
#include "MainLogic/movegen.hpp"
#include "ChessGame.hpp"
#include <fstream>
#include "ai.hpp"
#include "tables.hpp"
//#define WINDOWS
#ifdef WINDOWS
#include <windows.h>
#endif
#define CLEAR system("clear");

using namespace chess;
using namespace std;



void Rules()
{
    std::cout << "\n\n\t\t\t\t\t     ~Rules~\n\n\n";
    std::cout << "\t\t    Chess is a two-player board game using a chessboard.\n"
              << "\t\tOne of them play for White and another for Black.\n"
              << "\t\tWhite makes first move, then players alternate moves.\n"
              << "\t\t    There are six pieces: Pawn, Knight, Rook, Bishop, Queen and King,\n\t\tthe main figure in game.\n"
              << "\t\t8 Pawns, 2 Knight, 2 Rook, 2 Bishop, 1 Queen and 1 King for each player.\n"
              << "\t\t    In our game we use upper case to mark White and lower for Black:\n"
              << "\t\tPawn - P, p; Knight - N, n; Rook - R, r; Bishop - B, b; Queen - Q, q; King - K, k.\n\n";

    std::cout << "\t\t\tThe chessboard and order of figures look like this:\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t| r | n | b | q | k | b | n | r |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t| p | p | p | p | p | p | p | p |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t|   |   |   |   |   |   |   |   |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t|   |   |   |   |   |   |   |   |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t|   |   |   |   |   |   |   |   |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t|   |   |   |   |   |   |   |   |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t|   |   |   |   |   |   |   |   |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t| P | P | P | P | P | P | P | P |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n"
              << "\t\t\t\t| R | N | B | Q | K | B | G | R |\n"
              << "\t\t\t\t+---+---+---+---+---+---+---+---+\n";

    std::cout
        << "\t\t\t\t\tMoves of figures\n"
        << "\tFigures can move to a vacant squares and all beat opponent's figures, expect King.\n\n";

    std::cout << "\t\t\tQueen\t\t\t\t\t\tBishop\n"
              << "\t    1. moves any number of vacant squares        |     1. moves any number of vacant squares\n"
              << "\t horizontally, vertically, or diagonally.        | diagonally.\n\n";

    std::cout << "\t\t\tKnight\t\t\t\t\t\tRook\n"
              << "\t    1. moves to the nearest square not on the    |    1. moves any number of vacant squares\n"
              << "\t same color in an `L` pattern in all directions. | horizontally or vertically.\n"
              << "\t    2. jumps over figures to the new location,   |    2.is moved when castling.\n"
              << "\t unlike the rest ones.\n\n";

    std::cout << "\t\t\tPawn\t\t\t\t\t\tKing\n"
              << "\t    1. moves straight forward one square and     |    1. moves exactly one square horizontally,\n"
              << "\t two square from initial position.               | vertically, or diagonally.\n"
              << "\t    2. cannot move backwards.                    |    2. take part in castling.\n"
              << "\t    3. capture an enemy piece on either          |    3. can't move to a square, where can be\n"
              << "\t of the two squares diagonally in front of it.   | in check\n"
              << "\t    4. has special capture `En passant`.         |\n"
              << "\t    5. can be converted.                         |\n\n\n";

    std::cout << "\t\t\t\t\tSpecial moves and states\n\n";

    std::cout << "\t\t\t\t\t       Simple move\n"
              << "\t\t   In our game we allows you to make any different move you want to.\n"
              << "\t\tBut be aware, our core engine is checking every your move, so you should\n"
              << "\t\tnot make mistakes, because you will enter your moves infinetly. To make\n"
              << "\t\tsimple move just type a starting square and destination square.\n"
              << "\t\tFor example, a2a4, d7d5, f3g5 and so on.\n";

    std::cout << "\t\t\t\t\t       Check\n"
              << "\t\t   A check is a condition when a player's king is under threat of\n"
              << "\t\tcapture on their opponent's next turn. A king so threatened is said\n"
              << "\t\tto be in check. You must get out of check, if possible, else the game\n"
              << "\t\tends in checkmate and the player loses. Players cannot make any move\n"
              << "\t\tthat puts their own king in check.\n";

    std::cout << "\t\t\t\t\t    Promotion\n"
              << "\t\t    It is a rule that allow a pawn that reaches a square on the back\n"
              << "\t\trank of the opponent be replaced by Bishop, Knight, Rook, or Queen\n"
              << "\t\tof the same color. Just enter like simple move, but in the end of a move \n"
              << "\t\tput b - for bishop, q - for queen, n - for knight and r - for rook\n\n";

    std::cout << "\t\t\t\t\t    En passant\n"
              << "\t\t   It is a special pawn capture that can only occur immediately after a\n"
              << "\t\tpawn makes double-step move from its starting square, and it could have\n"
              << "\t\tbeen captured by an enemy pawn had it advanced only one square.\n\n";

    std::cout << "\t\t\t\t\t    Castling\n"
              << "\t\t   It is a move in the game of chess involving a player's king and either\n"
              << "\t\tof the player's original rooks. It is the only move in chess in which a\n"
              << "\t\tplayer moves two pieces in the same move. You can use O-O for short\n"
              << "\t\tcastling or O-O-O for long castling\n";
}

int main()
{
    fenParser fen_parser;
    Board brd;
    Graphic graphic(brd);
    Movelist<10000> list;
    Hashlist<1000> hashlist;
    Hashlist_ref hashlist_ref(hashlist);
    Movelist_ref list_ref(list);
    Movegen gen(brd, list_ref);
    const types::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
#ifdef WINDOWS
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r); // stores the console's current dimensions

    MoveWindow(console, r.left, r.top, 920, 500, true);
#endif

    cout << "\t\t\t\t=======Welcome to chess game=======\n";

    while (true)
    {
        std::string fen = "";
        PlayerVPlayer pvp(gen, brd, fen_parser, graphic);
        PlayerVComputer pvc(gen, brd, fen_parser, graphic, hashlist_ref);

        std::string answer = "";
        cout << "\n\t\t\t\t\tPlease choose option\n";
        cout << "\t\t\t\t1.Rules 2.PlayerVPlayer 4.Exit\n";
        cout << "\t\t\t\t\t3.PlayerVComp\n";
        cin >> answer;
        CLEAR
        if (answer == "1")
        {
            Rules();
        }

        if (answer == "2")
        {
            ChessGame game(pvp);
            CLEAR
            cout << "\t\t\tOur realization using a FEN notation to configure start pos.\n";
            cout << "\t\t\tType \"start\" to configure from start position\n";
            cout << "\t\t\tEnter FEN notation - to configure a chessboard with special position\n";
            cout << "\t\t\tType \"lp\" to load last saved position\n";
            cout << "\t\t\tType \"exit\" during the game, and the game will stop and automaticaly save position\n";
            cout << "\t\t\tType \"surr\" during the game to surrender\n";

            getchar();
            getline(cin, fen);

            if (fen == "lp")
            {
                pvp.configure_last_position();
                game.start();
            }

            else if (fen == "start")
            {
                CLEAR
                game.start();
            }

            else
            {
                CLEAR
                try
                {
                    pvc.set_new_position(fen);
                }
                catch (std::logic_error& error)
                {
                    CLEAR
                    cout << error.what();
                    continue;
                }
                int not_moving_king_id = brd.get_turn() ? brd.get_right_king_position<Black>() : brd.get_right_king_position<White>();
                bool is_under_check = (brd.get_turn() ? gen.is_id_under_any_check<Black>(not_moving_king_id) : gen.is_id_under_any_check<White>(not_moving_king_id));
                if (is_under_check)
                {
                    cout << "FEN is invalid, the king is under check!\n";
                    continue;
                }
                game.start();
            }
            cout << "\n\t\t\t\tThanks for playing\n";
        }

        if (answer == "3")
        {
            ChessGame game(pvc);
            CLEAR
            cout << "\t\t\tOur realization using a FEN notation to configure start pos.\n";
            cout << "\t\t\tType \"start\" to configure from start position\n";
            cout << "\t\t\tEnter FEN notation - to configure a chessboard with special position\n";
            cout << "\t\t\tType \"lp\" to load last saved position\n";
            cout << "\t\t\tType \"exit\" during the game, and the game will stop and automaticaly save position\n";
            cout << "\t\t\tType \"surr\" during the game to surrender\n";

            getchar();
            getline(cin, fen, '\n');

            if (fen == "lp")
            {
                pvc.configure_last_position();
                pvc.set_first_player(brd.get_turn());
                game.start();
                cout << "\n\t\t\t\tThanks for playing\n";
                continue;
            }
            if (fen != "start")
            {
                try
                {
                    pvc.set_new_position(fen);
                }
                catch (std::logic_error& error)
                {
                    CLEAR
                    cout << error.what();
                    continue;
                }
                int not_moving_king_id = brd.get_turn() ? brd.get_right_king_position<Black>() : brd.get_right_king_position<White>();
                bool is_under_check = (brd.get_turn() ? gen.is_id_under_any_check<Black>(not_moving_king_id) : gen.is_id_under_any_check<White>(not_moving_king_id));
                if (is_under_check)
                {
                    cout << "FEN is invalid, the king is under check!\n";
                    continue;
                }
            }
            else
            {
                pvc.set_new_position(START_POS);
            }

            string diff = "";
            string color = "";

            while ((diff != "1") && (diff != "2") && (diff != "3"))
            {
                cout << "\n\t\t\tPlease choose difficult for AI bot\n";
                cout << "\t\t\t1.Easy       2.Medium       3.Hard\n";
                cin >> diff;

                if (diff == "1")
                {
                    pvc.set_difficult(Easy);
                }
                else if (diff == "2")
                {
                    pvc.set_difficult(Normal);
                }
                else if (diff == "3")
                {
                    pvc.set_difficult(Hard);
                }
            }
            Color site;
            while ((color != "1") && (color != "2"))
            {
                cout << "\t\t\tPlease choose which color you wont to play\n";
                cout << "\t\t\t1.White                            2.Black\n";
                cin >> color;

                site = static_cast<Color>(color == "1");
                pvc.set_first_player(site);
            }

            CLEAR
            game.start();
            cout << "\n\t\t\t\tThanks for playing\n";
        }
        if (answer == "4")
        {
            CLEAR
            cout << "\t\t\t\tThank you so mush for choosing our game! See you soon!\n";
            break;
        }
    }

    return 0;
}