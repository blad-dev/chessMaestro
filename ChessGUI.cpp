#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr unsigned int WindowWidth = 800;
constexpr unsigned int WindowHeight = 700;
constexpr float BoardSize = 480.0f;
constexpr float SquareSize = BoardSize / 8.0f;

const std::string FontPath = "media_files/fonts/Roboto/Roboto-Regular.ttf";
const std::string WhiteKnightPath = "media_files/pieces/png/white_knight.png";

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
}  // namespace

int main() {
  // Create the native SFML 2.6.2 window and cap rendering at 60 FPS.
  sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight),
                          "Chess GUI - SFML 2.6.2",
                          sf::Style::Titlebar | sf::Style::Close);

  if (!window.isOpen()) {
    std::cerr << "Error: failed to create the SFML window.\n";
    return EXIT_FAILURE;
  }

  window.setFramerateLimit(60);
  window.setKeyRepeatEnabled(false);

  // Keep these resources alive for as long as their sf::Text/sf::Sprite objects
  // exist.
  sf::Font uiFont;
  if (!uiFont.loadFromFile(FontPath)) {
    std::cerr << "Error: could not load font: " << FontPath << '\n'
              << "Run the application from the project root directory.\n";
    return EXIT_FAILURE;
  }

  sf::Texture knightTexture;
  if (!knightTexture.loadFromFile(WhiteKnightPath)) {
    std::cerr << "Error: could not load piece image: " << WhiteKnightPath
              << '\n'
              << "Run the application from the project root directory.\n";
    return EXIT_FAILURE;
  }
  knightTexture.setSmooth(true);

  constexpr float boardLeft =
      (static_cast<float>(WindowWidth) - BoardSize) / 2.0f;
  constexpr float boardTop = 135.0f;
  const std::vector<sf::RectangleShape> boardSquares =
      createBoard(boardLeft, boardTop);

  sf::Sprite whiteKnight(knightTexture);
  const sf::Vector2u textureSize = knightTexture.getSize();
  if (textureSize.x == 0 || textureSize.y == 0) {
    std::cerr << "Error: the white knight image has invalid dimensions.\n";
    return EXIT_FAILURE;
  }
  const float scaleFactor = SquareSize / static_cast<float>(textureSize.x);
  whiteKnight.setScale(scaleFactor, scaleFactor);
  // Position on square b1 (column 1, row 7)
  whiteKnight.setPosition(boardLeft + 1.0f * SquareSize,
                          boardTop + 7.0f * SquareSize);

  // Setup sample text using loaded Roboto font
  sf::Text headerText("ChessMaestro - Milestone 1 Sanity Check", uiFont, 24);
  headerText.setFillColor(sf::Color(220, 220, 220));
  centerText(headerText, static_cast<float>(WindowWidth) / 2.0f, 50.0f);

  sf::Text instructionText("SFML 2.6.2 & Assets Verified. Press [Esc] to exit.",
                           uiFont, 16);
  instructionText.setFillColor(sf::Color(160, 160, 160));
  centerText(instructionText, static_cast<float>(WindowWidth) / 2.0f, 90.0f);

  // --- Main Event & Render Loop ---
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape) {
        window.close();
      }
    }

    // Render frame
    window.clear(sf::Color(35, 39, 42));  // Dark background

    // 1. Draw board squares
    for (const auto& square : boardSquares) {
      window.draw(square);
    }

    // 2. Draw piece sprite
    window.draw(whiteKnight);

    // 3. Draw text
    window.draw(headerText);
    window.draw(instructionText);

    window.display();
  }

  return EXIT_SUCCESS;
}