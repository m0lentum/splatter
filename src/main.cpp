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
#include <camera.h>

// particles

static float particle_radius = 0.5f;
static const float EPSILON_MULTIPLIER = 0.25f;
static std::size_t particle_count = 20;
static const glm::vec3 COLOR1 = glm::vec3(0.5f, 0.8f, 0.6f);
static const glm::vec3 COLOR2 = glm::vec3(0.1f, 0.6f, 0.8f);

static sim::Simulation simulation(sim::scenarios::cube(5.0f, particle_count, COLOR1, COLOR2), particle_radius, false);

void incrementParticleRadius(float r)
{
    particle_radius += r;
}

void incrementParticleCount(std::size_t c)
{
    particle_count += c;
    simulation.setParticles(sim::scenarios::cube(5.0f, particle_count, COLOR1, COLOR2));
}

// lighting

static const glm::vec3 LIGHT_DIR = glm::normalize(glm::vec3(-0.5f, -1.0f, 0.4f));
static const glm::vec3 LIGHT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f);
static const float AMBIENT_STRENGTH = 0.5f;
static const float SPECULAR_STRENGTH = 0.2f;

// window

static const unsigned int WINDOW_WIDTH = 800;
static const unsigned int WINDOW_HEIGHT = 600;
static const float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
static const float FOV = glm::radians(90.0f);
static const float Z_NEAR = 0.1f;
static const float Z_FAR = 15.0f;

// camera

static Camera cam(8.0f, 0.12f, 0.2f, -30.0f, 45.0f);
static bool cam_is_spinning = true;
static double last_mouse_x = 0.0f;
static double last_mouse_y = 0.0f;
static bool mouse_initialized = false;

enum class DisplayMode
{
    DEPTH,
    COLORS,
    NORMALS,
    FULL
};
static DisplayMode display_mode = DisplayMode::FULL;

// forward declarations

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window, double xpos, double ypos);

void setupGBuffers(
    unsigned int &gBuffer,
    unsigned int &depthTexture,
    unsigned int &normalTexture,
    unsigned int &colorTexture);

GLuint setupRenderQuad();

//

