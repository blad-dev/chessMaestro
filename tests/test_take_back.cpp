#include <cassert>
#include <iostream>
#include "GameController.hpp"

void test_pvp_take_back() {
    chess::GameController controller;
    controller.set_game_mode(chess::GameMode::PvP);

    // Initial position: 0 moves
    assert(!controller.can_take_move_back());
    assert(!controller.take_move_back());

    // White plays E2 -> E4
    bool ok = controller.make_move(chess::SQ_E2, chess::SQ_E4);
    assert(ok);
    assert(controller.get_turn() == chess::Black);
    assert(controller.get_piece_at(chess::SQ_E4) == chess::W_Pawn);
    assert(controller.get_history_size() == 1);
    assert(controller.can_take_move_back());

    // Take move back
    ok = controller.take_move_back();
    assert(ok);
    assert(controller.get_turn() == chess::White);
    assert(controller.get_piece_at(chess::SQ_E4) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_E2) == chess::W_Pawn);
    assert(controller.get_history_size() == 0);
    assert(!controller.can_take_move_back());

    // Multiple moves
    controller.make_move(chess::SQ_E2, chess::SQ_E4);
    controller.make_move(chess::SQ_E7, chess::SQ_E5);
    controller.make_move(chess::SQ_G1, chess::SQ_F3);
    assert(controller.get_history_size() == 3);
    assert(controller.get_turn() == chess::Black);

    // Take back G1-F3
    assert(controller.take_move_back());
    assert(controller.get_turn() == chess::White);
    assert(controller.get_piece_at(chess::SQ_F3) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_G1) == chess::W_Knight);
    assert(controller.get_last_move().to_square == chess::SQ_E5);

    std::cout << "[PASS] test_pvp_take_back\n";
}

void test_castling_and_en_passant_undo() {
    // 1. Castling undo
    {
        chess::GameController controller;
        controller.set_game_mode(chess::GameMode::PvP);

        controller.make_move(chess::SQ_E2, chess::SQ_E4);
        controller.make_move(chess::SQ_E7, chess::SQ_E5);
        controller.make_move(chess::SQ_G1, chess::SQ_F3);
        controller.make_move(chess::SQ_B8, chess::SQ_C6);
        controller.make_move(chess::SQ_F1, chess::SQ_C4);
        controller.make_move(chess::SQ_F8, chess::SQ_C5);

        // Kingside castling: E1 -> G1
        assert(controller.get_board().has_castling(chess::White_OO));
        bool castled = controller.make_move(chess::SQ_E1, chess::SQ_G1);
        assert(castled);
        assert(controller.get_piece_at(chess::SQ_G1) == chess::W_King);
        assert(controller.get_piece_at(chess::SQ_F1) == chess::W_Rook);
        assert(controller.get_piece_at(chess::SQ_H1) == chess::No_Piece);
        assert(controller.get_piece_at(chess::SQ_E1) == chess::No_Piece);

        // Undo castling
        assert(controller.take_move_back());
        assert(controller.get_piece_at(chess::SQ_E1) == chess::W_King);
        assert(controller.get_piece_at(chess::SQ_H1) == chess::W_Rook);
        assert(controller.get_piece_at(chess::SQ_F1) == chess::No_Piece);
        assert(controller.get_piece_at(chess::SQ_G1) == chess::No_Piece);
        assert(controller.get_board().has_castling(chess::White_OO));
    }

    // 2. En Passant undo
    {
        chess::GameController controller;
        controller.set_game_mode(chess::GameMode::PvP);

        controller.make_move(chess::SQ_E2, chess::SQ_E4);
        controller.make_move(chess::SQ_A7, chess::SQ_A6);
        controller.make_move(chess::SQ_E4, chess::SQ_E5);
        // Black plays D7 -> D5
        controller.make_move(chess::SQ_D7, chess::SQ_D5);
        assert(controller.get_board().get_en_passant() == chess::SQ_D6);

        // White captures en passant: E5 -> D6
        bool ep_captured = controller.make_move(chess::SQ_E5, chess::SQ_D6);
        assert(ep_captured);
        assert(controller.get_piece_at(chess::SQ_D6) == chess::W_Pawn);
        assert(controller.get_piece_at(chess::SQ_D5) == chess::No_Piece); // Black pawn removed

        // Undo en passant
        assert(controller.take_move_back());
        assert(controller.get_piece_at(chess::SQ_E5) == chess::W_Pawn);
        assert(controller.get_piece_at(chess::SQ_D5) == chess::B_Pawn); // Black pawn revived!
        assert(controller.get_piece_at(chess::SQ_D6) == chess::No_Piece);
        assert(controller.get_board().get_en_passant() == chess::SQ_D6);
    }

    std::cout << "[PASS] test_castling_and_en_passant_undo\n";
}

