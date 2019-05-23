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
    unsigned int m_vao_index;
    unsigned int m_vbo_index;

    // for artificial motion until I get an actual simulation together
    float m_time;
    std::vector<Particle> m_particles_offset;

public:
    void prepare_rendering();
    void draw();

    void update_sinewave(float dt, float strength);

    Simulation(std::vector<Particle> particles, float particle_radius);

private:
    Simulation();
};

namespace scenarios
{
Simulation cube(float side_length, std::size_t particles_per_side, float particle_radius, glm::vec3 color1, glm::vec3 color2);
}

} // namespace sim

#endif // SIMULATION_H