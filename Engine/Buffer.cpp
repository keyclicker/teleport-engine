#include "Buffer.hpp"

#include <stdexcept>
#include <string>

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

/**
 * @brief Compile one shader stage
 *
 * @throw std::runtime_error With the compiler log
 */
static GLuint compileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint ok;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    glDeleteShader(shader);
    throw std::runtime_error(std::string("Shader compilation failed: ") + log);
  }

  return shader;
}

/**
 * @brief Compile and link the present program
 *
 * @throw std::runtime_error With the compiler or linker log
 */
static GLuint buildProgram() {
  GLuint vertex = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER);
  GLuint fragment = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  // The program keeps them alive while attached
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  GLint ok;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);

  if (!ok) {
    char log[1024];
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);
    glDeleteProgram(program);
    throw std::runtime_error(std::string("Shader linking failed: ") + log);
  }

  return program;
}

// ---- Buffer ------------------------------------------------------------

Buffer::Buffer(uint16_t width, uint16_t height): frame(width, height) {
  program = buildProgram();

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
