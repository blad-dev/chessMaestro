#include <SFML/Graphics.hpp>
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "GameController.hpp"

namespace {
constexpr unsigned int WindowWidth = 840;
constexpr unsigned int WindowHeight = 760;
constexpr float BoardSize = 480.0f;
constexpr float SquareSize = BoardSize / 8.0f;

const std::string FontPath = "media_files/fonts/Roboto/Roboto-Regular.ttf";
const std::string FontBoldPath = "media_files/fonts/Roboto/Roboto-Bold.ttf";

const std::array<std::string, 12> PieceTexturePaths = {
    "media_files/pieces/png/white_pawn.png",
    "media_files/pieces/png/white_knight.png",
    "media_files/pieces/png/white_bishop.png",
    "media_files/pieces/png/white_rook.png",
    "media_files/pieces/png/white_queen.png",
    "media_files/pieces/png/white_king.png",
    "media_files/pieces/png/black_pawn.png",
    "media_files/pieces/png/black_knight.png",
    "media_files/pieces/png/black_bishop.png",
    "media_files/pieces/png/black_rook.png",
    "media_files/pieces/png/black_queen.png",
    "media_files/pieces/png/black_king.png"
};

void centerText(sf::Text& text, float centerX, float centerY) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f,
                   bounds.top + bounds.height / 2.0f);
    text.setPosition(centerX, centerY);
}

