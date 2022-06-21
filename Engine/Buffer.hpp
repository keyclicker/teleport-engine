#pragma once

//#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>

#include <exception>
#include <stdexcept>

/**
 * @brief Screen canvas abstraction
 * 
 */
class Buffer {
private:
  uint16_t width, height;
  sf::Sprite sprite;
  sf::Texture texture;
  sf::Image image;

  sf::RenderWindow window;

public:
  Buffer(const Buffer &rhs) = delete;
  Buffer &operator=(const Buffer &rhs) = delete;

  /**
   * @brief Construct a new Buffer object
   * 
   * @param width Window width
   * @param height Window height
   * @param str Window title
   */
  Buffer(uint16_t width, uint16_t height, char *title):
          width(width), height(height),
          window(sf::RenderWindow(sf::VideoMode(width, height), title)){

    image.create(width, height);
  }

  /**
   * @brief Draw pixel with specified color
   * 
   * @param x x coordinate
   * @param y y coordinate
   * @param r red
   * @param g green
   * @param b blue
   */
  void setPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    image.setPixel(x, y, sf::Color(r, g, b));
  }

  /**
   * @brief Draw pixel with SFML Color object
   * 
   * @param x x coordinate
   * @param y y coordinate
   * @param color sf::Color
   */
  void setPixel(uint16_t x, uint16_t y, const sf::Color &color) {
    image.setPixel(x, y, color);
  }

  sf::Color getPixel(uint16_t x, uint16_t y) {
    return image.getPixel(x, y);
  }
  /**
   * @brief Displays changes and returns if window is open
   * 
   * @return true Window is open
   * @return false Window is closed
   */
  bool display() {
    texture.loadFromImage(image);
    sprite.setTexture(texture);
    window.draw(sprite);
    image.create(width, height);

    window.display();

    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();

    return window.isOpen();
  }

  /**
   * @brief Get window width
   * 
   * @return uint16_t Window width
   */
  uint16_t getWidth() const {
    return width;
  }

  /**
   * @brief Get window height
   * 
   * @return uint16_t Window height
   */
  uint16_t getHeight() const {
    return height;
  }

};