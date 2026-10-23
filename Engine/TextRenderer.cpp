#include "TextRenderer.hpp"
#include "Shader.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <stb_easy_font.h>

// ---- Text shader -------------------------------------------------------

static const char *VERTEX_SHADER = R"(
#version 330 core

uniform vec2 viewport;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

out vec4 vColor;

void main() {
  // Pixels with y down to clip space with y up
  vec2 p = position / viewport * 2.0 - 1.0;
  gl_Position = vec4(p.x, -p.y, 0.0, 1.0);

  vColor = color;
}
)";

static const char *FRAGMENT_SHADER = R"(
#version 330 core

in vec4 vColor;
out vec4 color;

void main() {
  color = vec4(vColor.rgb, 1.0);
}
)";

// Upper bound per character: the busiest glyph, `X`, needs 11 quads
static constexpr size_t MAX_QUADS_PER_CHAR = 16;

// ---- TextRenderer ------------------------------------------------------

TextRenderer::TextRenderer() {
  static_assert(sizeof(Vertex) == 16, "Vertex must match stb_easy_font");

  program = buildProgram(VERTEX_SHADER, FRAGMENT_SHADER);
  viewportUniform = glGetUniformLocation(program, "viewport");

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  // The VAO remembers the attribute layout and the element buffer
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, x));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                        (void *) offsetof(Vertex, color));

  glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program);
}

void TextRenderer::print(const std::string &text, float x, float y,
                         float scale, Color color) {
  append(text, x + scale, y + scale, scale, {0, 0, 0});
  append(text, x, y, scale, color);
}

void TextRenderer::append(const std::string &text, float x, float y,
                          float scale, Color color) {
  // stb wants mutable pointers, and truncates if the buffer is short
  std::string chars = text;
  std::vector<Vertex> glyphs(chars.size() * MAX_QUADS_PER_CHAR * 4);

  int quads = stb_easy_font_print(0, 0, chars.data(), &color.r,
                                  glyphs.data(),
                                  glyphs.size() * sizeof(Vertex));

  for (int i = 0; i < quads * 4; ++i) {
    auto v = glyphs[i];
    vertices.push_back({x + v.x * scale, y + v.y * scale, 0, v.color});
  }
}

void TextRenderer::reserveIndices(size_t quads) {
  if (quads <= indexedQuads) return;

  // Two triangles per quad: 0 1 2, 0 2 3
  indexedQuads = std::max(quads, indexedQuads * 2);
  std::vector<uint32_t> indices;
  indices.reserve(indexedQuads * 6);

  for (uint32_t q = 0; q < indexedQuads; ++q) {
    for (uint32_t corner : {0, 1, 2, 0, 2, 3}) {
      indices.push_back(q * 4 + corner);
    }
  }

  glBindVertexArray(vao);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
               indices.data(), GL_STATIC_DRAW);
}

void TextRenderer::render(int viewportWidth, int viewportHeight) {
  if (vertices.empty()) return;

  size_t quads = vertices.size() / 4;
  reserveIndices(quads);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STREAM_DRAW);

  glViewport(0, 0, viewportWidth, viewportHeight);
  glUseProgram(program);
  glUniform2f(viewportUniform, viewportWidth, viewportHeight);
  glDrawElements(GL_TRIANGLES, quads * 6, GL_UNSIGNED_INT, nullptr);

  vertices.clear();
}
