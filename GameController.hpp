#pragma once

#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include "Board/board.hpp"
#include "MainLogic/movegen.hpp"
#include "MainLogic/fenParser.hpp"

namespace chess {

enum class GameStatus {
    Active,
    Checkmate,
    Stalemate,
    DrawFiftyMoves,
    DrawThreefoldRepetition,
    Resigned
};

class GameController {
private:
    Board board;
    fenParser parser;
    Movelist<10000> movelist;
    Movelist_ref movelist_ref;
    Movegen movegen;
    Boardlist<103> boardlist;
    Hashlist<1000> hashlist;
    Hashlist_ref hashlist_ref;

    GameStatus status = GameStatus::Active;
    PositionState position_state = quite;
    Square selected_square = No_Square;
    Move_full_info last_move{};
    Color resigned_player = White;

    inline const static std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    void regenerate_all_legal_moves() {
        if (board.get_turn() == White) {
            position_state = movegen.gen_all_moves<White>();
        } else {
            position_state = movegen.gen_all_moves<Black>();
        }
        movelist_ref = movegen.get_list();
    }

    void evaluate_game_status() {
        if (status == GameStatus::Resigned) {
            return;
        }

        regenerate_all_legal_moves();

        if (movelist_ref.no_moves()) {
            if (position_state != quite) {
                status = GameStatus::Checkmate;
            } else {
                status = GameStatus::Stalemate;
            }
            return;
        }

        if (boardlist.size() >= 100) {
            status = GameStatus::DrawFiftyMoves;
            return;
        }

        if (boardlist.contains_count(board) >= 2) {
            status = GameStatus::DrawThreefoldRepetition;
            return;
        }

        status = GameStatus::Active;
    }

public:
    GameController() 
        : movelist_ref(movelist),
          movegen(board, movelist_ref),
          hashlist_ref(hashlist) {
        reset_to_start();
    }

    void reset_to_start() {
        load_fen(START_POS);
    }

    bool load_fen(const std::string& fen) {
        try {
            board.clear_board();
            parser.parse_from_FEN(fen, board);
        } catch (...) {
            return false;
        }

        boardlist.clear_list();
        hashlist_ref.clear_hash();
        boardlist.push(board);
        hashlist_ref.add(board.get_hash());

        selected_square = No_Square;
        last_move = Move_full_info{};
        status = GameStatus::Active;

        evaluate_game_status();
        return true;
    }

    std::string get_fen() {
        return parser.parse_to_Fen(board);
    }

    bool save_to_file(const std::string& filename = "pos.txt") {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << get_fen();
        file.close();
        return true;
    }

    bool load_from_file(const std::string& filename = "pos.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return load_fen(START_POS);
        }
        std::string fen;
        std::string line;
        while (std::getline(file, line)) {
            fen += line;
        }
        file.close();
        if (fen.empty()) {
            return load_fen(START_POS);
        }
        return load_fen(fen);
    }

    void resign(Color player) {
        status = GameStatus::Resigned;
        resigned_player = player;
    }

    Color get_resigned_player() const {
        return resigned_player;
    }

    GameStatus get_status() const {
        return status;
    }

    PositionState get_position_state() const {
        return position_state;
    }

    bool is_in_check() const {
        return position_state != quite;
    }

    Color get_turn() const {
        return board.get_turn();
    }

    Piece get_piece_at(Square sq) const {
        if (sq >= 64) return No_Piece;
        return board[sq];
    }

    Piece get_piece_at(int row, int col) const {
        if (row < 0 || row > 7 || col < 0 || col > 7) return No_Piece;
        Square sq = static_cast<Square>((7 - row) * 8 + col);
        return get_piece_at(sq);
    }

    Square get_king_square(Color color) {
        if (color == White) {
            return board.get_right_king_position<White>();
        } else {
            return board.get_right_king_position<Black>();
        }
    }

    Square get_selected_square() const {
        return selected_square;
    }

    Move_full_info get_last_move() const {
        return last_move;
    }

    const Board& get_board() const {
        return board;
    }

    Board& get_board_mutable() {
        return board;
    }

    Hashlist_ref& get_hashlist_ref() {
        return hashlist_ref;
    }

    std::vector<Move_full_info> get_legal_moves_from(Square from) {
        std::vector<Move_full_info> moves;
        if (from >= 64 || status != GameStatus::Active) return moves;

        regenerate_all_legal_moves();
        for (const Move_full_info* it = movelist_ref.begin; it != movelist_ref.end; ++it) {
            if (it->from_square == from) {
                moves.push_back(*it);
            }
        }
        return moves;
    }

    bool is_square_selectable(Square sq) const {
        if (sq >= 64 || status != GameStatus::Active) return false;
        Piece p = board[sq];
        if (p == No_Piece) return false;
        return piece_color(p) == board.get_turn();
    }

    bool select_square(Square sq) {
        if (status != GameStatus::Active) {
            selected_square = No_Square;
            return false;
        }

        if (sq >= 64) {
            selected_square = No_Square;
            return false;
        }

        if (is_square_selectable(sq)) {
            selected_square = sq;
            return true;
        }

        selected_square = No_Square;
        return false;
    }

    void deselect() {
        selected_square = No_Square;
    }

    bool requires_promotion(Square from, Square to) {
        if (from >= 64 || to >= 64) return false;
        Piece p = board[from];
        if (p != W_Pawn && p != B_Pawn) return false;

        std::vector<Move_full_info> moves = get_legal_moves_from(from);
        for (const auto& m : moves) {
            if (m.to_square == to && m.special == SP_Promotion) {
                return true;
            }
        }
        return false;
    }

    bool make_move(Square from, Square to, Promotion promo = No_promotion) {
        if (status != GameStatus::Active) return false;
        if (from >= 64 || to >= 64) return false;

        std::vector<Move_full_info> legal_moves = get_legal_moves_from(from);
        std::optional<Move_full_info> matched_move;

        for (const auto& m : legal_moves) {
            if (m.to_square == to) {
                if (m.special == SP_Promotion) {
                    if (m.promotion == promo) {
                        matched_move = m;
                        break;
                    }
                } else {
                    matched_move = m;
                    break;
                }
            }
        }

        if (!matched_move.has_value()) {
            return false;
        }

        const Move_full_info move_to_apply = *matched_move;

        if ((board[move_to_apply.from_square] == W_Pawn) || 
            (board[move_to_apply.from_square] == B_Pawn) || 
            (board[move_to_apply.to_square] != No_Piece)) {
            boardlist.clear_list();
            hashlist_ref.clear_hash();
        }

        if (board.get_turn() == White) {
            board.make_move<White>(move_to_apply);
        } else {
            board.make_move<Black>(move_to_apply);
        }

        boardlist.push(board);
        hashlist_ref.add(board.get_hash());

        last_move = move_to_apply;
        selected_square = No_Square;

        evaluate_game_status();
        return true;
    }

    bool make_move(const Move_full_info& move) {
        return make_move(static_cast<Square>(move.from_square), 
                         static_cast<Square>(move.to_square), 
                         move.promotion);
    }
};

} // namespace chess
