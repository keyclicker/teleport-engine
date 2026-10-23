#pragma once

#include "Window.hpp"

/**
 * @brief Compile and link a vertex + fragment shader program
 *
 * Needs a current GL context, see Window.
 *
 * @param vertexSource Vertex shader GLSL source
 * @param fragmentSource Fragment shader GLSL source
 * @return GLuint Linked program, delete it with glDeleteProgram
 * @throw std::runtime_error With the compiler or linker log
 */
GLuint buildProgram(const char *vertexSource, const char *fragmentSource);
