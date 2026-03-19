#pragma once
#include "pipeline.hpp"

struct Light {
    void init() {
        // create shadow texture as cube map
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_shadow_texture);
        glTextureStorage2D(_shadow_texture, 1, GL_DEPTH_COMPONENT32F, _shadow_width, _shadow_height); // 32F (32-bit float) per pixel
        // create shadow camera matrices
        _shadow_projection = glm::perspectiveFov<float>(glm::radians(90.0f), _shadow_width, _shadow_height, 1.0f, _range);
        _shadow_views[0] = glm::lookAt(_position, _position + glm::vec3(+1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // right
        _shadow_views[1] = glm::lookAt(_position, _position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // left
        _shadow_views[2] = glm::lookAt(_position, _position + glm::vec3( 0.0f, +1.0f,  0.0f), glm::vec3(0.0f,  0.0f, +1.0f)); // top
        _shadow_views[3] = glm::lookAt(_position, _position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); // bottom
        _shadow_views[4] = glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f, +1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // back
        _shadow_views[5] = glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); // front
    }
    void destroy() {

    }
    // bind the light for writing to the shadow texture
    void bind_shadow_write(Pipeline& shadow_pipeline, uint32_t face_i) {
        // viewport needs to be adjusted to match the shadow texture size
        glViewport(0, 0, _shadow_width, _shadow_height);

        // bind the target shadow map
        glNamedFramebufferTextureLayer(shadow_pipeline._framebuffer, GL_DEPTH_ATTACHMENT, _shadow_texture, 0, face_i);
        // clear it before doing any writing
        glClear(GL_DEPTH_BUFFER_BIT);
        // bind the light view+projection matrices (act like it is the camera)
        glUniformMatrix4fv( 8, 1, false, glm::value_ptr(_shadow_views[face_i]));
        glUniformMatrix4fv(12, 1, false, glm::value_ptr(_shadow_projection));
        // for shadow rendering, we only care about light position and range
        glUniform3fv(30, 1, glm::value_ptr(_position));
        glUniform1f(32, _range);
    }
    // bind the light for reading the shadow texture
    void bind_shadow_read() {
        // bind shadow textures for reading (slot 1)
        glBindTextureUnit(1, _shadow_texture);
        // bind the standard light properties
        glUniform3fv(30, 1, glm::value_ptr(_position));
        glUniform3fv(31, 1, glm::value_ptr(_color));
        glUniform1f(32, _range);
    }

    // light settings
    glm::vec3 _position{ 3, 3, 0 };
    glm::vec3 _color{ 0.992, 0.984, 0.827 };
    float _range = 100.0;
    // shadow settings
    bool _shadow_dirty = true; // only render shadows when light or other objects moved!
    GLuint _shadow_width = 4096;
    GLuint _shadow_height = 4096;
    GLuint _shadow_texture; // cubemap (6 textures)
    glm::mat4x4 _shadow_projection;
    std::array<glm::mat4x4, 6> _shadow_views;
};