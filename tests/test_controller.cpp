#include <iostream>
#include <cassert>
#include "../GameController.hpp"

using namespace chess;

void test_initial_position() {
    GameController gc;
    assert(gc.get_status() == GameStatus::Active);
    assert(gc.get_turn() == White);
    assert(!gc.is_in_check());
    assert(gc.get_piece_at(SQ_E1) == W_King);
    assert(gc.get_piece_at(SQ_E8) == B_King);
    assert(gc.get_piece_at(SQ_E2) == W_Pawn);
    assert(gc.get_piece_at(SQ_E7) == B_Pawn);
    assert(gc.get_piece_at(SQ_B1) == W_Knight);
    assert(gc.get_piece_at(SQ_G1) == W_Knight);

    // Coordinate conversion test (row 0 is rank 8, row 7 is rank 1)
    assert(gc.get_piece_at(7, 4) == W_King); // e1
    assert(gc.get_piece_at(0, 4) == B_King); // e8
    assert(gc.get_piece_at(7, 1) == W_Knight); // b1
    assert(gc.get_piece_at(6, 4) == W_Pawn); // e2
    assert(gc.get_piece_at(3, 3) == No_Piece);

    std::cout << "[PASS] test_initial_position\n";
}

void test_legal_moves_and_selection() {
    GameController gc;

    // White's turn: cannot select black piece
    assert(!gc.select_square(SQ_E7));
    assert(gc.get_selected_square() == No_Square);

    // Can select white e2 pawn
    assert(gc.select_square(SQ_E2));
    assert(gc.get_selected_square() == SQ_E2);

    auto moves = gc.get_legal_moves_from(SQ_E2);
    assert(moves.size() == 2); // e3, e4
    bool has_e3 = false, has_e4 = false;
    for (const auto& m : moves) {
        if (m.to_square == SQ_E3) has_e3 = true;
        if (m.to_square == SQ_E4) has_e4 = true;
    }
    assert(has_e3 && has_e4);

    // Knight moves from b1: a3, c3
    assert(gc.select_square(SQ_B1));
    moves = gc.get_legal_moves_from(SQ_B1);
    assert(moves.size() == 2);

    // Deselect
    gc.deselect();
    assert(gc.get_selected_square() == No_Square);

    std::cout << "[PASS] test_legal_moves_and_selection\n";
}

void test_move_execution() {
    GameController gc;

    // Illegal move: e2 to e5
    assert(!gc.make_move(SQ_E2, SQ_E5));
    assert(gc.get_turn() == White);

    // Legal move 1: e2 -> e4
    assert(gc.make_move(SQ_E2, SQ_E4));
    assert(gc.get_turn() == Black);
    assert(gc.get_piece_at(SQ_E2) == No_Piece);
    assert(gc.get_piece_at(SQ_E4) == W_Pawn);
    assert(gc.get_last_move().from_square == SQ_E2);
    assert(gc.get_last_move().to_square == SQ_E4);

    // Black move 1: e7 -> e5
    assert(gc.make_move(SQ_E7, SQ_E5));
    assert(gc.get_turn() == White);
    assert(gc.get_piece_at(SQ_E7) == No_Piece);
    assert(gc.get_piece_at(SQ_E5) == B_Pawn);

    // White move 2: g1 -> f3
    assert(gc.make_move(SQ_G1, SQ_F3));
    assert(gc.get_turn() == Black);
    assert(gc.get_piece_at(SQ_F3) == W_Knight);

    std::cout << "[PASS] test_move_execution\n";
}

void test_scholars_mate() {
    // 1. e4 e5 2. Qh5 Nc6 3. Bc4 Nf6 4. Qxf7#
    GameController gc;

    assert(gc.make_move(SQ_E2, SQ_E4)); // 1. e4
    assert(gc.make_move(SQ_E7, SQ_E5)); // 1... e5
    assert(gc.make_move(SQ_D1, SQ_H5)); // 2. Qh5
    assert(gc.make_move(SQ_B8, SQ_C6)); // 2... Nc6
    assert(gc.make_move(SQ_F1, SQ_C4)); // 3. Bc4
    assert(gc.make_move(SQ_G8, SQ_F6)); // 3... Nf6
    assert(gc.make_move(SQ_H5, SQ_F7)); // 4. Qxf7#

    assert(gc.is_in_check());
    assert(gc.get_status() == GameStatus::Checkmate);
    assert(gc.get_turn() == Black); // Black is checkmated

    // Cannot make moves when game is over
    assert(!gc.make_move(SQ_C6, SQ_D4));

    std::cout << "[PASS] test_scholars_mate\n";
}

