#include "Screen.hpp"
#include "Shader.hpp"

namespace te {

// ---- Present shader ----------------------------------------------------

// Fullscreen triangle from gl_VertexID, so no vertex buffer is needed
static const char *VERTEX_SHADER = R"(
out vec2 uv;

void main() {
  vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);

  // Frame rows go top to bottom, GL textures bottom to top
  uv = vec2(p.x, 1.0 - p.y);
  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

// The frame is column-major, so its texture is transposed: swap uv
static const char *FRAGMENT_SHADER = R"(
uniform sampler2D frame;

in vec2 uv;
out vec4 color;

void main() {
  vec4 c = texture(frame, uv.yx);

  // Alpha holds brightness, see Color
  color = vec4(c.rgb * c.a, 1.0);
}
)";

// ---- Screen ------------------------------------------------------------

Screen::Screen() {
  program = buildProgram(VERTEX_SHADER, FRAGMENT_SHADER);

  // Core profile refuses to draw without a bound VAO, even an empty one
  glGenVertexArrays(1, &vao);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Screen::~Screen() {
  glDeleteTextures(1, &texture);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program);
}

void Screen::present(const Frame &frame, int viewportWidth,
                     int viewportHeight) {
  // Texture rows are frame columns
  int texWidth = frame.getHeight();
  int texHeight = frame.getWidth();

  glBindTexture(GL_TEXTURE_2D, texture);

  // Reallocate only on resize, otherwise overwrite in place
  if (texWidth != textureWidth || texHeight != textureHeight) {
    textureWidth = texWidth;
    textureHeight = texHeight;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight,
                  GL_RGBA, GL_UNSIGNED_BYTE, frame.data());

  // The triangle covers every pixel, so no glClear
  glViewport(0, 0, viewportWidth, viewportHeight);
  glUseProgram(program);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

} // namespace te
