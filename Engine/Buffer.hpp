#pragma once

#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>

#include <exception>
#include <stdexcept>


class Buffer {
private:
  uint16_t width, height;
  sf::Sprite sprite;
  sf::Texture texture;
  sf::Image image;

public:
  sf::RenderWindow window;

  Buffer(const Buffer &rhs) = delete;
  Buffer &operator=(const Buffer &rhs) = delete;

  Buffer(uint16_t width, uint16_t height, char *str):
          width(width), height(height),
          window(sf::RenderWindow(sf::VideoMode(width, height), str)){

    image.create(width, height);
  }

  void setPixel(uint16_t x, uint16_t y, GLushort r, GLushort g, GLushort b) {
    image.setPixel(x, y, sf::Color(r, g, b));
  }

  bool isOpen() {
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

  uint16_t getWidth() const {
    return width;
  }

  uint16_t getHeight() const {
    return height;
  }

};


//class Buffer {
//private:
//  GLushort ***buffer;
//  uint16_t width, height;
//  GLFWwindow *window;
//
//public:
//  Buffer(uint16_t width, uint16_t height, char *str):
//  width(width), height(height) {
//
//    buffer = new GLushort**[width];
//    for (int i = 0; i < width; ++i) {
//      buffer[i] = new GLushort*[height];
//      for (int j = 0; j < height; ++j) {
//        buffer[i][j] = new GLushort[3];
//      }
//    }
//
//    /* Initialize the library */
//    if (!glfwInit())
//      throw std::runtime_error("glfwInit() failed!");
//
//    /* Create a windowed mode window and its OpenGL context */
//    window = glfwCreateWindow(width, height, str, NULL, NULL);
//    if (!window) {
//      glfwTerminate();
//      throw std::runtime_error("glfwCreateWindow() failed!");
//    }
//
//    /* Make the window's context current */
//    glfwMakeContextCurrent(window);
//
//
//    GLuint fbo;
//    glGenFramebuffers(1, &fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//
//
//    GLuint rb;
//    glGenRenderbuffers(1, &rb);
//    glBindRenderbuffer(GL_RENDERBUFFER, rb);
//  }
//
//  void drawPixel(uint16_t x, uint8_t y, GLushort r, GLushort g, GLushort b) {
//    buffer[x][y][0] = r;
//    buffer[x][y][1] = g;
//    buffer[x][y][2] = b;
//  }
//
//  void makeCurrent() {
//    glfwMakeContextCurrent(window);
//  };
//
//  bool isOpen() {
//    /* Swap front and back buffers */
//    glfwSwapBuffers(window);
//    /* Poll for and process events */
//    glfwPollEvents();
//
//    glClear(GL_COLOR_BUFFER_BIT);
//    return !glfwWindowShouldClose(window);
//  }
//
//
//  ~Buffer() {
//    glfwTerminate();
//    for (int i = 0; i < width; ++i) {
//      delete[] buffer[i];
//    }
//    delete[] buffer;
//  }
//};