int main(void)
{
    GLFWwindow *window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "fluid \"simulation\"", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

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
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT); // y up
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // pure additive blend, no alpha multiplication

    // compile shaders
    Shader shader_depth_pass("shaders/splat.vert", "shaders/splat_depth.frag");
    Shader shader_blend_pass("shaders/splat.vert", "shaders/splat_attributes.frag");
    shader_blend_pass.use();
    shader_blend_pass.setInt("g_depth", 0);
    Shader shader_render_depth("shaders/render.vert", "shaders/render_depth.frag");
    shader_render_depth.use();
    shader_render_depth.setInt("g_depths", 0);
    Shader shader_render_colors("shaders/render.vert", "shaders/render_colors.frag");
    shader_render_colors.use();
    shader_render_colors.setInt("g_colors", 0);
    Shader shader_render_normals("shaders/render.vert", "shaders/render_normals.frag");
    shader_render_normals.use();
    shader_render_normals.setInt("g_normals", 0);
    Shader shader_render_full("shaders/render.vert", "shaders/render.frag");
    shader_render_full.use();
    shader_render_full.setInt("g_normals", 0);
    shader_render_full.setInt("g_colors", 1);

    // setup G-buffers
    unsigned int gBuffer, gDepthTex, gNormalTex, gColorTex;
    setupGBuffers(gBuffer, gDepthTex, gNormalTex, gColorTex);

    unsigned int quadVAO = setupRenderQuad();

    //

    glm::mat4 proj = glm::perspective(FOV, WINDOW_ASPECT_RATIO, Z_NEAR, Z_FAR);
    simulation.initialize();

    while (!glfwWindowShouldClose(window))
    {
        if (cam_is_spinning)
            cam.handleMouse(-2.5f, 0.0f);

        glm::mat4 view = cam.getViewMatrix();
        float epsilon = EPSILON_MULTIPLIER * particle_radius;
        simulation.updateOffsetsSinewave(0.02f, 0.3f);

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
        shader_depth_pass.setFloat("epsilon", epsilon);
        shader_depth_pass.setFloat("particle_radius", particle_radius);
        shader_depth_pass.setFloat("z_near", Z_NEAR);
        shader_depth_pass.setFloat("z_far", Z_FAR);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // only write to depth buffer
        simulation.draw();

        // attribute blend pass
        shader_blend_pass.use();
        shader_blend_pass.setMat4("view_proj_matrix", proj * view);
        shader_blend_pass.setFloat("fov_y", FOV);
        shader_blend_pass.setVec2("viewport_size", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        shader_blend_pass.setFloat("epsilon", epsilon);
        shader_blend_pass.setFloat("particle_radius", particle_radius);
        shader_blend_pass.setFloat("z_near", Z_NEAR);
        shader_blend_pass.setFloat("z_far", Z_FAR);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gDepthTex);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // write to color buffers again
        glDepthMask(GL_FALSE);                           // but not to depth buffer
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        simulation.draw();

        // rendering pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        switch (display_mode)
        {
        case DisplayMode::DEPTH:
            shader_render_depth.use();
            shader_render_depth.setFloat("z_far", Z_FAR);
            shader_render_depth.setFloat("z_near", Z_NEAR);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gDepthTex);
            break;
        case DisplayMode::COLORS:
            shader_render_colors.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gColorTex);
            break;
        case DisplayMode::NORMALS:
            shader_render_normals.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gNormalTex);
            break;
        case DisplayMode::FULL:
            glm::vec4 ld_view = view * glm::vec4(LIGHT_DIR, 0.0);
            shader_render_full.use();
            shader_render_full.setVec3("light_dir_view_spc", glm::vec3(ld_view));
            shader_render_full.setVec3("light_color", LIGHT_COLOR);
            shader_render_full.setFloat("ambient_strength", AMBIENT_STRENGTH);
            shader_render_full.setFloat("specular_strength", SPECULAR_STRENGTH);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gNormalTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gColorTex);
            break;
        }

        glDisable(GL_BLEND);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
    case GLFW_KEY_ESCAPE: // ESC: close window
        glfwSetWindowShouldClose(window, true);
        break;
    case GLFW_KEY_S: // S: make camera spin
        cam_is_spinning = !cam_is_spinning;
        break;
    case GLFW_KEY_Z: // Z: depth visualization
        display_mode = DisplayMode::DEPTH;
        break;
    case GLFW_KEY_X: // X: color visualization
        display_mode = DisplayMode::COLORS;
        break;
    case GLFW_KEY_C: // C: normal visualization
        display_mode = DisplayMode::NORMALS;
        break;
    case GLFW_KEY_V: // V: full visualization
        display_mode = DisplayMode::FULL;
        break;
    case GLFW_KEY_UP: // up arrow: bigger particles
        incrementParticleRadius(0.1f);
        break;
    case GLFW_KEY_DOWN: // down arrow: smaller particles
        incrementParticleRadius(-0.1f);
        break;
    case GLFW_KEY_RIGHT: // right arrow: more particles
        incrementParticleCount(1);
        break;
    case GLFW_KEY_LEFT: // left arrow: less particles
        incrementParticleCount(-1);
        break;
    }
}

void mouseCallback(GLFWwindow *window, double x_pos, double y_pos)
{
    if (!mouse_initialized)
    {
        last_mouse_x = x_pos;
        last_mouse_y = y_pos;
        mouse_initialized = true;
        return;
    }

    double x_diff = x_pos - last_mouse_x;
    double y_diff = y_pos - last_mouse_y;
    last_mouse_x = x_pos;
    last_mouse_y = y_pos;

    cam.handleMouse((float)x_diff, (float)y_diff);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    cam.handleScroll((float)yoffset);
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
    // using RGBA because RGB doesn't work on my laptop, A is not actually used for anything
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
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
