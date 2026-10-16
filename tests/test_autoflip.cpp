#include <cassert>
#include <iostream>
#include "GameController.hpp"

// We simulate the exact logic used in ChessGUI.cpp for board orientation and auto-flip

struct GUIState {
    chess::GameController controller;
    bool boardFlipped = false;
    bool autoFlip = false;

    GUIState() {
        // Initial setup matching ChessGUI.cpp
        boardFlipped = (controller.get_human_color() == chess::Black);
    }

    void toggleMode() {
        controller.cancel_ai();
        if (controller.get_game_mode() == chess::GameMode::PvP) {
            controller.set_game_mode(chess::GameMode::PvC);
            boardFlipped = (controller.get_human_color() == chess::Black);
        } else {
            controller.set_game_mode(chess::GameMode::PvP);
            boardFlipped = false;
        }
        controller.reset_to_start();
    }

    void toggleHumanColor() {
        if (controller.get_game_mode() != chess::GameMode::PvC) return;
        chess::Color nextColor = (controller.get_human_color() == chess::White) ? chess::Black : chess::White;
        controller.set_human_color(nextColor);
        boardFlipped = (nextColor == chess::Black);
        controller.reset_to_start();
    }

    void toggleFlip() {
        boardFlipped = !boardFlipped;
    }

    void toggleAutoFlip() {
        autoFlip = !autoFlip;
        if (autoFlip) {
            if (controller.get_game_mode() == chess::GameMode::PvP) {
                bool desiredFlipped = (controller.get_turn() == chess::Black);
                if (boardFlipped != desiredFlipped) boardFlipped = desiredFlipped;
            } else if (controller.get_game_mode() == chess::GameMode::PvC) {
                if (!controller.is_ai_turn()) {
                    bool desiredFlipped = (controller.get_human_color() == chess::Black);
                    if (boardFlipped != desiredFlipped) boardFlipped = desiredFlipped;
                }
            }
        }
    }

    void resetGame() {
        controller.cancel_ai();
        controller.reset_to_start();
        if (controller.get_game_mode() == chess::GameMode::PvC) {
            boardFlipped = (controller.get_human_color() == chess::Black);
        } else {
            boardFlipped = false;
        }
    }

    void handleAutoFlipAfterMove() {
        if (!autoFlip) return;
        if (controller.get_game_mode() == chess::GameMode::PvP) {
            bool desiredFlipped = (controller.get_turn() == chess::Black);
            if (boardFlipped != desiredFlipped) {
                boardFlipped = desiredFlipped;
            }
        }
    }

    void onHumanMove(chess::Square from, chess::Square to) {
        if (controller.make_move(from, to)) {
            handleAutoFlipAfterMove();
        }
    }

    void onAiMoveCompleted() {
        if (autoFlip && controller.get_game_mode() == chess::GameMode::PvC) {
            bool desiredFlipped = (controller.get_human_color() == chess::Black);
            if (boardFlipped != desiredFlipped) {
                boardFlipped = desiredFlipped;
            }
        }
    }
};

void test_pvai_initial_orientation() {
    GUIState state;
    // Controller starts in PvP by default
    assert(state.controller.get_game_mode() == chess::GameMode::PvP);
    assert(!state.boardFlipped);

    // Switch to PvC
    state.toggleMode();
    assert(state.controller.get_game_mode() == chess::GameMode::PvC);
    assert(state.controller.get_human_color() == chess::White);
    assert(!state.boardFlipped); // Normal orientation

    // Change human to Black
    state.toggleHumanColor();
    assert(state.controller.get_human_color() == chess::Black);
    assert(state.boardFlipped); // Flipped orientation

    // Reset game while playing as Black
    state.toggleFlip(); // Manually flip to normal
    assert(!state.boardFlipped);
    state.resetGame();
    // After reset, takes human orientation (Black = flipped)
    assert(state.boardFlipped);

    std::cout << "[PASS] test_pvai_initial_orientation\n";
}

