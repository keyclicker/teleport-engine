#include "Shader.hpp"

#include <stdexcept>
#include <string>

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

GLuint buildProgram(const char *vertexSource, const char *fragmentSource) {
  GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
  GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

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
