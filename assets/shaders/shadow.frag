#version 460 core

// input (from vertex shader)
layout(location = 0) in vec3 in_pos;

// light properties
layout(location = 30) uniform vec3 light_pos = vec3(3, 3, 0);
layout(location = 32) uniform float light_range = 100.0;

void main() {
    float light_distance = length(in_pos - light_pos); // distance from current pixel world position to light source
    // output texture can only store values from 0 to 1, so we have to scale light_distance into that range
    light_distance = light_distance / light_range; // light_range as the maximum
    // this distance will be the depth we write to the texture
    gl_FragDepth = light_distance;
}
