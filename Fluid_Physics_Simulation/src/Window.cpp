#include "../HeaderFiles/Window.h"

//Defining static members
unsigned int Window::vao = 0;
unsigned int Window::vbo = 0;

Window :: Window(int w, int h, bool waitVSync) {
    glfwInit();

    GLFWwindow* window;
    width = w;
    height = h;
    aspectRatio = (float)width / (float)height;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
    }
    win = window;

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (waitVSync)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

#if USE_CPP_IOSTREAM
    if (glewInit() != GLEW_OK) std::cout << "Error!" << std::endl;
#else
    if (glewInit() != GLEW_OK) printf( "Error!\n" );
#endif
}

void Window::drawBoundary(int object_Location, int color_Location) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), recData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform3f(color_Location, 1.0f, 1.0f, 1.0f);

    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}