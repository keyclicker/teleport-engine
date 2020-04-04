#pragma once

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

#include <exception>
#include <stdexcept>

class Buffer {
private:
  GLushort ***buffer;

  uint16_t width, height;

public:
  GLFWwindow *window;


  Buffer(uint16_t width, uint16_t height, char *str):
  width(width), height(height) {

    buffer = new GLushort**[width];
    for (int i = 0; i < width; ++i) {
      buffer[i] = new GLushort*[height];
      for (int j = 0; j < height; ++j) {
        buffer[i][j] = new GLushort[3];
      }
    }

    /* Initialize the library */
    if (!glfwInit())
      throw std::runtime_error("glfwInit() failed!");

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, str, NULL, NULL);
    if (!window) {
      glfwTerminate();
      throw std::runtime_error("glfwCreateWindow() failed!");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);


    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);


    GLuint rb;
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
  }

  void makeCurrent() {
    glfwMakeContextCurrent(window);
  };

  bool isOpen() {
    return !glfwWindowShouldClose(window);
  }


  ~Buffer() {
    glfwTerminate();
    for (int i = 0; i < width; ++i) {
      delete[] buffer[i];
    }
    delete[] buffer;
  }
};