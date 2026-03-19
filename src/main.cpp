#include "engine.hpp"

auto SDL_AppInit(void** engine_pp, int, char**) -> SDL_AppResult {
    *engine_pp = new Engine();
    return SDL_AppResult::SDL_APP_CONTINUE;
}
auto SDL_AppIterate(void* engine_p) -> SDL_AppResult {
    return static_cast<Engine*>(engine_p)->handle_sdl_frame();
}
auto SDL_AppEvent(void* engine_p, SDL_Event* event_p) -> SDL_AppResult {
    return static_cast<Engine*>(engine_p)->handle_sdl_event(*event_p);
}
void SDL_AppQuit(void* engine_p, SDL_AppResult) {
    delete static_cast<Engine*>(engine_p);
}