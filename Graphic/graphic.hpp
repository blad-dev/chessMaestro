#pragma once
#include "../Board/board.hpp"

namespace chess
{

    class Graphic
    {
    private:
        const types::string separate_line_pattern = "+---+---+---+---+---+---+---+---+\n";
        const types::string white_files_orientation = "  a   b   c   d   e   f   g   h\n";
        const types::string black_files_orientation = "  h   g   f   e   d   c   b   a\n";
        const Board &brd;

        types::map<Piece, char> pieces;

        const int BOARD_HEIGHT = 8;
        const int BOARD_WIDTH = 8;

        void init()
        {
            pieces.insert(std::pair<Piece, char>(W_King, 'K'));   // \U00002654
            pieces.insert(std::pair<Piece, char>(W_Queen, 'Q'));  // \U00002655
            pieces.insert(std::pair<Piece, char>(W_Rook, 'R'));   // \U00002656
            pieces.insert(std::pair<Piece, char>(W_Bishop, 'B')); // \U00002657
            pieces.insert(std::pair<Piece, char>(W_Knight, 'N')); // \U00002658
            pieces.insert(std::pair<Piece, char>(W_Pawn, 'P'));   // \U00002659
            pieces.insert(std::pair<Piece, char>(B_King, 'k'));   // \U0000265A
            pieces.insert(std::pair<Piece, char>(B_Queen, 'q'));  // \U0000265B
            pieces.insert(std::pair<Piece, char>(B_Rook, 'r'));   // \U0000265C
            pieces.insert(std::pair<Piece, char>(B_Bishop, 'b')); // \U0000265D
            pieces.insert(std::pair<Piece, char>(B_Knight, 'n')); // \U0000265E
            pieces.insert(std::pair<Piece, char>(B_Pawn, 'p'));   // \U0000265F

            pieces.insert(std::pair<Piece, char>(No_Piece, ' ')); // white square \U00002B1C
        }

    public:
        Graphic(Board &boar) : brd(boar)
        {
            init();
        }
        Graphic(const Board &boar) : brd(boar)
        {
            init();
        }

        void draw_board(Color turn)
        {
            std::string board = "";
            char piece_to_print = ' ';
            board.append("\t\t\t\t");
            board.append(separate_line_pattern);

            if (turn == White)
            {
                for (int height = 0; height < BOARD_HEIGHT; height++)
                {
                    board.append("\t\t\t\t");
                    for (int width = 0; width < BOARD_WIDTH; width++)
                    {
                        int index = real_board_id(width + BOARD_WIDTH * height);
                        Piece current_piece = brd[index];
                        piece_to_print = pieces.at(current_piece);
                        board.append("| ");
                        board += piece_to_print;
                        board.append(" ");
                    }
                    board.append("|  ").append(std::to_string(BOARD_WIDTH - height)).append("\n");
                    board.append("\t\t\t\t");
                    board.append(separate_line_pattern);
                }
                std::cout << board;
            }
            else
            {
                for (int height = BOARD_HEIGHT - 1; height >= 0; height--)
                {
                    board.append("\t\t\t\t");
                    for (int width = BOARD_WIDTH - 1; width >= 0; width--)
                    {
                        int index = real_board_id(width + BOARD_WIDTH * height);
                        Piece current_piece = brd[index];
                        piece_to_print = pieces.at(current_piece);
                        board.append("| ");
                        board += piece_to_print;
                        board.append(" ");
                    }
                    board.append("|  ").append(std::to_string(BOARD_WIDTH - height)).append("\n");
                    board.append("\t\t\t\t");
                    board.append(separate_line_pattern);
                }
                std::cout << board;
            }

            std::cout << "\t\t\t\t" + (turn ? white_files_orientation : black_files_orientation);
        }
    };

}