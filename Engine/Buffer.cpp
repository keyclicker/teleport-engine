#include "Buffer.hpp"
#include "Shader.hpp"

// ---- Present shader ----------------------------------------------------

// Fullscreen triangle from gl_VertexID, so no vertex buffer is needed
static const char *VERTEX_SHADER = R"(
#version 330 core

out vec2 uv;

void main() {
  vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);

  // Frame rows go top to bottom, GL textures bottom to top
  uv = vec2(p.x, 1.0 - p.y);
  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

static const char *FRAGMENT_SHADER = R"(
#version 330 core

uniform sampler2D frame;

in vec2 uv;
out vec4 color;

void main() {
  vec4 c = texture(frame, uv);

  // Alpha holds brightness, see Color
  color = vec4(c.rgb * c.a, 1.0);
}
)";

// ---- Buffer ------------------------------------------------------------

Buffer::Buffer(uint16_t width, uint16_t height): frame(width, height) {
  program = buildProgram(VERTEX_SHADER, FRAGMENT_SHADER);

  // Core profile refuses to draw without a bound VAO, even an empty one
  glGenVertexArrays(1, &vao);

  // Allocate storage once, present() only overwrites it
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Buffer::~Buffer() {
  glDeleteTextures(1, &texture);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program);
}

void Buffer::present(int viewportWidth, int viewportHeight) {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWidth(), getHeight(),
                  GL_RGBA, GL_UNSIGNED_BYTE, frame.data());

  // The triangle covers every pixel, so no glClear
  glViewport(0, 0, viewportWidth, viewportHeight);
  glUseProgram(program);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  frame.fill({});
}
