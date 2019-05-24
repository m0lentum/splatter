#ifndef SIMULATION_H
#define SIMULATION_H

#include <glm/glm.hpp>
#include <vector>

namespace sim
{
struct Particle
{
    glm::vec3 position;
    glm::vec3 color;
};

class Simulation
{
private:
    std::vector<Particle> m_particles;
    float m_particle_radius;
    unsigned int m_vao_id;
    unsigned int m_vbo_id;
    bool m_has_vao;

    // for artificial motion until I get an actual simulation together
    float m_time;
    std::vector<Particle> m_particles_offset;

public:
    void prepareRendering();
    void draw();

    void updateOffsetsSinewave(float dt, float strength);
    void setParticles(std::vector<Particle> particles);

    void initialize();
    Simulation(std::vector<Particle> particles, float particle_radius, bool initialize_now = true);

private:
    Simulation();
};

namespace scenarios
{
std::vector<Particle> cube(float side_length, std::size_t particles_per_side, glm::vec3 color1, glm::vec3 color2);
}

} // namespace sim

#endif // SIMULATION_H