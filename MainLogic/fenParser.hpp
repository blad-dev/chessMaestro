#pragma once
#include "../Board/board.hpp"

namespace chess
{

    struct fenParser
    {
        types::map<char, Piece> fenToMap;

        types::map<Piece, char> mapToFen;

    public:
        fenParser()
        {
            init();
        }

        void parse_from_FEN(const types::string &FEN, Board &brd)
        {
            brd.clear_board();
            std::istringstream FEN_in(FEN);
            types::string command;
            FEN_in >> command;
            parse_position(command, brd);

            FEN_in >> command;
            parse_turn(command, brd);

            FEN_in >> command;
            parse_castle(command, brd);

            FEN_in >> command;
            parse_en_passant(command, brd);

            FEN_in >> command;
            parse_fifty_moves_rule(command, brd);

            FEN_in >> command;
            parse_total_moves(command, brd);

            brd.find_kings_and_check_pawns();
        }

        types::string parse_to_Fen(Board &brd)
        {
            types::string result_fen;
            result_fen.reserve(95);
            result_fen += position_to_fen(brd);
            result_fen += " ";
            result_fen += turn_to_fen(brd.get_turn());
            result_fen += " ";
            result_fen += castle_to_fen(brd);
            result_fen += " ";
            result_fen += en_passant_to_fen(brd.get_en_passant());
            result_fen += " ";
            result_fen += fifty_moves_to_fen(brd.get_fifty_rule());
            result_fen += " ";
            result_fen += total_moves_to_fen(brd.get_total_moves());

            result_fen.shrink_to_fit();
            return result_fen;
        }

    private:
        void parse_position(const types::string &FEN_position, Board &brd)
        {
            int iterator = 0;
            int noPieceIterator = 0;
            for (int i = 0; i < (int)FEN_position.size(); i++)
            {
                char currentChar = FEN_position[i];
                if (currentChar == '/'){
                    if((iterator % 8) != 0)
                        throw std::logic_error("Invalid FEN, invalid pieces positions!");
                    continue;
                }
                    
                if (isalpha(currentChar))
                {
                    if(!fenToMap.contains(currentChar))
                        throw std::logic_error("Invalid FEN, invalid piece symbol!");
                    brd[real_board_id(iterator)] = fenToMap.at(currentChar);
                    iterator++;
                }
                else if(isdigit(currentChar))
                {
                    noPieceIterator = currentChar - '0';
                    for (int j = 0; j < noPieceIterator; j++)
                    {
                        brd[real_board_id(iterator)] = Piece::No_Piece;
                        iterator++;
                    }
                }
                else
                    throw std::logic_error("Invalid FEN, unknown symbol!");
            }
        }

        void parse_turn(const types::string &FEN_turn, Board &brd)
        {
            if(FEN_turn == "w")
                brd.set_turn(White);
            else if(FEN_turn == "b")
                brd.set_turn(Black);
            else   
                throw std::logic_error("Invalid FEN, invalid turn!");
        }

        void parse_castle(const types::string &FEN_castle, Board &brd)
        {
            brd.remove_all_castle<White>();
            brd.remove_all_castle<Black>();
            if(FEN_castle == "-")
                return;
            if(FEN_castle.size() > 4){
                throw std::logic_error("Invalid FEN, invalid castling, too many arguments!");
            }
            for(auto i : FEN_castle){
                switch(i){
                case 'Q':
                    if(brd.can_castle_long<White>())
                        throw std::logic_error("Invalid FEN, double white long castling!");
                    brd.add_castling(White_OOO);
                    break;
                case 'K':
                    if(brd.can_castle_short<White>())
                        throw std::logic_error("Invalid FEN, double white short castling!");
                    brd.add_castling(White_OO);
                    break;
                case 'q':
                    if(brd.can_castle_long<Black>())
                        throw std::logic_error("Invalid FEN, double black long castling!");
                    brd.add_castling(Black_OOO);
                    break;
                case 'k':
                    if(brd.can_castle_short<Black>())
                        throw std::logic_error("Invalid FEN, double black short castling!");
                    brd.add_castling(Black_OO);
                    break;
                default:
                    throw std::logic_error("Invalid FEN, unknown castling symbol!");
                }
            }
        }

        void parse_en_passant(const types::string &FEN_en_passant, Board &brd)
        {
            if (FEN_en_passant == "-")
            {
                return;
            }
            else if(FEN_en_passant.size() != 2){
                throw std::logic_error("Invalid FEN, invalid en passant!");
            }
            else if(!is_legal_file(FEN_en_passant[0])){
                throw std::logic_error("Invalid FEN, invalid en passant file!");
            }
            else if(!is_legal_rank(FEN_en_passant[1])){
                throw std::logic_error("Invalid FEN, invalid en passant rank!");
            }

            int rank = FEN_en_passant[1] - '1';
            int file = FEN_en_passant[0] - 'a';

            brd.set_en_passant(static_cast<Square>(8 * rank + file));
        }

