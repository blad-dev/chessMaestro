#include <cassert>
#include <iostream>
#include "GameController.hpp"

// Simulation of GUIState logic for color switching
struct GUIColorSwitchState {
    chess::GameController controller;
    bool boardFlipped = false;
    bool autoFlip = false;
    bool isPromoting = false;

    GUIColorSwitchState() {
        controller.set_game_mode(chess::GameMode::PvC);
        boardFlipped = (controller.get_human_color() == chess::Black);
    }

    void toggleAutoFlip() {
        autoFlip = !autoFlip;
    }

    void toggleHumanColor() {
        if (controller.get_game_mode() != chess::GameMode::PvC) return;
        controller.cancel_ai();
        isPromoting = false;
        controller.deselect();
        chess::Color nextColor = (controller.get_human_color() == chess::White) ? chess::Black : chess::White;
        controller.set_human_color(nextColor);
        if (autoFlip) {
            bool desiredFlipped = (nextColor == chess::Black);
            if (boardFlipped != desiredFlipped) {
                boardFlipped = desiredFlipped;
            }
        }
        if (controller.is_ai_turn() && controller.get_status() == chess::GameStatus::Active) {
            controller.start_ai_thinking();
        }
    }
};

void test_ai_interrupted_becomes_player_move() {
    GUIColorSwitchState state;
    assert(state.controller.get_game_mode() == chess::GameMode::PvC);
    assert(state.controller.get_human_color() == chess::White);
    assert(state.controller.get_turn() == chess::White);

    // Human (White) plays E2 -> E4
    bool moved = state.controller.make_move(chess::SQ_E2, chess::SQ_E4);
    assert(moved);
    assert(state.controller.get_turn() == chess::Black);
    assert(state.controller.is_ai_turn());

    // AI starts thinking for Black
    state.controller.start_ai_thinking();
    assert(state.controller.is_ai_thinking());

    // While AI is thinking, player switches color to Black!
    state.toggleHumanColor();

    // 1. AI should be interrupted
    assert(!state.controller.is_ai_thinking());
    // 2. Position should NOT be reset (e4 still has White Pawn, e2 is empty)
    assert(state.controller.get_piece_at(chess::SQ_E4) == chess::W_Pawn);
    assert(state.controller.get_piece_at(chess::SQ_E2) == chess::No_Piece);
    // 3. Human color is now Black
    assert(state.controller.get_human_color() == chess::Black);
    // 4. Current move becomes the player's move
    assert(state.controller.get_turn() == chess::Black);
    assert(!state.controller.is_ai_turn());

    // Human makes Black's move (E7 -> E5)
    moved = state.controller.make_move(chess::SQ_E7, chess::SQ_E5);
    assert(moved);
    assert(state.controller.get_turn() == chess::White);
    // Now it's White's turn, and AI is White!
    assert(state.controller.is_ai_turn());

    std::cout << "[PASS] test_ai_interrupted_becomes_player_move\n";
}

void test_player_turn_ai_starts_thinking() {
    GUIColorSwitchState state;
    assert(state.controller.get_human_color() == chess::White);
    assert(state.controller.get_turn() == chess::White);

    // Initial position: it is player's move (White to move)
    assert(!state.controller.is_ai_turn());
    assert(!state.controller.is_ai_thinking());

    // Player switches color to Black
    state.toggleHumanColor();

    // Position not reset
    assert(state.controller.get_piece_at(chess::SQ_E2) == chess::W_Pawn);
    assert(state.controller.get_human_color() == chess::Black);
    assert(state.controller.get_turn() == chess::White);

    // AI is now White, so it's AI's turn and AI starts thinking!
    assert(state.controller.is_ai_turn());
    assert(state.controller.is_ai_thinking());

    // Wait for AI to finish its move
    while (state.controller.is_ai_thinking()) {
        state.controller.update_ai_thinking();
    }

    // AI made its move for White!
    assert(state.controller.get_turn() == chess::Black);
    assert(!state.controller.is_ai_turn()); // Now player's turn (as Black)

    std::cout << "[PASS] test_player_turn_ai_starts_thinking\n";
}

void test_orientation_behavior() {
    // 1. When autoFlip is OFF:
    {
        GUIColorSwitchState state;
        state.autoFlip = false;
        state.boardFlipped = false; // normal

        state.toggleHumanColor(); // switch to Black
        // Board orientation remains unchanged
        assert(!state.boardFlipped);

        state.boardFlipped = true; // user manually flipped with [F]
        state.toggleHumanColor(); // switch to White
        // Board orientation remains unchanged
        assert(state.boardFlipped);
    }

    // 2. When autoFlip is ON:
    {
        GUIColorSwitchState state;
        state.autoFlip = true;
        state.boardFlipped = false; // normal

        state.toggleHumanColor(); // switch to Black
        // Flips to player's new orientation (Black = true)
        assert(state.boardFlipped);

        state.toggleHumanColor(); // switch to White
        // Flips to player's new orientation (White = false)
        assert(!state.boardFlipped);
    }

    std::cout << "[PASS] test_orientation_behavior\n";
}

int main() {
    std::cout << "Testing Non-Resetting Color Switch functionality...\n";
    test_ai_interrupted_becomes_player_move();
    test_player_turn_ai_starts_thinking();
    test_orientation_behavior();
    std::cout << "All Color Switch tests passed successfully!\n";
    return 0;
}
