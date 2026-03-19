#version 460 core

// input (from vertex shader)
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_col;
// output (to pixel of output image)
layout(location = 0) out vec4 out_color;
// texture unit
layout(binding = 0) uniform sampler2D tex_diffuse;
layout(binding = 1) uniform samplerCube tex_shadow;
// uniforms
// start at location 16 as vert/frag shaders in a pipeline share these locations
layout(location = 16) uniform vec3 camera_pos;
layout(location = 17) uniform float texture_contribution = 1.0;
layout(location = 18) uniform float specular = 1.0;
layout(location = 19) uniform float specular_shininess = 32.0;
// light properties
layout(location = 30) uniform vec3 light_pos = vec3(3, 3, 0);
layout(location = 31) uniform vec3 light_col = vec3(0.992, 0.984, 0.827);
layout(location = 32) uniform float light_range = 100.0;

// calculate ambient light strength
float calc_ambient() {
    return 0.35;
}

// calculate the diffuse light strength
float calc_diffuse(vec3 normal, vec3 light_dir) {
    float diffuse = dot(normal, light_dir); // dot product to figure out angle
    return max(diffuse, 0.0); // negative values mean that light does not hit the surface
}

// calculate specular light strength
float calc_specular(vec3 normal, vec3 light_dir) {
    vec3 reflected_dir = reflect(light_dir, normal); // reflect camera direction off the surface via normal
    vec3 camera_dir = normalize(in_pos - camera_pos); // unit vector from camera to fragment/pixel
    float specular_strength = dot(camera_dir, reflected_dir); // check if camera is near reflected light
    specular_strength = max(specular_strength, 0.0); // filter out "negative" strength
    specular_strength = pow(specular_strength, specular_shininess); // scale by "shininess"
    return specular_strength * specular; // scale by "specular"
}

float calc_shadow(vec3 normal, vec3 light_dir) {
    // vector from light to pixel
    vec3 light_to_pixel = in_pos - light_pos;
    // calculate distance from light source to the current pixel
    float light_distance = length(light_to_pixel);
    // read from the shadowmap to know min depth that the light can "see"
    float light_distance_in_shadowmap = texture(tex_shadow, light_to_pixel).r; // depth tex only has 1 channel
    // since it was scaled down to range between 0 and 1, scale back up to true length
    light_distance_in_shadowmap = light_distance_in_shadowmap * light_range;

    // due to floating point inaccuracies, we introduce a small bias based on light angle
    float bias_max = 1.0; // on sharp angles
    float bias_min = 0.005; // on perpendicular angles
    float bias = max((1.0 - dot(normal, light_dir) * bias_max), bias_min); 

    // if light_distance_in_shadowmap is smaller, current pixel is behind something from the light's POV
    if (light_distance_in_shadowmap + bias < light_distance) return 0.0;
    else return 1.0;
}

// smooth out shadows with percentage-closer-filter (good ol blur)
float calc_shadow_smooth(vec3 normal, vec3 light_dir) {
    // vector from light to pixel
    vec3 light_to_pixel = in_pos - light_pos;
    // calculate distance from light source to the current pixel
    float light_distance = length(light_to_pixel);

    // due to floating point inaccuracies, we introduce a small bias based on light angle
    float bias_max = 1.0; // on sharp angles
    float bias_min = 0.005; // on perpendicular angles
    float bias = max((1.0 - dot(normal, light_dir) * bias_max), bias_min); 

    // percentage-closer-filter
    float shadow = 0.0;
    float max_samples = 4;
    float offset_max = 0.005;
    float offset_step = offset_max / (max_samples * 0.5);
    for(float x = -offset_max; x < offset_max; x += offset_step) {
        for(float y = -offset_max; y < offset_max; y += offset_step) {
            for(float z = -offset_max; z < offset_max; z += offset_step) {
                // sample shadow texture with a slight offset
                vec3 sample_vec = light_to_pixel + vec3(x, y, z);
                float light_distance_in_shadowmap = texture(tex_shadow, sample_vec).r; // depth tex only has 1 channel
                // since it was scaled down to range between 0 and 1, scale back up to true length
                light_distance_in_shadowmap *= light_range;
                if (light_distance_in_shadowmap + bias < light_distance) shadow += 1.0;
            }
        }
    }
    // normalize shadow from 0 to 1 (blur all samples)
    shadow /= max_samples * max_samples * max_samples;
    // invert shadow to get light contribution
    return 1.0 - shadow;
}

void main() {
    vec3 normal = normalize(in_norm); // make sure its normalized after interpolation

    // simulate a light at a static position
    vec3 light_dir = normalize(light_pos - in_pos); // vector from light to current pixel world position

    // calculate shadow influence
    float shadow = calc_shadow(normal, light_dir);

    // calculate ambient light (light influence that is present everywhere)
    float ambient_str = calc_ambient();
    vec3 ambient_light = light_col * ambient_str;

    // calculate diffuse light (based on angle between normal and light)
    float diffuse_str = calc_diffuse(normal, light_dir);
    vec3 diffuse_light = light_col * diffuse_str * shadow;

    // calculate specular light (reflecting spots on sharp angles)
    float specular_str = calc_specular(normal, light_dir);
    vec3 specular_light = light_col * specular_str * shadow;

    // interpolate between texture and vertex color
    vec4 texture_color = texture(tex_diffuse, in_uv);
    vec4 vertex_color = in_col;
    vec4 diffuse_col = mix(vertex_color, texture_color, texture_contribution); // linear interpolation (0.0 to 1.0)

    // combine interpolated color with light colors
    out_color.rgb = diffuse_col.rgb * (ambient_light + diffuse_light + specular_light);
    out_color.a = diffuse_col.a; // since alpha is left out of light calculations
}
