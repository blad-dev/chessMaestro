#include <SFML/Graphics.hpp>
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "GameController.hpp"

namespace {
constexpr unsigned int WindowWidth = 840;
constexpr unsigned int WindowHeight = 720;
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
    "media_files/pieces/png/black_king.png"};

void centerText(sf::Text& text, float centerX, float centerY) {
  const sf::FloatRect bounds = text.getLocalBounds();
  text.setOrigin(bounds.left + bounds.width / 2.0f,
                 bounds.top + bounds.height / 2.0f);
  text.setPosition(centerX, centerY);
}

std::vector<sf::RectangleShape> createBoard(float left, float top) {
  std::vector<sf::RectangleShape> squares;
  squares.reserve(64);

  const sf::Color lightSquare(240, 217, 181);
  const sf::Color darkSquare(181, 136, 99);

  for (unsigned int row = 0; row < 8; ++row) {
    for (unsigned int column = 0; column < 8; ++column) {
      sf::RectangleShape square(sf::Vector2f(SquareSize, SquareSize));
      square.setPosition(left + static_cast<float>(column) * SquareSize,
                         top + static_cast<float>(row) * SquareSize);
      square.setFillColor(((row + column) % 2 == 0) ? lightSquare : darkSquare);
      squares.push_back(square);
    }
  }

  return squares;
}

