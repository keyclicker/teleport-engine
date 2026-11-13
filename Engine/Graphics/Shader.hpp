#pragma once

#include "../Core/Window.hpp"

namespace te {

/**
 * @brief Compile and link a vertex + fragment shader program
 *
 * Needs a current GL context, see Window. Sources have no `#version`
 * line: it's prepended to match the context, GLSL 3.30 or ES 3.00.
 *
 * @param vertexSource Vertex shader GLSL source
 * @param fragmentSource Fragment shader GLSL source
 * @return GLuint Linked program, delete it with glDeleteProgram
 * @throw std::runtime_error With the compiler or linker log
 */
GLuint buildProgram(const char *vertexSource, const char *fragmentSource);

} // namespace te
