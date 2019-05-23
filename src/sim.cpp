#include <sim.h>
#include <GL/glew.h>
#include <iostream>

namespace sim
{
Simulation::Simulation(std::vector<Particle> particles, float particle_radius)
    : m_particles(particles),
      m_particle_radius(particle_radius),
      m_time(0.0f),
      m_particles_offset(particles)
{
    GLuint VBO;
    glGenBuffers(1, &VBO);

    // vertex array object
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_particles.size() * sizeof(Particle), &m_particles[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    m_vao_index = VAO;
    m_vbo_index = VBO;
}

void Simulation::draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_index);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_offset.size() * sizeof(Particle), &m_particles_offset[0]);
    glBindVertexArray(m_vao_index);
    glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_offset.size());
}

void Simulation::update_sinewave(float dt, float strength)
{
    m_time += dt;
    for (unsigned int i = 0; i < m_particles.size(); ++i)
    {
        m_particles_offset[i].position.y = m_particles[i].position.y + (strength * glm::sin(m_particles[i].position.x + m_particles[i].position.z + m_time));
    }
}

namespace scenarios
{
Simulation cube(float side_length, std::size_t particles_per_side, float particle_radius, glm::vec3 color1, glm::vec3 color2)
{
    std::vector<Particle> particles;
    particles.reserve(particles_per_side * particles_per_side * particles_per_side);
    float incr = side_length / (float)(particles_per_side - 1);
    float half_side = side_length / 2.0f;
    for (unsigned int x = 0; x < particles_per_side; ++x)
    {
        for (unsigned int y = 0; y < particles_per_side; ++y)
        {
            for (unsigned int z = 0; z < particles_per_side; ++z)
            {
                Particle p;
                p.position = glm::vec3(x * incr - half_side, y * incr - half_side, z * incr - half_side);

                float d = (float)(particles_per_side - 1) * 3;
                float t = x / d + y / d + z / d;
                p.color = t * color2 + (1 - t) * color1;

                particles.push_back(p);
            }
        }
    }

    return Simulation(particles, particle_radius);
}
} // namespace scenarios
} // namespace sim