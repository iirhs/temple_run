#pragma once

struct Audio {
    void init(const std::string& path) {
        SDL_LoadWAV(path.c_str(), &spec, &buffer, &buffer_size);
    }
    void destroy() {
        SDL_free(buffer);
    }
    SDL_AudioSpec spec;
    Uint8* buffer;
    Uint32 buffer_size;
};