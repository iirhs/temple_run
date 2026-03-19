#pragma once

struct Texture {
    void init(const std::string& relative_path, const std::string& base_path = "") {
        // model loading may provide full paths, rather than relative ones
        std::string full_path;
        if (base_path.size() == 0) {
            full_path = std::string(SDL_GetBasePath()) + "assets/textures/" + relative_path;
        }
        else full_path = base_path + relative_path;
        
        // load image from disk
        SDL_Surface* texture_raw_p = IMG_Load(full_path.c_str());
        if (texture_raw_p == nullptr) {
            std::println("IMG_Load(\"{}\"): {}", full_path, SDL_GetError());
            std::exit(1);
        }

        // store width and height
        _width = texture_raw_p->w;
        _height = texture_raw_p->h;

        // create texture to store image in (texture is gpu buffer)
        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
        const GLuint mip_levels = 4;

        switch (texture_raw_p->format) {
            case SDL_PIXELFORMAT_RGB24: {
                glTextureStorage2D(_texture, mip_levels, GL_SRGB8, _width, _height);
                glTextureSubImage2D(_texture, 0, 0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, texture_raw_p->pixels);
                break;
            }
            case SDL_PIXELFORMAT_BGR24: {
                glTextureStorage2D(_texture, mip_levels, GL_SRGB8, _width, _height);
                glTextureSubImage2D(_texture, 0, 0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, texture_raw_p->pixels);
                break;
            }
            case SDL_PIXELFORMAT_ARGB8888: {
                glTextureStorage2D(_texture, mip_levels, GL_SRGB8_ALPHA8, _width, _height);
                glTextureSubImage2D(_texture, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, texture_raw_p->pixels);
                break;
            }
            default: {
                std::println("Unknown texture format: {}", uint32_t(texture_raw_p->format));
                exit(1);
            }
        }

        // free image on cpu side
        SDL_DestroySurface(texture_raw_p);

        // set sampler parameters
        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // interpolation mode when scaling image down
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // interpolation mode when scaling image up

        // generate mipmap textures
        glGenerateTextureMipmap(_texture);

        // set level of anisotropic filtering (max x16)
        GLfloat max_anisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_anisotropy);
        glTextureParameterf(_texture, GL_TEXTURE_MAX_ANISOTROPY, std::min(max_anisotropy, 16.0f));
    }
    void destroy() {
        glDeleteTextures(1, &_texture);
    }
    void bind() {
        glBindTextureUnit(0, _texture);
    }

    GLuint _texture;
    GLuint _width;
    GLuint _height;
};