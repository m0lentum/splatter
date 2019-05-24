#version 330 core

in VertOut {
    vec2 texcoords;
} i;

uniform sampler2D g_normals;
uniform sampler2D g_colors;

uniform vec3 light_dir_view_spc; // single global directional light
uniform vec3 light_color;
uniform float ambient_strength;
uniform float specular_strength;

out vec4 FragColor;

void main() {
    vec4 color_weighted = texture(g_colors, i.texcoords);
    vec3 color = color_weighted.rgb / color_weighted.a;
    vec3 normal = normalize(texture(g_normals, i.texcoords).xyz);

    vec3 light_dir = normalize(light_dir_view_spc);
    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = max(-dot(normal, light_dir), 0.0) * light_color;
    vec3 reflection = reflect(light_dir, normal);
    // we're in view space so view direction is the negative z-axis
    // therefore dot(view_dir, reflection) = reflection.z
    float specular_effect = pow(max(reflection.z, 0.0), 32);
    vec3 specular = specular_strength * specular_effect * light_color;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
    
    // normal visualization:
    //vec3 normal_vis = (normal + vec3(1.0, 1.0, 1.0)) / 2.0;
    //FragColor = vec4(normal_vis, 1.0);

    // color visualization:
    //FragColor = vec4(color, 1.0);
}