void test_castling_and_promotion() {
    // Castling setup: FEN where White can castle kingside
    GameController gc;
    // FEN: White king at e1, White rook at h1, f1 and g1 empty
    const std::string castle_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1";
    assert(gc.load_fen(castle_fen));
    assert(gc.get_turn() == White);

    auto moves = gc.get_legal_moves_from(SQ_E1);
    bool has_castle = false;
    for (const auto& m : moves) {
        if (m.special == SP_castling && m.to_square == SQ_G1) {
            has_castle = true;
        }
    }
    assert(has_castle);

    // Execute short castling (e1 -> g1)
    assert(gc.make_move(SQ_E1, SQ_G1));
    assert(gc.get_piece_at(SQ_G1) == W_King);
    assert(gc.get_piece_at(SQ_F1) == W_Rook);
    assert(gc.get_piece_at(SQ_H1) == No_Piece);

    // Promotion test: White pawn on a7, about to promote on a8
    const std::string promo_fen = "8/P7/8/8/8/8/8/K6k w - - 0 1";
    assert(gc.load_fen(promo_fen));
    assert(gc.requires_promotion(SQ_A7, SQ_A8));

    // Move without promotion or default promotes to queen if not specified
    assert(gc.make_move(SQ_A7, SQ_A8, promote_to_queen));
    assert(gc.get_piece_at(SQ_A8) == W_Queen);

    std::cout << "[PASS] test_castling_and_promotion\n";
}

void test_en_passant() {
    // 1. e4 a6 2. e5 d5 3. exd6 (en passant)
    GameController gc;
    assert(gc.make_move(SQ_E2, SQ_E4));
    assert(gc.make_move(SQ_A7, SQ_A6));
    assert(gc.make_move(SQ_E4, SQ_E5));
    assert(gc.make_move(SQ_D7, SQ_D5)); // double step -> creates en passant on d6

    auto moves = gc.get_legal_moves_from(SQ_E5);
    bool has_ep = false;
    for (const auto& m : moves) {
        if (m.to_square == SQ_D6 && m.special == SP_en_passant) {
            has_ep = true;
        }
    }
    assert(has_ep);

    // Execute en passant
    assert(gc.make_move(SQ_E5, SQ_D6));
    assert(gc.get_piece_at(SQ_D6) == W_Pawn);
    assert(gc.get_piece_at(SQ_D5) == No_Piece); // Black pawn captured via en passant!

    std::cout << "[PASS] test_en_passant\n";
}

void test_stalemate_and_fen_persistence() {
    // Stalemate position: King on a8, White King on a6, White Queen on c7 -> Black to move, no legal moves, not in check
    const std::string stalemate_fen = "k7/2Q5/K7/8/8/8/8/8 b - - 0 1";
    GameController gc;
    assert(gc.load_fen(stalemate_fen));
    assert(gc.get_turn() == Black);
    assert(!gc.is_in_check());
    assert(gc.get_status() == GameStatus::Stalemate);

    // Test FEN export / import
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    gc.reset_to_start();
    assert(gc.get_fen() == start_fen);

    // Test save and load to file
    assert(gc.save_to_file("pos_test.txt"));
    GameController gc2;
    assert(gc2.load_from_file("pos_test.txt"));
    assert(gc2.get_fen() == start_fen);

    std::cout << "[PASS] test_stalemate_and_fen_persistence\n";
}

