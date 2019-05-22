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

static const float RADIUS = 1.0f;
static const float EPSILON = 1.5f * RADIUS;

static const unsigned int WINDOW_WIDTH = 800;
static const unsigned int WINDOW_HEIGHT = 600;
static const float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
static const float FOV = glm::radians(90.0f);
static const float Z_NEAR = 0.1f;
static const float Z_FAR = 20.0f;

void setupGBuffers(
    unsigned int &gBuffer,
    unsigned int &depthTexture,
    unsigned int &normalTexture,
    unsigned int &colorTexture);

GLuint setupRenderQuad();

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

    // enable point size variation and blending
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // pure additive blend, no alphas

    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);

    // compile shaders
    Shader shader_depth_pass("shaders/splat.vert", "shaders/splat_depth.frag");
    Shader shader_blend_pass("shaders/splat.vert", "shaders/splat_attributes.frag");
    shader_blend_pass.use();
    shader_blend_pass.setInt("g_depth", 0);
    Shader shader_render_pass("shaders/render.vert", "shaders/render.frag");
    shader_render_pass.use();
    shader_render_pass.setInt("g_normals", 0);
    shader_render_pass.setInt("g_colors", 1);

    // setup G-buffers
    unsigned int gBuffer, gDepthTex, gNormalTex, gColorTex;
    setupGBuffers(gBuffer, gDepthTex, gNormalTex, gColorTex);

    unsigned int quadVAO = setupRenderQuad();

    // scene setup

    sim::Simulation simulation = sim::scenarios::cube(5.0f, 3, RADIUS, glm::vec3(0.5, 0.8, 0.6), glm::vec3(0.8, 0.5, 0.7));

    glm::mat4 view = glm::lookAt(
        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::mat4 proj = glm::perspective(FOV, WINDOW_ASPECT_RATIO, Z_NEAR, Z_FAR);

    float t = 0.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        view = glm::lookAt(
            glm::vec3(2 * glm::sin(t), 2 + 1 * glm::cos(t), 5 + 2 * glm::cos(t)),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0));

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // depth pass
        shader_depth_pass.use();
        shader_depth_pass.setMat4("view_proj_matrix", proj * view);
        shader_depth_pass.setFloat("fov_y", FOV);
        shader_depth_pass.setVec2("viewport_size", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        shader_depth_pass.setFloat("epsilon", EPSILON);
        shader_depth_pass.setFloat("particle_radius", RADIUS);
        shader_depth_pass.setFloat("z_near", Z_NEAR);
        shader_depth_pass.setFloat("z_far", Z_FAR);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // only write to depth buffer
        simulation.draw();
        // attribute blend pass
        shader_blend_pass.use();
        shader_blend_pass.setMat4("view_proj_matrix", proj * view);
        shader_blend_pass.setFloat("fov_y", FOV);
        shader_blend_pass.setVec2("viewport_size", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        shader_blend_pass.setFloat("epsilon", EPSILON);
        shader_blend_pass.setFloat("particle_radius", RADIUS);
        shader_blend_pass.setFloat("z_near", Z_NEAR);
        shader_blend_pass.setFloat("z_far", Z_FAR);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gDepthTex);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // write to color buffers again
        glDepthMask(GL_FALSE);                           // but not to depth buffer
        glEnable(GL_BLEND);
        simulation.draw();
        // rendering pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader_render_pass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gNormalTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gColorTex);
        //shader_render_depth.setFloat("z_far", Z_FAR);
        //shader_render_depth.setFloat("z_near", Z_NEAR);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, gDepthTex);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        t += 0.01f;
    }

    glfwTerminate();
    return 0;
}

void setupGBuffers(
    unsigned int &gBuffer,
    unsigned int &depthTexture,
    unsigned int &normalTexture,
    unsigned int &colorTexture)
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // depth buffer
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // normal buffer
    glGenTextures(1, &normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTexture, 0);

    // color/weight buffer (colorR, colorG, colorB, weight)
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colorTexture, 0);

    unsigned int attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint setupRenderQuad()
{
    float verts[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    GLuint VBO;
    glGenBuffers(1, &VBO);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}
