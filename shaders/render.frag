#version 330 core

in VertOut {
    vec2 texcoords;
} i;

uniform sampler2D g_normals;
uniform sampler2D g_colors;

out vec4 FragColor;

void main() {
    vec4 cw = texture(g_colors, i.texcoords);
    vec3 color = cw.rgb / cw.a;
    //if (cw.a < 0.05) {
    //    discard;
    //}
    vec3 normal = normalize(texture(g_normals, i.texcoords).xyz);
    vec3 normal_vis = (normal + vec3(1.0, 1.0, 1.0)) / 2.0;
    //FragColor = vec4(cw.a, cw.a, cw.a, 1.0);
    //FragColor = vec4(color, 1.0);
    FragColor = vec4(normal_vis, 1.0);
}