void test_pvp_autoflip() {
    GUIState state;
    assert(state.controller.get_game_mode() == chess::GameMode::PvP);
    assert(!state.boardFlipped);

    // Auto-flip is OFF by default
    assert(!state.autoFlip);
    state.onHumanMove(chess::SQ_E2, chess::SQ_E4);
    assert(state.controller.get_turn() == chess::Black);
    assert(!state.boardFlipped); // Did NOT flip because auto-flip is OFF

    // Now turn auto-flip ON
    state.toggleAutoFlip();
    assert(state.autoFlip);
    assert(state.boardFlipped); // Since it's Black's turn, instantly aligned to Black!

    // Black plays E7 -> E5
    state.onHumanMove(chess::SQ_E7, chess::SQ_E5);
    assert(state.controller.get_turn() == chess::White);
    assert(!state.boardFlipped); // Auto-flipped to White!

    // White manually flips before moving
    state.toggleFlip();
    assert(state.boardFlipped); // White is now looking at flipped board
    // White plays G1 -> F3
    state.onHumanMove(chess::SQ_G1, chess::SQ_F3);
    assert(state.controller.get_turn() == chess::Black);
    // Desired is Black (flipped=true). Board was already flipped!
    // "if the board is already in the right position, it should be left unchanged, otherwise flipped."
    assert(state.boardFlipped); // Unchanged

    // Black plays B8 -> C6
    state.onHumanMove(chess::SQ_B8, chess::SQ_C6);
    assert(state.controller.get_turn() == chess::White);
    // Desired is White (flipped=false). Board was flipped(true) -> flipped to false!
    assert(!state.boardFlipped);

    std::cout << "[PASS] test_pvp_autoflip\n";
}

void test_pvai_autoflip() {
    GUIState state;
    state.toggleMode(); // Switch to PvC mode, human is White
    assert(state.controller.get_game_mode() == chess::GameMode::PvC);
    assert(state.controller.get_human_color() == chess::White);
    assert(!state.boardFlipped);

    state.toggleAutoFlip(); // Enable autoFlip
    assert(state.autoFlip);

    // Human plays E2 -> E4
    state.onHumanMove(chess::SQ_E2, chess::SQ_E4);
    assert(state.controller.get_turn() == chess::Black);
    // In PvAI, board does NOT flip after player's move!
    assert(!state.boardFlipped);

    // Suppose player manually flips board while waiting for AI
    state.toggleFlip();
    assert(state.boardFlipped);

    // AI completes move
    state.controller.make_move(chess::SQ_E7, chess::SQ_E5);
    state.onAiMoveCompleted();
    // After AI move, auto-flip restores player orientation (White = not flipped)!
    assert(!state.boardFlipped);

    // Now test with human playing as Black
    state.toggleHumanColor(); // Resets and sets human to Black
    assert(state.controller.get_human_color() == chess::Black);
    assert(state.boardFlipped); // Black orientation

    // Turn autoFlip back on if it was reset
    if (!state.autoFlip) state.toggleAutoFlip();

    // AI (White) makes move
    state.controller.make_move(chess::SQ_E2, chess::SQ_E4);
    state.onAiMoveCompleted();
    // Already in Black orientation -> unchanged!
    assert(state.boardFlipped);

    // Player manually flips to White orientation
    state.toggleFlip();
    assert(!state.boardFlipped);

    // Player (Black) plays E7 -> E5
    state.controller.make_move(chess::SQ_E7, chess::SQ_E5);
    state.handleAutoFlipAfterMove(); // in PvAI, does not flip
    assert(!state.boardFlipped);

    // AI plays G1 -> F3
    state.controller.make_move(chess::SQ_G1, chess::SQ_F3);
    state.onAiMoveCompleted();
    // AI move completed -> flips to player orientation (Black = flipped)!
    assert(state.boardFlipped);

    std::cout << "[PASS] test_pvai_autoflip\n";
}

int main() {
    std::cout << "Testing Auto-Flip functionality...\n";
    test_pvai_initial_orientation();
    test_pvp_autoflip();
    test_pvai_autoflip();
    std::cout << "All Auto-Flip tests passed successfully!\n";
    return 0;
}