struct Button {
    sf::FloatRect bounds;
    std::string text;
    bool active = true;
};

} // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight),
                            "ChessMaestro - SFML 2.6.2",
                            sf::Style::Titlebar | sf::Style::Close);

    if (!window.isOpen()) {
        std::cerr << "Error: failed to create the SFML window.\n";
        return EXIT_FAILURE;
    }

    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    sf::Font uiFont;
    if (!uiFont.loadFromFile(FontPath)) {
        std::cerr << "Error: could not load font: " << FontPath << '\n';
        return EXIT_FAILURE;
    }

    sf::Font titleFont;
    if (!titleFont.loadFromFile(FontBoldPath)) {
        titleFont = uiFont;
    }

    std::array<sf::Texture, 12> pieceTextures;
    for (size_t i = 0; i < 12; ++i) {
        if (!pieceTextures[i].loadFromFile(PieceTexturePaths[i])) {
            std::cerr << "Error: could not load piece texture: " << PieceTexturePaths[i] << '\n';
            return EXIT_FAILURE;
        }
        pieceTextures[i].setSmooth(true);
    }

    const float pieceTextureWidth = static_cast<float>(pieceTextures[0].getSize().x);
    const float spriteScale = SquareSize / pieceTextureWidth;

    constexpr float boardLeft = (static_cast<float>(WindowWidth) - BoardSize) / 2.0f;
    constexpr float boardTop = 115.0f;

    chess::GameController controller;

    // View & state options
    bool boardFlipped = false;
    bool autoFlip = false;
    bool isPromoting = false;
    chess::Square promoFrom = chess::No_Square;
    chess::Square promoTo = chess::No_Square;

    // Toast feedback notification
    std::string toastMessage;
    sf::Color toastColor = sf::Color(129, 199, 132);
    float toastTimer = 0.0f;

    sf::Clock frameClock;
    sf::Clock animClock;

    // Coordinate conversion lambdas (handles board flipping)
    auto getSquareAtVisual = [&](int row, int col) -> chess::Square {
        if (row < 0 || row > 7 || col < 0 || col > 7) return chess::No_Square;
        int rank = boardFlipped ? row : (7 - row);
        int file = boardFlipped ? (7 - col) : col;
        return static_cast<chess::Square>(rank * 8 + file);
    };

    auto getVisualAtSquare = [&](chess::Square sq) -> std::pair<int, int> {
        int file = sq % 8;
        int rank = sq / 8;
        int col = boardFlipped ? (7 - file) : file;
        int row = boardFlipped ? rank : (7 - rank);
        return {row, col};
    };

    const sf::Color lightSquare(240, 217, 181);
    const sf::Color darkSquare(181, 136, 99);

    auto handleAutoFlipAfterMove = [&]() {
        if (!autoFlip) return;
        if (controller.get_game_mode() == chess::GameMode::PvP) {
            bool desiredFlipped = (controller.get_turn() == chess::Black);
            if (boardFlipped != desiredFlipped) {
                boardFlipped = desiredFlipped;
            }
        }
    };

    auto toggleMode = [&]() {
        controller.cancel_ai();
        isPromoting = false;
        if (controller.get_game_mode() == chess::GameMode::PvP) {
            controller.set_game_mode(chess::GameMode::PvC);
            boardFlipped = (controller.get_human_color() == chess::Black);
        } else {
            controller.set_game_mode(chess::GameMode::PvP);
            boardFlipped = false;
        }
        controller.reset_to_start();
        toastMessage = (controller.get_game_mode() == chess::GameMode::PvP) ? "Mode: Two-Player" : "Mode: vs AI";
        toastColor = sf::Color(144, 202, 249);
        toastTimer = 2.0f;
    };

    auto cycleDifficulty = [&]() {
        if (controller.get_game_mode() != chess::GameMode::PvC) return;
        
        bool wasThinking = controller.is_ai_thinking();
        controller.cancel_ai();

        auto diff = controller.get_ai_difficulty();
        std::string diffName;
        if (diff == chess::Easy) {
            controller.set_ai_difficulty(chess::Normal);
            diffName = "Normal";
        } else if (diff == chess::Normal) {
            controller.set_ai_difficulty(chess::Hard);
            diffName = "Hard";
        } else {
            controller.set_ai_difficulty(chess::Easy);
            diffName = "Easy";
        }

        if ((wasThinking || controller.is_ai_turn()) && 
            controller.get_status() == chess::GameStatus::Active && 
            !isPromoting) {
            controller.start_ai_thinking();
        }

        toastMessage = "AI Difficulty: " + diffName;
        toastColor = sf::Color(144, 202, 249);
        toastTimer = 2.0f;
    };

    auto toggleHumanColor = [&]() {
        if (controller.get_game_mode() != chess::GameMode::PvC) return;
        controller.cancel_ai();
        isPromoting = false;
        chess::Color nextColor = (controller.get_human_color() == chess::White) ? chess::Black : chess::White;
        controller.set_human_color(nextColor);
        boardFlipped = (nextColor == chess::Black);
        controller.reset_to_start();
        toastMessage = "Playing as: " + std::string(nextColor == chess::White ? "White" : "Black");
        toastColor = sf::Color(144, 202, 249);
        toastTimer = 2.0f;
    };

    auto toggleFlip = [&]() {
        boardFlipped = !boardFlipped;
        toastMessage = boardFlipped ? "Board flipped (Black at bottom)" : "Board normal (White at bottom)";
        toastColor = sf::Color(200, 200, 200);
        toastTimer = 1.5f;
    };

    auto toggleAutoFlip = [&]() {
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
        toastMessage = autoFlip ? "Auto-flip: ON" : "Auto-flip: OFF";
        toastColor = autoFlip ? sf::Color(129, 199, 132) : sf::Color(200, 200, 200);
        toastTimer = 1.5f;
    };

    auto resetGame = [&]() {
        controller.cancel_ai();
        isPromoting = false;
        controller.reset_to_start();
        if (controller.get_game_mode() == chess::GameMode::PvC) {
            boardFlipped = (controller.get_human_color() == chess::Black);
        } else {
            boardFlipped = false;
        }
        toastMessage = "New game started";
        toastColor = sf::Color(200, 200, 200);
        toastTimer = 1.5f;
    };

    auto saveGame = [&]() {
        if (controller.save_to_file("pos.txt")) {
            toastMessage = "Position saved to pos.txt";
            toastColor = sf::Color(129, 199, 132);
        } else {
            toastMessage = "Failed to save pos.txt";
            toastColor = sf::Color(239, 83, 80);
        }
        toastTimer = 2.5f;
    };

    auto loadGame = [&]() {
        controller.cancel_ai();
        isPromoting = false;
        if (controller.load_from_file("pos.txt")) {
            toastMessage = "Position loaded from pos.txt";
            toastColor = sf::Color(129, 199, 132);
        } else {
            toastMessage = "Failed to load pos.txt";
            toastColor = sf::Color(239, 83, 80);
        }
        toastTimer = 2.5f;
    };

    auto resignGame = [&]() {
        if (controller.get_status() == chess::GameStatus::Active) {
            chess::Color surr = controller.get_turn();
            controller.resign(surr);
            toastMessage = std::string(surr == chess::White ? "White" : "Black") + " surrendered!";
            toastColor = sf::Color(255, 179, 0);
            toastTimer = 3.0f;
        }
    };

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        if (toastTimer > 0.0f) {
            toastTimer -= dt;
        }

        // Asynchronous AI trigger & polling
        if (controller.is_ai_turn() && !controller.is_ai_thinking() && !isPromoting) {
            controller.start_ai_thinking();
        }

        if (controller.is_ai_thinking()) {
            if (controller.update_ai_thinking()) {
                if (autoFlip && controller.get_game_mode() == chess::GameMode::PvC) {
                    bool desiredFlipped = (controller.get_human_color() == chess::Black);
                    if (boardFlipped != desiredFlipped) {
                        boardFlipped = desiredFlipped;
                    }
                }
            }
        }

        // --- Toolbar Buttons Setup ---
        // Row 1 (y = 630): Mode, Difficulty, Color, Flip, Auto-Flip
        constexpr float btnY1 = boardTop + BoardSize + 30.0f;
        constexpr float btnHeight = 28.0f;
        Button btnMode{sf::FloatRect(boardLeft, btnY1, 88.0f, btnHeight), ""};
        Button btnDiff{sf::FloatRect(boardLeft + 94.0f, btnY1, 86.0f, btnHeight), ""};
        Button btnColor{sf::FloatRect(boardLeft + 186.0f, btnY1, 86.0f, btnHeight), ""};
        Button btnFlip{sf::FloatRect(boardLeft + 278.0f, btnY1, 80.0f, btnHeight), boardFlipped ? "[F] Flipped" : "[F] Normal"};
        Button btnAutoFlip{sf::FloatRect(boardLeft + 364.0f, btnY1, 116.0f, btnHeight), autoFlip ? "[A] Auto-Flip: ON" : "[A] Auto-Flip: OFF"};

        btnMode.text = (controller.get_game_mode() == chess::GameMode::PvP) ? "[M] PvP" : "[M] vs AI";
        if (controller.get_game_mode() == chess::GameMode::PvC) {
            auto diff = controller.get_ai_difficulty();
            btnDiff.text = (diff == chess::Easy) ? "[D] Easy" : (diff == chess::Normal) ? "[D] Normal" : "[D] Hard";
            btnColor.text = (controller.get_human_color() == chess::White) ? "[C] White" : "[C] Black";
        } else {
            btnDiff.text = "[D] ---";
            btnDiff.active = false;
            btnColor.text = "[C] ---";
            btnColor.active = false;
        }

        // Row 2 (y = 665): Save, Load, Resign, Reset
        constexpr float btnY2 = btnY1 + btnHeight + 7.0f;
        Button btnSave{sf::FloatRect(boardLeft, btnY2, 115.0f, btnHeight), "[S] Save"};
        Button btnLoad{sf::FloatRect(boardLeft + 122.0f, btnY2, 115.0f, btnHeight), "[L] Load"};
        Button btnResign{sf::FloatRect(boardLeft + 244.0f, btnY2, 115.0f, btnHeight), "[Q] Resign", controller.get_status() == chess::GameStatus::Active};
        Button btnReset{sf::FloatRect(boardLeft + 366.0f, btnY2, 114.0f, btnHeight), "[R] Reset"};

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                controller.cancel_ai();
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    if (isPromoting) {
                        isPromoting = false;
                        controller.deselect();
                    } else {
                        controller.cancel_ai();
                        window.close();
                    }
                } else if (event.key.code == sf::Keyboard::R) {
                    resetGame();
                } else if (event.key.code == sf::Keyboard::M) {
                    toggleMode();
                } else if (event.key.code == sf::Keyboard::D) {
                    cycleDifficulty();
                } else if (event.key.code == sf::Keyboard::C) {
                    toggleHumanColor();
                } else if (event.key.code == sf::Keyboard::F) {
                    toggleFlip();
                } else if (event.key.code == sf::Keyboard::A) {
                    toggleAutoFlip();
                } else if (event.key.code == sf::Keyboard::S) {
                    saveGame();
                } else if (event.key.code == sf::Keyboard::L) {
                    loadGame();
                } else if (event.key.code == sf::Keyboard::Q) {
                    resignGame();
                }
            } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                float mx = static_cast<float>(event.mouseButton.x);
                float my = static_cast<float>(event.mouseButton.y);

                // Row 1 Buttons
                if (btnMode.bounds.contains(mx, my)) {
                    toggleMode();
                } else if (btnDiff.active && btnDiff.bounds.contains(mx, my)) {
                    cycleDifficulty();
                } else if (btnColor.active && btnColor.bounds.contains(mx, my)) {
                    toggleHumanColor();
                } else if (btnFlip.bounds.contains(mx, my)) {
                    toggleFlip();
                } else if (btnAutoFlip.bounds.contains(mx, my)) {
                    toggleAutoFlip();
                }
                // Row 2 Buttons
                else if (btnSave.bounds.contains(mx, my)) {
                    saveGame();
                } else if (btnLoad.bounds.contains(mx, my)) {
                    loadGame();
                } else if (btnResign.active && btnResign.bounds.contains(mx, my)) {
                    resignGame();
                } else if (btnReset.bounds.contains(mx, my)) {
                    resetGame();
                } else if (isPromoting) {
                    // Check promotion dialog clicks
                    constexpr float modalWidth = 260.0f;
                    constexpr float modalHeight = 90.0f;
                    float modalX = (static_cast<float>(WindowWidth) - modalWidth) / 2.0f;
                    float modalY = boardTop + (BoardSize - modalHeight) / 2.0f;
                    constexpr float boxSize = 56.0f;
                    constexpr float boxPadding = 6.0f;
                    float startX = modalX + (modalWidth - (4 * boxSize + 3 * boxPadding)) / 2.0f;
                    float startY = modalY + 24.0f;

                    std::array<chess::Promotion, 4> promos = {
                        chess::promote_to_queen,
                        chess::promote_to_rook,
                        chess::promote_to_bishop,
                        chess::promote_to_knight
                    };

                    for (size_t i = 0; i < 4; ++i) {
                        float bx = startX + static_cast<float>(i) * (boxSize + boxPadding);
                        float by = startY;
                        sf::FloatRect boxRect(bx, by, boxSize, boxSize);
                        if (boxRect.contains(mx, my)) {
                            if (controller.make_move(promoFrom, promoTo, promos[i])) {
                                handleAutoFlipAfterMove();
                            }
                            isPromoting = false;
                            break;
                        }
                    }
                } else if (controller.get_status() == chess::GameStatus::Active && !controller.is_ai_thinking()) {
                    if (mx >= boardLeft && mx < boardLeft + BoardSize &&
                        my >= boardTop && my < boardTop + BoardSize) {
                        int col = static_cast<int>((mx - boardLeft) / SquareSize);
                        int row = static_cast<int>((my - boardTop) / SquareSize);
                        if (col >= 0 && col < 8 && row >= 0 && row < 8) {
                            chess::Square clickedSq = getSquareAtVisual(row, col);
                            chess::Square currentSelected = controller.get_selected_square();

                            if (currentSelected != chess::No_Square) {
                                if (clickedSq == currentSelected) {
                                    controller.deselect();
                                } else {
                                    // Check if clicked square is a legal destination
                                    auto legalMoves = controller.get_legal_moves_from(currentSelected);
                                    bool isLegalDest = false;
                                    for (const auto& m : legalMoves) {
                                        if (m.to_square == clickedSq) {
                                            isLegalDest = true;
                                            break;
                                        }
                                    }

                                    if (isLegalDest) {
                                        if (controller.requires_promotion(currentSelected, clickedSq)) {
                                            isPromoting = true;
                                            promoFrom = currentSelected;
                                            promoTo = clickedSq;
                                        } else {
                                            if (controller.make_move(currentSelected, clickedSq)) {
                                                handleAutoFlipAfterMove();
                                            }
                                        }
                                    } else if (controller.is_square_selectable(clickedSq)) {
                                        controller.select_square(clickedSq);
                                    } else {
                                        controller.deselect();
                                    }
                                }
                            } else {
                                if (controller.is_square_selectable(clickedSq)) {
                                    controller.select_square(clickedSq);
                                }
                            }
                        }
                    } else {
                        controller.deselect();
                    }
                }
            }
        }

        // --- Render Frame ---
        window.clear(sf::Color(30, 33, 36));

        // 1. Title & Status Header
        std::string modeTitle = "ChessMaestro - ";
        if (controller.get_game_mode() == chess::GameMode::PvP) {
            modeTitle += "Two-Player Mode";
        } else {
            auto diff = controller.get_ai_difficulty();
            std::string diffStr = (diff == chess::Easy) ? "Easy" : (diff == chess::Normal) ? "Normal" : "Hard";
            std::string colorStr = (controller.get_human_color() == chess::White) ? "White" : "Black";
            modeTitle += "vs AI (" + diffStr + ", You: " + colorStr + ")";
        }

        sf::Text titleText(modeTitle, titleFont, 21);
        titleText.setFillColor(sf::Color(240, 240, 240));
        centerText(titleText, static_cast<float>(WindowWidth) / 2.0f, 26.0f);
        window.draw(titleText);

        std::string statusStr;
        sf::Color statusColor = sf::Color(180, 180, 180);
        const auto status = controller.get_status();

        if (status == chess::GameStatus::Active) {
            if (controller.is_ai_thinking()) {
                int dots = (static_cast<int>(animClock.getElapsedTime().asSeconds() * 3)) % 4;
                statusStr = "AI is thinking" + std::string(dots, '.');
                statusColor = sf::Color(100, 181, 246);
            } else {
                statusStr = (controller.get_turn() == chess::White) ? "White to move" : "Black to move";
                if (controller.is_in_check()) {
                    statusStr += "  [CHECK!]";
                    statusColor = sf::Color(255, 82, 82);
                }
            }
        } else if (status == chess::GameStatus::Checkmate) {
            statusStr = (controller.get_turn() == chess::White) ? "CHECKMATE! Black won!" : "CHECKMATE! White won!";
            statusColor = sf::Color(255, 215, 0);
        } else if (status == chess::GameStatus::Stalemate) {
            statusStr = "STALEMATE! Game is a draw.";
            statusColor = sf::Color(255, 215, 0);
        } else if (status == chess::GameStatus::DrawFiftyMoves) {
            statusStr = "DRAW by 50-move rule.";
            statusColor = sf::Color(255, 215, 0);
        } else if (status == chess::GameStatus::DrawThreefoldRepetition) {
            statusStr = "DRAW by threefold repetition.";
            statusColor = sf::Color(255, 215, 0);
        } else if (status == chess::GameStatus::Resigned) {
            statusStr = (controller.get_resigned_player() == chess::White) ? "White surrendered! Black won!" : "Black surrendered! White won!";
            statusColor = sf::Color(255, 179, 0);
        }

        sf::Text statusText(statusStr, titleFont, 18);
        statusText.setFillColor(statusColor);
        centerText(statusText, static_cast<float>(WindowWidth) / 2.0f, 58.0f);
        window.draw(statusText);

        // Toast feedback or quick helper text
        if (toastTimer > 0.0f && !toastMessage.empty()) {
            sf::Text toast(toastMessage, titleFont, 13);
            toast.setFillColor(toastColor);
            centerText(toast, static_cast<float>(WindowWidth) / 2.0f, 84.0f);
            window.draw(toast);
        } else {
            sf::Text hintText("Click to move | [S]ave [L]oad [F]lip [A]uto-flip [Q]Resign [R]eset [Esc]Exit", uiFont, 12);
            hintText.setFillColor(sf::Color(135, 135, 135));
            centerText(hintText, static_cast<float>(WindowWidth) / 2.0f, 84.0f);
            window.draw(hintText);
        }

        // 2. Draw Board Frame & Squares
        sf::RectangleShape boardFrame(sf::Vector2f(BoardSize + 6.0f, BoardSize + 6.0f));
        boardFrame.setPosition(boardLeft - 3.0f, boardTop - 3.0f);
        boardFrame.setFillColor(sf::Color(20, 20, 20));
        window.draw(boardFrame);

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                sf::RectangleShape square(sf::Vector2f(SquareSize, SquareSize));
                square.setPosition(boardLeft + static_cast<float>(col) * SquareSize,
                                   boardTop + static_cast<float>(row) * SquareSize);
                chess::Square sq = getSquareAtVisual(row, col);
                int parity = (sq / 8 + sq % 8) % 2;
                square.setFillColor((parity != 0) ? lightSquare : darkSquare);
                window.draw(square);
            }
        }

        // Rank labels (left)
        for (int row = 0; row < 8; ++row) {
            int rankNum = boardFlipped ? (1 + row) : (8 - row);
            sf::Text rankLabel(std::to_string(rankNum), uiFont, 13);
            rankLabel.setFillColor(sf::Color(160, 160, 160));
            centerText(rankLabel, boardLeft - 14.0f, boardTop + (static_cast<float>(row) + 0.5f) * SquareSize);
            window.draw(rankLabel);
        }

        // File labels (bottom)
        for (int col = 0; col < 8; ++col) {
            char fileCh = boardFlipped ? static_cast<char>('h' - col) : static_cast<char>('a' + col);
            std::string fileStr(1, fileCh);
            sf::Text fileLabel(fileStr, uiFont, 13);
            fileLabel.setFillColor(sf::Color(160, 160, 160));
            centerText(fileLabel, boardLeft + (static_cast<float>(col) + 0.5f) * SquareSize, boardTop + BoardSize + 14.0f);
            window.draw(fileLabel);
        }

        // 3. Last Move Highlight
        chess::Move_full_info lastMove = controller.get_last_move();
        if (!lastMove.is_no_move()) {
            for (chess::Square sq : {static_cast<chess::Square>(lastMove.from_square), static_cast<chess::Square>(lastMove.to_square)}) {
                auto [row, col] = getVisualAtSquare(sq);
                sf::RectangleShape lastMoveHighlight(sf::Vector2f(SquareSize, SquareSize));
                lastMoveHighlight.setPosition(boardLeft + static_cast<float>(col) * SquareSize,
                                             boardTop + static_cast<float>(row) * SquareSize);
                lastMoveHighlight.setFillColor(sf::Color(255, 235, 59, 80));
                window.draw(lastMoveHighlight);
            }
        }

        // 4. Selected Square Highlight
        chess::Square selectedSq = controller.get_selected_square();
        if (selectedSq != chess::No_Square) {
            auto [row, col] = getVisualAtSquare(selectedSq);
            sf::RectangleShape selectHighlight(sf::Vector2f(SquareSize, SquareSize));
            selectHighlight.setPosition(boardLeft + static_cast<float>(col) * SquareSize,
                                        boardTop + static_cast<float>(row) * SquareSize);
            selectHighlight.setFillColor(sf::Color(255, 215, 0, 130));
            window.draw(selectHighlight);
        }

        // 5. In-Check King Highlight
        if (controller.is_in_check() && status == chess::GameStatus::Active) {
            chess::Square kingSq = controller.get_king_square(controller.get_turn());
            if (kingSq != chess::No_Square) {
                auto [row, col] = getVisualAtSquare(kingSq);
                sf::RectangleShape checkHighlight(sf::Vector2f(SquareSize, SquareSize));
                checkHighlight.setPosition(boardLeft + static_cast<float>(col) * SquareSize,
                                           boardTop + static_cast<float>(row) * SquareSize);
                checkHighlight.setFillColor(sf::Color(244, 67, 54, 150));
                window.draw(checkHighlight);
            }
        }

        // 6. Draw Pieces
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                chess::Square sq = getSquareAtVisual(row, col);
                chess::Piece piece = controller.get_piece_at(sq);
                if (piece != chess::No_Piece) {
                    sf::Sprite sprite(pieceTextures[static_cast<size_t>(piece)]);
                    sprite.setScale(spriteScale, spriteScale);
                    sprite.setPosition(boardLeft + static_cast<float>(col) * SquareSize,
                                       boardTop + static_cast<float>(row) * SquareSize);
                    window.draw(sprite);
                }
            }
        }

        // 7. Legal Move Indicators
        if (selectedSq != chess::No_Square && status == chess::GameStatus::Active) {
            auto legalMoves = controller.get_legal_moves_from(selectedSq);
            for (const auto& m : legalMoves) {
                auto [row, col] = getVisualAtSquare(static_cast<chess::Square>(m.to_square));
                float centerX = boardLeft + (static_cast<float>(col) + 0.5f) * SquareSize;
                float centerY = boardTop + (static_cast<float>(row) + 0.5f) * SquareSize;

                if (controller.get_piece_at(static_cast<chess::Square>(m.to_square)) != chess::No_Piece || m.special == chess::SP_en_passant) {
                    constexpr float ringRadius = SquareSize * 0.42f;
                    sf::CircleShape ring(ringRadius);
                    ring.setOrigin(ringRadius, ringRadius);
                    ring.setPosition(centerX, centerY);
                    ring.setFillColor(sf::Color::Transparent);
                    ring.setOutlineThickness(4.0f);
                    ring.setOutlineColor(sf::Color(239, 83, 80, 200));
                    window.draw(ring);
                } else {
                    constexpr float dotRadius = SquareSize * 0.16f;
                    sf::CircleShape dot(dotRadius);
                    dot.setOrigin(dotRadius, dotRadius);
                    dot.setPosition(centerX, centerY);
                    dot.setFillColor(sf::Color(76, 175, 80, 210));
                    window.draw(dot);
                }
            }
        }

        // 8. Promotion Modal Dialog
        if (isPromoting) {
            sf::RectangleShape modalDim(sf::Vector2f(WindowWidth, WindowHeight));
            modalDim.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(modalDim);

            constexpr float modalWidth = 270.0f;
            constexpr float modalHeight = 100.0f;
            float modalX = (static_cast<float>(WindowWidth) - modalWidth) / 2.0f;
            float modalY = boardTop + (BoardSize - modalHeight) / 2.0f;

            sf::RectangleShape modalCard(sf::Vector2f(modalWidth, modalHeight));
            modalCard.setPosition(modalX, modalY);
            modalCard.setFillColor(sf::Color(45, 50, 56));
            modalCard.setOutlineThickness(2.0f);
            modalCard.setOutlineColor(sf::Color(255, 215, 0));
            window.draw(modalCard);

            sf::Text modalTitle("Choose Promotion Piece", titleFont, 14);
            modalTitle.setFillColor(sf::Color(240, 240, 240));
            centerText(modalTitle, modalX + modalWidth / 2.0f, modalY + 16.0f);
            window.draw(modalTitle);

            constexpr float boxSize = 56.0f;
            constexpr float boxPadding = 6.0f;
            float startX = modalX + (modalWidth - (4 * boxSize + 3 * boxPadding)) / 2.0f;
            float startY = modalY + 32.0f;

            bool isWhite = (controller.get_turn() == chess::White);
            std::array<chess::Piece, 4> promoPieces = isWhite ? 
                std::array<chess::Piece, 4>{chess::W_Queen, chess::W_Rook, chess::W_Bishop, chess::W_Knight} :
                std::array<chess::Piece, 4>{chess::B_Queen, chess::B_Rook, chess::B_Bishop, chess::B_Knight};

            for (size_t i = 0; i < 4; ++i) {
                float bx = startX + static_cast<float>(i) * (boxSize + boxPadding);
                float by = startY;

                sf::RectangleShape box(sf::Vector2f(boxSize, boxSize));
                box.setPosition(bx, by);
                box.setFillColor(sf::Color(60, 66, 74));
                box.setOutlineThickness(1.0f);
                box.setOutlineColor(sf::Color(100, 110, 120));
                window.draw(box);

                sf::Sprite pSprite(pieceTextures[static_cast<size_t>(promoPieces[i])]);
                pSprite.setScale(boxSize / pieceTextureWidth, boxSize / pieceTextureWidth);
                pSprite.setPosition(bx, by);
                window.draw(pSprite);
            }
        }

        // 9. Game Over Overlay Card
        if (status != chess::GameStatus::Active && !isPromoting) {
            constexpr float bannerWidth = 360.0f;
            constexpr float bannerHeight = 85.0f;
            float bannerX = (static_cast<float>(WindowWidth) - bannerWidth) / 2.0f;
            float bannerY = boardTop + (BoardSize - bannerHeight) / 2.0f;

            sf::RectangleShape banner(sf::Vector2f(bannerWidth, bannerHeight));
            banner.setPosition(bannerX, bannerY);
            banner.setFillColor(sf::Color(25, 27, 30, 240));
            banner.setOutlineThickness(2.0f);
            banner.setOutlineColor(statusColor);
            window.draw(banner);

            sf::Text overTitle(statusStr, titleFont, 16);
            overTitle.setFillColor(statusColor);
            centerText(overTitle, bannerX + bannerWidth / 2.0f, bannerY + 28.0f);
            window.draw(overTitle);

            sf::Text playAgain("Press [R] to Play Again", uiFont, 13);
            playAgain.setFillColor(sf::Color(200, 200, 200));
            centerText(playAgain, bannerX + bannerWidth / 2.0f, bannerY + 56.0f);
            window.draw(playAgain);
        }

        // 10. Toolbar Buttons
        auto drawButton = [&](const Button& btn) {
            bool isToggledOn = (btn.text.find(": ON") != std::string::npos);
            sf::RectangleShape bShape(sf::Vector2f(btn.bounds.width, btn.bounds.height));
            bShape.setPosition(btn.bounds.left, btn.bounds.top);
            if (!btn.active) {
                bShape.setFillColor(sf::Color(35, 38, 42));
                bShape.setOutlineColor(sf::Color(50, 53, 56));
            } else if (isToggledOn) {
                bShape.setFillColor(sf::Color(40, 56, 48));
                bShape.setOutlineColor(sf::Color(100, 180, 110));
            } else {
                bShape.setFillColor(sf::Color(45, 52, 60));
                bShape.setOutlineColor(sf::Color(70, 80, 92));
            }
            bShape.setOutlineThickness(1.0f);
            window.draw(bShape);

            sf::Text bText(btn.text, uiFont, 12);
            if (!btn.active) {
                bText.setFillColor(sf::Color(100, 100, 100));
            } else if (isToggledOn) {
                bText.setFillColor(sf::Color(165, 214, 167));
            } else {
                bText.setFillColor(sf::Color(220, 220, 220));
            }
            centerText(bText, btn.bounds.left + btn.bounds.width / 2.0f, btn.bounds.top + btn.bounds.height / 2.0f);
            window.draw(bText);
        };

        // Row 1
        drawButton(btnMode);
        drawButton(btnDiff);
        drawButton(btnColor);
        drawButton(btnFlip);
        drawButton(btnAutoFlip);

        // Row 2
        drawButton(btnSave);
        drawButton(btnLoad);
        drawButton(btnResign);
        drawButton(btnReset);

        window.display();
    }

    return EXIT_SUCCESS;
}