struct PromotionOption {
  sf::FloatRect bounds;
  chess::Promotion promo;
  chess::Piece piece;
};

}  // namespace

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
      std::cerr << "Error: could not load piece texture: "
                << PieceTexturePaths[i] << '\n';
      return EXIT_FAILURE;
    }
    pieceTextures[i].setSmooth(true);
  }

  constexpr float boardLeft =
      (static_cast<float>(WindowWidth) - BoardSize) / 2.0f;
  constexpr float boardTop = 130.0f;
  const std::vector<sf::RectangleShape> boardSquares =
      createBoard(boardLeft, boardTop);

  chess::GameController controller;

  // Promotion pending state
  bool isPromoting = false;
  chess::Square promoFrom = chess::No_Square;
  chess::Square promoTo = chess::No_Square;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
          if (isPromoting) {
            isPromoting = false;
            controller.deselect();
          } else {
            window.close();
          }
        } else if (event.key.code == sf::Keyboard::R) {
          controller.reset_to_start();
          isPromoting = false;
        }
      } else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left) {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);

        if (isPromoting) {
          // Check promotion dialog clicks
          constexpr float modalWidth = 260.0f;
          constexpr float modalHeight = 90.0f;
          float modalX = (static_cast<float>(WindowWidth) - modalWidth) / 2.0f;
          float modalY = boardTop + (BoardSize - modalHeight) / 2.0f;
          constexpr float boxSize = 56.0f;
          constexpr float boxPadding = 6.0f;
          float startX =
              modalX + (modalWidth - (4 * boxSize + 3 * boxPadding)) / 2.0f;
          float startY = modalY + 24.0f;

          std::array<chess::Promotion, 4> promos = {
              chess::promote_to_queen, chess::promote_to_rook,
              chess::promote_to_bishop, chess::promote_to_knight};

          for (size_t i = 0; i < 4; ++i) {
            float bx = startX + static_cast<float>(i) * (boxSize + boxPadding);
            float by = startY;
            sf::FloatRect boxRect(bx, by, boxSize, boxSize);
            if (boxRect.contains(mx, my)) {
              controller.make_move(promoFrom, promoTo, promos[i]);
              isPromoting = false;
              break;
            }
          }
        } else if (controller.get_status() == chess::GameStatus::Active) {
          if (mx >= boardLeft && mx < boardLeft + BoardSize && my >= boardTop &&
              my < boardTop + BoardSize) {
            int col = static_cast<int>((mx - boardLeft) / SquareSize);
            int row = static_cast<int>((my - boardTop) / SquareSize);
            if (col >= 0 && col < 8 && row >= 0 && row < 8) {
              chess::Square clickedSq =
                  static_cast<chess::Square>((7 - row) * 8 + col);
              chess::Square currentSelected = controller.get_selected_square();

              if (currentSelected != chess::No_Square) {
                if (clickedSq == currentSelected) {
                  controller.deselect();
                } else {
                  // Check if clicked square is a legal destination
                  auto legalMoves =
                      controller.get_legal_moves_from(currentSelected);
                  bool isLegalDest = false;
                  for (const auto& m : legalMoves) {
                    if (m.to_square == clickedSq) {
                      isLegalDest = true;
                      break;
                    }
                  }

                  if (isLegalDest) {
                    if (controller.requires_promotion(currentSelected,
                                                      clickedSq)) {
                      isPromoting = true;
                      promoFrom = currentSelected;
                      promoTo = clickedSq;
                    } else {
                      controller.make_move(currentSelected, clickedSq);
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

    // 1. Draw Title & Status Header
    sf::Text titleText("ChessMaestro - Two Player Mode", titleFont, 22);
    titleText.setFillColor(sf::Color(240, 240, 240));
    centerText(titleText, static_cast<float>(WindowWidth) / 2.0f, 32.0f);

    std::string statusStr;
    sf::Color statusColor = sf::Color(180, 180, 180);
    const auto status = controller.get_status();

    if (status == chess::GameStatus::Active) {
      statusStr = (controller.get_turn() == chess::White) ? "White to move"
                                                          : "Black to move";
      if (controller.is_in_check()) {
        statusStr += "  [CHECK!]";
        statusColor = sf::Color(255, 82, 82);
      }
    } else if (status == chess::GameStatus::Checkmate) {
      statusStr = (controller.get_turn() == chess::White)
                      ? "CHECKMATE! Black won!"
                      : "CHECKMATE! White won!";
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
    }

    sf::Text statusText(statusStr, titleFont, 18);
    statusText.setFillColor(statusColor);
    centerText(statusText, static_cast<float>(WindowWidth) / 2.0f, 68.0f);

    sf::Text subText("[R] New Game   |   [Esc] Exit", uiFont, 13);
    subText.setFillColor(sf::Color(140, 140, 140));
    centerText(subText, static_cast<float>(WindowWidth) / 2.0f, 96.0f);

    // 2. Draw Board Board Border & Coordinates
    sf::RectangleShape boardFrame(
        sf::Vector2f(BoardSize + 6.0f, BoardSize + 6.0f));
    boardFrame.setPosition(boardLeft - 3.0f, boardTop - 3.0f);
    boardFrame.setFillColor(sf::Color(20, 20, 20));
    window.draw(boardFrame);

    for (const auto& square : boardSquares) {
      window.draw(square);
    }

    // Rank labels (1 to 8 on the left)
    for (int row = 0; row < 8; ++row) {
      sf::Text rankLabel(std::to_string(8 - row), uiFont, 13);
      rankLabel.setFillColor(sf::Color(160, 160, 160));
      centerText(rankLabel, boardLeft - 14.0f,
                 boardTop + (static_cast<float>(row) + 0.5f) * SquareSize);
      window.draw(rankLabel);
    }

    // File labels (a to h on the bottom)
    for (int col = 0; col < 8; ++col) {
      std::string fileStr(1, static_cast<char>('a' + col));
      sf::Text fileLabel(fileStr, uiFont, 13);
      fileLabel.setFillColor(sf::Color(160, 160, 160));
      centerText(fileLabel,
                 boardLeft + (static_cast<float>(col) + 0.5f) * SquareSize,
                 boardTop + BoardSize + 14.0f);
      window.draw(fileLabel);
    }

    // 3. Draw Last Move Highlight
    chess::Move_full_info lastMove = controller.get_last_move();
    if (!lastMove.is_no_move()) {
      for (chess::Square sq :
           {static_cast<chess::Square>(lastMove.from_square),
            static_cast<chess::Square>(lastMove.to_square)}) {
        int col = sq % 8;
        int row = 7 - (sq / 8);
        sf::RectangleShape lastMoveHighlight(
            sf::Vector2f(SquareSize, SquareSize));
        lastMoveHighlight.setPosition(
            boardLeft + static_cast<float>(col) * SquareSize,
            boardTop + static_cast<float>(row) * SquareSize);
        lastMoveHighlight.setFillColor(sf::Color(255, 235, 59, 80));
        window.draw(lastMoveHighlight);
      }
    }

    // 4. Draw Selected Square Highlight
    chess::Square selectedSq = controller.get_selected_square();
    if (selectedSq != chess::No_Square) {
      int col = selectedSq % 8;
      int row = 7 - (selectedSq / 8);
      sf::RectangleShape selectHighlight(sf::Vector2f(SquareSize, SquareSize));
      selectHighlight.setPosition(
          boardLeft + static_cast<float>(col) * SquareSize,
          boardTop + static_cast<float>(row) * SquareSize);
      selectHighlight.setFillColor(sf::Color(255, 215, 0, 130));
      window.draw(selectHighlight);
    }

    // 5. Draw In-Check King Highlight
    if (controller.is_in_check() && status == chess::GameStatus::Active) {
      chess::Square kingSq = controller.get_king_square(controller.get_turn());
      if (kingSq != chess::No_Square) {
        int col = kingSq % 8;
        int row = 7 - (kingSq / 8);
        sf::RectangleShape checkHighlight(sf::Vector2f(SquareSize, SquareSize));
        checkHighlight.setPosition(
            boardLeft + static_cast<float>(col) * SquareSize,
            boardTop + static_cast<float>(row) * SquareSize);
        checkHighlight.setFillColor(sf::Color(244, 67, 54, 150));
        window.draw(checkHighlight);
      }
    }

    // 6. Draw Pieces
    const float spriteScale = SquareSize / 512.0f;
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        chess::Square sq = static_cast<chess::Square>((7 - row) * 8 + col);
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

    // 7. Draw Legal Move Indicators (over pieces for clear visibility)
    if (selectedSq != chess::No_Square && status == chess::GameStatus::Active) {
      auto legalMoves = controller.get_legal_moves_from(selectedSq);
      for (const auto& m : legalMoves) {
        int col = m.to_square % 8;
        int row = 7 - (m.to_square / 8);
        float centerX =
            boardLeft + (static_cast<float>(col) + 0.5f) * SquareSize;
        float centerY =
            boardTop + (static_cast<float>(row) + 0.5f) * SquareSize;

        if (controller.get_piece_at(static_cast<chess::Square>(m.to_square)) !=
                chess::No_Piece ||
            m.special == chess::SP_en_passant) {
          // Capture target: outer ring
          constexpr float ringRadius = SquareSize * 0.42f;
          sf::CircleShape ring(ringRadius);
          ring.setOrigin(ringRadius, ringRadius);
          ring.setPosition(centerX, centerY);
          ring.setFillColor(sf::Color::Transparent);
          ring.setOutlineThickness(4.0f);
          ring.setOutlineColor(sf::Color(239, 83, 80, 200));
          window.draw(ring);
        } else {
          // Quiet move target: small translucent green dot
          constexpr float dotRadius = SquareSize * 0.16f;
          sf::CircleShape dot(dotRadius);
          dot.setOrigin(dotRadius, dotRadius);
          dot.setPosition(centerX, centerY);
          dot.setFillColor(sf::Color(76, 175, 80, 210));
          window.draw(dot);
        }
      }
    }

    // 8. Draw Promotion Modal Dialog
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
      float startX =
          modalX + (modalWidth - (4 * boxSize + 3 * boxPadding)) / 2.0f;
      float startY = modalY + 32.0f;

      bool isWhite = (controller.get_turn() == chess::White);
      std::array<chess::Piece, 4> promoPieces =
          isWhite
              ? std::array<chess::Piece, 4>{chess::W_Queen, chess::W_Rook,
                                            chess::W_Bishop, chess::W_Knight}
              : std::array<chess::Piece, 4>{chess::B_Queen, chess::B_Rook,
                                            chess::B_Bishop, chess::B_Knight};

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
        pSprite.setScale(boxSize / 256.0f, boxSize / 256.0f);
        pSprite.setPosition(bx, by);
        window.draw(pSprite);
      }
    }

    // 9. Draw Game Over Overlay Card
    if (status != chess::GameStatus::Active && !isPromoting) {
      constexpr float bannerWidth = 340.0f;
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

    window.display();
  }

  return EXIT_SUCCESS;
}