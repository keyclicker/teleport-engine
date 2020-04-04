#include "../Engine/Buffer.hpp"

int main() {


  Buffer bf(1280, 720, "hello");

  while (bf.isOpen()) {
    glClear(GL_COLOR_BUFFER_BIT);


    glBegin(GL_TRIANGLES);
    glColor3f(0.1, 0.2, 0.3);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glEnd();

    /* Swap front and back buffers */
    glfwSwapBuffers(bf.window);
    /* Poll for and process events */
    glfwPollEvents();
  }

  return 0;
}