        void parse_fifty_moves_rule(const types::string &FEN_moves, Board &brd)
        {
            int moves_fifty;
            try{
                moves_fifty = stoi(FEN_moves);
            }
            catch(...){
                throw std::logic_error("Invalid FEN, invalid fifty moves number format!");
            }

            if((moves_fifty < 0) || (moves_fifty >= 100))
                throw std::logic_error("Invalid FEN, fifty moves number is out of range!");
            brd.set_fifty_rule(moves_fifty);

        }

        void parse_total_moves(const types::string &FEN_moves, Board &brd)
        {
            int moves_total;
            try{
                moves_total = stoi(FEN_moves);
            }
            catch(...){
                throw std::logic_error("Invalid FEN, invalid total moves number format!");
            }
            if(moves_total < 0)
                throw std::logic_error("Invalid FEN, total moves number is out of range!");

            brd.set_total_moves(moves_total);
        }

        types::string position_to_fen(const Board &brd)
        {
            types::string position_to_fen = "";
            position_to_fen.reserve(85);
            constexpr int board_height = 8;
            constexpr int board_width = 8;

            for (int height = 0; height < board_height; ++height)
            {
                int empty_squares_counter = 0;
                for (int width = 0; width < board_width; ++width)
                {

                    const int real_id = real_board_id(width + board_width * height);
                    char currentPiece = mapToFen.at(brd[real_id]);

                    if (currentPiece == '*')
                    {
                        empty_squares_counter++;
                    }
                    else
                    {
                        if (empty_squares_counter > 0)
                        {
                            position_to_fen += (empty_squares_counter + '0');
                            empty_squares_counter = 0;
                        }
                        position_to_fen += currentPiece;
                    }
                }

                if (empty_squares_counter > 0)
                {
                    position_to_fen += (empty_squares_counter + '0');
                }

                if (height < 7)
                {
                    position_to_fen += '/';
                }
            }

            return position_to_fen;
        }

        types::string turn_to_fen(Color clr)
        {
            if (clr)
            {
                return "w";
            }
            return "b";
        }

        types::string castle_to_fen(const Board &brd)
        {
            if(brd.get_castling() == NO_Castling){
                return "-";
            }
            types::string castl_FEN;
            if (brd.has_castling(White_OO))
            {
                castl_FEN += 'K';
            }
            if (brd.has_castling(White_OOO))
            {
                castl_FEN += 'Q';
            }
            if (brd.has_castling(Black_OO))
            {
                castl_FEN += 'k';
            }
            if (brd.has_castling(Black_OOO))
            {
                castl_FEN += 'q';
            }
            return castl_FEN;
        }

        types::string en_passant_to_fen(Square en_passant)
        {
            if (en_passant == No_Square)
                return "-";

            types::string en_passant_square = "";


            en_passant_square = (en_passant % 8) + 'a';
            en_passant_square +=  + (en_passant / 8) + '1';

            return en_passant_square;
        }

        types::string fifty_moves_to_fen(int move)
        {
            return std::to_string(move);
        }

        types::string total_moves_to_fen(int move)
        {
            return std::to_string(move);
        }

        void init()
        {
            fenToMap.insert(std::pair<char, Piece>('r', Piece::B_Rook));
            fenToMap.insert(std::pair<char, Piece>('n', Piece::B_Knight));
            fenToMap.insert(std::pair<char, Piece>('b', Piece::B_Bishop));
            fenToMap.insert(std::pair<char, Piece>('q', Piece::B_Queen));
            fenToMap.insert(std::pair<char, Piece>('k', Piece::B_King));
            fenToMap.insert(std::pair<char, Piece>('p', Piece::B_Pawn));
            fenToMap.insert(std::pair<char, Piece>('R', Piece::W_Rook));
            fenToMap.insert(std::pair<char, Piece>('N', Piece::W_Knight));
            fenToMap.insert(std::pair<char, Piece>('B', Piece::W_Bishop));
            fenToMap.insert(std::pair<char, Piece>('Q', Piece::W_Queen));
            fenToMap.insert(std::pair<char, Piece>('K', Piece::W_King));
            fenToMap.insert(std::pair<char, Piece>('P', Piece::W_Pawn));

            mapToFen.insert(std::pair<Piece, char>(Piece::B_Rook, 'r'));
            mapToFen.insert(std::pair<Piece, char>(Piece::B_Knight, 'n'));
            mapToFen.insert(std::pair<Piece, char>(Piece::B_Bishop, 'b'));
            mapToFen.insert(std::pair<Piece, char>(Piece::B_Queen, 'q'));
            mapToFen.insert(std::pair<Piece, char>(Piece::B_King, 'k'));
            mapToFen.insert(std::pair<Piece, char>(Piece::B_Pawn, 'p'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_Rook, 'R'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_Knight, 'N'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_Bishop, 'B'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_Queen, 'Q'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_King, 'K'));
            mapToFen.insert(std::pair<Piece, char>(Piece::W_Pawn, 'P'));
            mapToFen.insert(std::pair<Piece, char>(Piece::No_Piece, '*'));
        }
    };

}