void test_pvai_take_back_ai_thinking() {
    chess::GameController controller;
    controller.set_game_mode(chess::GameMode::PvC);
    controller.set_human_color(chess::White);

    // Human (White) plays E2 -> E4
    controller.make_move(chess::SQ_E2, chess::SQ_E4);
    assert(controller.is_ai_turn());

    // AI starts thinking for Black
    controller.start_ai_thinking();
    assert(controller.is_ai_thinking());
    assert(controller.can_take_move_back());

    // Case 1.1: AI is thinking -> stops thinking, takes one move back (the human's move)
    bool ok = controller.take_move_back();
    assert(ok);
    assert(!controller.is_ai_thinking());
    assert(controller.get_turn() == chess::White); // Back to player's turn!
    assert(controller.get_piece_at(chess::SQ_E4) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_E2) == chess::W_Pawn);
    assert(controller.get_history_size() == 0);

    // Edge case 1.1: Human is Black, AI is White thinking about opening move (0 stored moves)
    controller.set_human_color(chess::Black);
    controller.reset_to_start();
    assert(controller.get_history_size() == 0);
    controller.start_ai_thinking();
    assert(controller.is_ai_thinking());
    // No stored moves after initial position -> ignored!
    assert(!controller.can_take_move_back());
    assert(!controller.take_move_back());
    assert(controller.is_ai_thinking()); // AI still thinking

    controller.cancel_ai();
    std::cout << "[PASS] test_pvai_take_back_ai_thinking\n";
}

void test_pvai_take_back_player_move() {
    chess::GameController controller;
    controller.set_game_mode(chess::GameMode::PvC);
    controller.set_human_color(chess::White);

    // Human plays E2 -> E4 (move 1)
    controller.make_move(chess::SQ_E2, chess::SQ_E4);
    // AI plays E7 -> E5 (move 2)
    controller.make_move(chess::SQ_E7, chess::SQ_E5);

    // Now it is human's turn (White). Stored moves = 2.
    assert(controller.get_turn() == chess::White);
    assert(!controller.is_ai_thinking());
    assert(controller.can_take_move_back());

    // Case 1.2: Takes two moves back (AI's E7-E5 and player's E2-E4)
    bool ok = controller.take_move_back();
    assert(ok);
    assert(controller.get_turn() == chess::White);
    assert(controller.get_history_size() == 0);
    assert(controller.get_piece_at(chess::SQ_E4) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_E5) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_E2) == chess::W_Pawn);
    assert(controller.get_piece_at(chess::SQ_E7) == chess::B_Pawn);

    // Edge case 1.2: Human is Black, AI made 1 move (E2 -> E4)
    controller.set_human_color(chess::Black);
    controller.reset_to_start();
    controller.make_move(chess::SQ_E2, chess::SQ_E4); // AI's move (1 stored move)
    assert(controller.get_turn() == chess::Black);
    assert(controller.get_history_size() == 1);

    // Only 1 stored move -> ignored!
    assert(!controller.can_take_move_back());
    assert(!controller.take_move_back());
    assert(controller.get_history_size() == 1);
    assert(controller.get_turn() == chess::Black);

    // If Black also made a move (E7 -> E5) and AI made another move (G1 -> F3):
    controller.make_move(chess::SQ_E7, chess::SQ_E5); // Black's move (2 stored moves)
    controller.make_move(chess::SQ_G1, chess::SQ_F3); // AI's move (3 stored moves)
    assert(controller.get_turn() == chess::Black);
    assert(controller.get_history_size() == 3);
    assert(controller.can_take_move_back());

    // Takes two moves back: G1-F3 and E7-E5 -> leaves 1 stored move (E2-E4)
    assert(controller.take_move_back());
    assert(controller.get_turn() == chess::Black);
    assert(controller.get_history_size() == 1);
    assert(controller.get_piece_at(chess::SQ_F3) == chess::No_Piece);
    assert(controller.get_piece_at(chess::SQ_E5) == chess::No_Piece);

    std::cout << "[PASS] test_pvai_take_back_player_move\n";
}

void test_history_cleared_on_fen_and_reset() {
    chess::GameController controller;
    controller.set_game_mode(chess::GameMode::PvP);

    controller.make_move(chess::SQ_E2, chess::SQ_E4);
    assert(controller.get_history_size() == 1);

    // Reset clears history
    controller.reset_to_start();
    assert(controller.get_history_size() == 0);
    assert(!controller.can_take_move_back());

    // Load FEN clears history
    controller.make_move(chess::SQ_E2, chess::SQ_E4);
    assert(controller.get_history_size() == 1);
    controller.load_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    assert(controller.get_history_size() == 0);
    assert(!controller.can_take_move_back());

    std::cout << "[PASS] test_history_cleared_on_fen_and_reset\n";
}

int main() {
    std::cout << "Testing 'Take Move Back' functionality...\n";
    test_pvp_take_back();
    test_castling_and_en_passant_undo();
    test_pvai_take_back_ai_thinking();
    test_pvai_take_back_player_move();
    test_history_cleared_on_fen_and_reset();
    std::cout << "All 'Take Move Back' tests passed successfully!\n";
    return 0;
}
