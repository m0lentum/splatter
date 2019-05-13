#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <shader.h>
#include <sim.h>

static const float EPSILON = 0.0f; // TODO figure out what this should be
static const float RADIUS = 0.3f;

static const unsigned int WINDOW_WIDTH = 640;
static const unsigned int WINDOW_HEIGHT = 480;
static const float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
static const float FOV = glm::radians(90.0f);

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

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "fluid \"simulation\"", NULL, NULL);

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

    // depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // enable point size variation
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // compile shader
    Shader shader("shaders/splat.vert", "shaders/splat_depth.frag");

    // scene setup

    sim::Simulation simulation = sim::scenarios::cube(5.0f, 20, RADIUS, glm::vec3(0.5, 0.8, 0.6));

    glm::mat4 view = glm::lookAt(
        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::mat4 proj = glm::perspective(FOV, WINDOW_ASPECT_RATIO, 0.1f, 100.0f);

    float t = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = glm::lookAt(
            glm::vec3(2 * glm::sin(t), 2 + 1 * glm::cos(t), 5 + 2 * glm::cos(t)),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0));

        shader.use();
        shader.setMat4("view_proj_matrix", proj * view);
        shader.setFloat("fov_y", FOV);
        shader.setFloat("viewport_height", WINDOW_HEIGHT);
        shader.setFloat("epsilon", EPSILON);
        shader.setFloat("particle_radius", RADIUS);
        simulation.draw();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        t += 0.01f;
    }

    glfwTerminate();
    return 0;
}
