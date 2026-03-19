#version 460 core

// input (from vertex)
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_col;
// output (to fragment shader)
layout(location = 0) out vec3 out_pos;
// uniform buffers
layout(location =  0) uniform mat4x4 model_transform;
layout(location =  8) uniform mat4x4 camera_transform;
layout(location = 12) uniform mat4x4 camera_projection;

void main() {
    gl_Position = model_transform * vec4(in_pos, 1.0); // calculate world position
    out_pos = vec3(gl_Position); // store world position for light calculations
    gl_Position = camera_transform * gl_Position; // transform according to camera position/rotation
    gl_Position = camera_projection * gl_Position; // camera projection
}