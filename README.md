Project work for a computer graphics course.
Particles are rendered as spheres with blended normals and colors where they overlap,
and shaded with simple Blinn-Phong lighting with a single directional light.
This works with any particle-based simulation data;
as of now it's simply using a cube-shaped mock setup with some sine waves going on in it.

**Source**: Adams, B., Lenaerts, T. & Dutrè, P. (2006).
[Particle Splatting: Interactive Rendering of Particle-Based Simulation Data](http://graphics.cs.kuleuven.be/publications/PSIRPBSD/)

![demo.gif](demo.gif)

### Controls

* Esc - close the window
* S - start / stop camera spinning on its own
* mouse - move the camera manually
* Z - display mode: depth
* X - display mode: colors
* C - display mode: normals
* V - display mode: full
* Up arrow: increase particle size
* Down arrow: decrease particle size
* Right arrow: increase particle count
* Left arrow: reduce particle count

### Dependencies

* OpenGL: rendering
* GLWF: context creation
* GLEW: OpenGL extension managment
* GLM: math

Credit to learnopengl.com for the Shader class.

### Compiling

You'll need CMake and all the dependencies listed above, installed somewhere CMake can find them.
Alternatively for GLFW, GLEW and GLM, you can download the sources and set the CMake variables
GLFW_SRC_DIR, GLEW_SRC_DIR and/or GLM_SRC_DIR respectively.

If you really want to compile this yourself and need more detailed instructions,
bug me on [Twitter](https://twitter.com/moletrooper) and I might write some.
