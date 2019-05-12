#version 330 core

in VertOut {
    float radius;
    vec4 color;
} i;

uniform float epsilon;

out vec4 FragColor;

void main() {
    // render point as a sphere and shift it back by epsilon
    vec2 from_center_pt_spc = gl_PointCoord - vec2(0.5, 0.5);
    float mag_sq = dot(from_center_pt_spc, from_center_pt_spc);
    if (mag_sq > 0.25 /* 0.5^2 */) {
        discard;
    }

    vec2 from_center_wld_spc = from_center_pt_spc * 2 * i.radius;
    float depth_offset = sqrt(i.radius * i.radius - dot(from_center_wld_spc, from_center_wld_spc));
    float depth = gl_FragCoord.z - depth_offset + epsilon;
    gl_FragDepth = depth;
    FragColor = depth * i.color;
}