void test_ai_integration() {
    GameController gc;
    gc.set_game_mode(GameMode::PvC);
    gc.set_ai_difficulty(Easy);
    gc.set_human_color(White);

    // 1. Human (White) plays e2e4
    assert(gc.make_move(SQ_E2, SQ_E4));
    assert(gc.get_turn() == Black);
    assert(gc.is_ai_turn());

    // 2. Start AI move
    gc.start_ai_thinking();
    assert(gc.is_ai_thinking());

    // 3. Poll until complete
    int poll_count = 0;
    while (!gc.update_ai_thinking()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++poll_count;
        assert(poll_count < 1000); // Should finish within 5 seconds on Easy
    }

    assert(!gc.is_ai_thinking());
    assert(gc.get_turn() == White); // Turn returned to Human!
    assert(!gc.get_last_move().is_no_move());
    std::cout << "  AI played: " << gc.get_last_move() << '\n';

    // 4. Test AI playing White (Human is Black)
    gc.reset_to_start();
    gc.set_game_mode(GameMode::PvC);
    gc.set_ai_difficulty(Easy);
    gc.set_human_color(Black);

    assert(gc.get_turn() == White);
    assert(gc.is_ai_turn());

    gc.start_ai_thinking();
    assert(gc.is_ai_thinking());

    poll_count = 0;
    while (!gc.update_ai_thinking()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++poll_count;
        assert(poll_count < 1000);
    }

    assert(!gc.is_ai_thinking());
    assert(gc.get_turn() == Black); // Turn switched to Human!
    assert(!gc.get_last_move().is_no_move());
    std::cout << "  AI (as White) played: " << gc.get_last_move() << '\n';

    // 5. Test AI cancellation mid-search on Hard difficulty (800,000 nodes)
    gc.reset_to_start();
    gc.set_game_mode(GameMode::PvC);
    gc.set_ai_difficulty(Hard);
    gc.set_human_color(Black);

    gc.start_ai_thinking();
    assert(gc.is_ai_thinking());

    // Let it run for 20ms into the deep search
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(gc.is_ai_thinking());

    // Measure cancellation time
    auto cancel_start = std::chrono::steady_clock::now();
    gc.cancel_ai();
    auto cancel_dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cancel_start).count();

    assert(!gc.is_ai_thinking());
    assert(cancel_dur < 100); // Must interrupt immediately (<100ms instead of 3000ms)
    std::cout << "  AI cancelled mid-search in: " << cancel_dur << " ms\n";

    std::cout << "[PASS] test_ai_integration\n";
}

void test_persistence_and_interoperability() {
    GameController gc1;
    assert(gc1.make_move(SQ_E2, SQ_E4));
    assert(gc1.make_move(SQ_C7, SQ_C5));
    assert(gc1.make_move(SQ_G1, SQ_F3));
    assert(gc1.make_move(SQ_D7, SQ_D6));

    const std::string filename = "pos_interop_test.txt";
    assert(gc1.save_to_file(filename));

    // Verify file content has valid FEN structure
    std::ifstream infile(filename);
    assert(infile.is_open());
    std::string fen;
    std::getline(infile, fen);
    infile.close();
    assert(!fen.empty());
    std::cout << "  Saved FEN: " << fen << '\n';
    assert(fen.find("w KQkq - 0 5") != std::string::npos || fen.find("w KQkq -") != std::string::npos);

    // Verify loading into fresh controller
    GameController gc2;
    assert(gc2.load_from_file(filename));
    assert(gc2.get_piece_at(SQ_E4) == W_Pawn);
    assert(gc2.get_piece_at(SQ_C5) == B_Pawn);
    assert(gc2.get_piece_at(SQ_F3) == W_Knight);
    assert(gc2.get_piece_at(SQ_D6) == B_Pawn);
    assert(gc2.get_turn() == White);
    assert(gc2.get_status() == GameStatus::Active);

    // Verify resignation
    gc2.resign(White);
    assert(gc2.get_status() == GameStatus::Resigned);
    assert(gc2.get_resigned_player() == White);

    std::remove(filename.c_str());
    std::cout << "[PASS] test_persistence_and_interoperability\n";
}

int main() {
    std::cout << "Running GameController Unit Tests...\n";
    test_initial_position();
    test_legal_moves_and_selection();
    test_move_execution();
    test_scholars_mate();
    test_castling_and_promotion();
    test_en_passant();
    test_stalemate_and_fen_persistence();
    test_ai_integration();
    test_persistence_and_interoperability();
    std::cout << "All GameController Unit Tests Passed Successfully!\n";
    return 0;
}
