#version 330 core

in VertOut {
    vec2 texcoords;
} i;

uniform sampler2D tex_normals;
uniform sampler2D tex_colors;

out vec4 FragColor;

void main() {
    vec4 cw = texture(tex_colors, i.texcoords);
    vec3 color = cw.rgb / cw.a;
    vec3 normal = normalize(texture(tex_normals, i.texcoords).xyz);
    vec3 normal_vis = (normal + vec3(1.0, 1.0, 1.0)) / 2.0;
    //FragColor = vec4(color, 1.0);
    FragColor = vec4(normal_vis, 1.0);
}