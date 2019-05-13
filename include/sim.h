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

public:
    void prepare_rendering();
    void draw();

    Simulation(std::vector<Particle> particles, float particle_radius);

private:
    Simulation();
};

namespace scenarios
{
Simulation cube(float side_length, unsigned int particles_per_side, float particle_radius, glm::vec3 particle_color);
}

} // namespace sim

#endif // SIMULATION_H