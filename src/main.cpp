#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <shader.h>
#include <iostream>

#include <string>
#include <fstream>
#include <streambuf>

static const GLfloat cube_verts[] = {
    // position x, y, z, color r, g, b
    -1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 0.5f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 0.5f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.5f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.5f, 0.0f, 0.0f};

static const GLuint cube_indices[] = {
    0, 1, 3, 2, 3, 1,
    0, 4, 1, 5, 1, 4,
    0, 3, 4, 7, 4, 3,
    3, 2, 7, 6, 7, 2,
    6, 2, 5, 1, 5, 2,
    6, 5, 7, 4, 7, 5};

static const unsigned int WINDOW_WIDTH = 640;
static const unsigned int WINDOW_HEIGHT = 480;
static const float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(void)
{
    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "spinning cube", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error initializing GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // vertex buffer
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), cube_verts, GL_DYNAMIC_DRAW);

    // index buffer
    GLuint EB;
    glGenBuffers(1, &EB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_DYNAMIC_DRAW);

    // vertex array object
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), cube_verts, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // compile shader
    Shader shader("shaders/spinner.vert", "shaders/vertColor.frag");

    // scene setup

    glm::mat4 view = glm::lookAt(
        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::mat4 proj = glm::perspective(glm::radians(90.0f), WINDOW_ASPECT_RATIO, 0.1f, 10.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = proj * view * model;

    float t = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("MVP", mvp);
        shader.setFloat("t", t);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        t += 0.02f;
    }

    glfwTerminate();
    return 0;
}
