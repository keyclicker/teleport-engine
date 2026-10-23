#pragma once

#include <string>
#include <vector>

#include "Image.hpp"
#include "Window.hpp"

/**
 * @brief Draws debug text over the frame, in framebuffer pixels
 *
 * Glyphs are stb_easy_font quads: ASCII only, no font file. Queue
 * strings with print(), then draw them in one call with render().
 * Needs a current GL context for its whole lifetime, see Window.
 */
class TextRenderer {
private:
  // stb_easy_font vertex layout, z is unused
  struct Vertex {
    float x, y, z;
    Color color;
  };

  // Queued glyph quads, 4 vertices each
  std::vector<Vertex> vertices;

  GLuint program = 0;
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ebo = 0;
  GLint viewportUniform = -1;

  // Quads the element buffer has indices for
  size_t indexedQuads = 0;

  /**
   * @brief Queue one layer of text, see print()
   */
  void append(const std::string &text, float x, float y, float scale,
              Color color);

  /**
   * @brief Grow the element buffer to index at least `quads` quads
   */
  void reserveIndices(size_t quads);

public:
  TextRenderer(const TextRenderer &rhs) = delete;
  TextRenderer &operator=(const TextRenderer &rhs) = delete;

  /**
   * @brief Construct a new TextRenderer object
   *
   * @throw std::runtime_error The text shader failed to build
   */
  TextRenderer();

  ~TextRenderer();

  /**
   * @brief Queue text with a drop shadow for the next render()
   *
   * @param text ASCII text, '\n' starts a new line
   * @param x Left edge in framebuffer pixels
   * @param y Top edge in framebuffer pixels, y goes down
   * @param scale Font pixel size in framebuffer pixels, a glyph is
   *        6 by 12 font pixels
   * @param color Text color, alpha is ignored
   */
  void print(const std::string &text, float x, float y, float scale,
             Color color);

  /**
   * @brief Draw and dequeue all text printed since the last call
   *
   * Call after Buffer::present(), before Window::display().
   *
   * @param viewportWidth Target framebuffer width in pixels
   * @param viewportHeight Target framebuffer height in pixels
   */
  void render(int viewportWidth, int viewportHeight);
};
