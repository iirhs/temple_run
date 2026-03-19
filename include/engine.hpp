#pragma once
#include "time.hpp"
#include "input.hpp"
#include "model.hpp"
#include "light.hpp"
#include "audio.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "pipeline.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>

struct Engine {
    Engine() {
        _time.init();
        std::srand((unsigned)std::time(nullptr));

        // create render components
        _window.init(1280, 720, 4);
        _pipeline.init("default.vert", "default.frag");
        _shadow_pipeline.init("shadow.vert", "shadow.frag");
        _shadow_pipeline.create_shadow_framebuffer();

        // cube with texture
        _cube_textured.init(Primitive::eCube, "grass.png");
        _cube_textured._transform._position.x = -2;
        // cube with vertex colors
        _cube_vertcols.init(Primitive::eCube);
        _cube_vertcols._transform._position.x = +2;
        // sphere
        _sphere.init(Primitive::eSphere);

        // sponza scene
        _sponza.init("sponza/sponza.obj");
        _sponza._transform._scale = glm::vec3{ 0.01f, 0.01f, 0.01f };

        // move the camera to the back a little
        _camera._position = { 3, 3, 0 };

        // create light for lighting and shadows
        _light.init();

        // create an audio stream for default audio device
        SDL_InitSubSystem(SDL_INIT_AUDIO);
        // load audio file
        _doom.init("assets/audio/doom.wav");
        // create an audio stream for current playback device
        audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr, nullptr, nullptr);
        if (audio_stream == nullptr) std::println("{}", SDL_GetError());
        // get the format of the device (sample rate and such)
        SDL_AudioSpec device_format;
        SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &device_format, nullptr);
        // set up the audio stream to convert from our .wav file sample rate to the device's sample rate
        if (!SDL_SetAudioStreamFormat(audio_stream, &_doom.spec, &device_format)) std::println("{}", SDL_GetError());
        // load .wav into the audio stream and play
        if (!SDL_PutAudioStreamData(audio_stream, _doom.buffer, _doom.buffer_size)) std::println("{}", SDL_GetError());
        if (!SDL_ResumeAudioStreamDevice(audio_stream)) std::println("{}", SDL_GetError());

        // ---RUNNER GAME INIT---
        // Runner: cubes for a clean aesthetic
        _runner_player.init(Primitive::eCube, "grass.png");
        _runner_ground_tile.init(Primitive::eCube, "grass.png");
        // obstacles: keep vertex-colored cube, but make it less shiny so it reads better
        _runner_obstacle_cube.init(Primitive::eCube);
        if (!_runner_obstacle_cube._materials.empty()) {
            _runner_obstacle_cube._materials[0]._specular = 0.15f;
            _runner_obstacle_cube._materials[0]._specular_shininess = 6.0f;
        }
        
        // player transform
        _runner_player._transform._position = glm::vec3(0.0f, _player_y, 0.0f);
        _runner_player._transform._scale = glm::vec3(0.9f);

        // --- Initialize runner camera immediately ---
        _cam_pos = glm::vec3(0.0f, _player_ground_y + _cam_height, _cam_back);
        _camera._position = _cam_pos;
        _camera._rotation = glm::vec3(_cam_pitch, 0.0f, 0.0f);

        // ground tiles - strip
        _ground_tiles.clear();
        const int tile_count = 24;
        for (int i = 0; i < tile_count; i++){
            Transform t;
            t._position = glm::vec3(0.0f, 0.0f, -float(i) * 2.0f);
            t._scale = glm::vec3(8.0f, 0.2f, 3.0f); //wide floor
            _ground_tiles.push_back(t);
        }
        // ----- Obstacles init -----
        _obstacles.clear();
        randomize_obstacle_rows();

    }

    ~Engine() {
        // destroy in reversed init() order
        SDL_DestroyAudioStream(audio_stream);
        _sphere.destroy();
        _cube_textured.destroy();
        _cube_vertcols.destroy();
        _pipeline.destroy();
        _shadow_pipeline.destroy();
        _window.destroy();
        _runner_player.destroy();
        _runner_ground_tile.destroy();
        _runner_obstacle_cube.destroy();

    }

    void handle_inputs() {
        // move via WASDQE
        float speed = 2.0 * _time._delta; // 2.0 units per second
        if (Keys::down(SDLK_W)) _camera.translate(0, 0, -speed);
        if (Keys::down(SDLK_A)) _camera.translate(-speed, 0, 0);
        if (Keys::down(SDLK_S)) _camera.translate(0, 0, +speed);
        if (Keys::down(SDLK_D)) _camera.translate(+speed, 0, 0);
        if (Keys::down(SDLK_Q)) _camera.translate(0, -speed, 0);
        if (Keys::down(SDLK_E)) _camera.translate(0, +speed, 0);

        // let go of mouse capture when we press ESCAPE
        if (Mouse::captured() && Keys::pressed(SDLK_ESCAPE)) {
            Input::register_capture(false);
            SDL_SetWindowRelativeMouseMode(_window._window_p, Mouse::captured());
        }
        // grab mouse capture when we click into the window
        if (!Mouse::captured() && Mouse::pressed(Mouse::ids::left)) {
            Input::register_capture(true);
            SDL_SetWindowRelativeMouseMode(_window._window_p, Mouse::captured());
        }
        // camera rotation
        if (Mouse::captured()) {
            float mouse_sensitivity = 0.003f;
            _camera._rotation.x -= mouse_sensitivity * Mouse::delta().second;
            _camera._rotation.y -= mouse_sensitivity * Mouse::delta().first;
        }

        // draw wireframe while holding F
        if (Keys::down(SDLK_F)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    auto handle_sdl_event(SDL_Event& event) -> SDL_AppResult {
        switch (event.type) {
            case SDL_EventType::SDL_EVENT_QUIT: return SDL_AppResult::SDL_APP_SUCCESS;
            default: break;
        }
        Input::register_event(event);
        return SDL_AppResult::SDL_APP_CONTINUE;
    }
    auto handle_sdl_frame() -> SDL_AppResult {
        _time.update();
        if (_runner_mode) runner_update(_time._delta);
        else handle_inputs();


        // draw shadows
        if (_light._shadow_dirty) {
            _shadow_pipeline.bind();
            // render into each cubemap face separately
            for (uint32_t face_i = 0; face_i < 6; face_i++) {
                _light.bind_shadow_write(_shadow_pipeline, face_i);
                // draw the stuff
                _sponza.draw(true);
                _cube_textured.draw(true);
                _cube_vertcols.draw(true);
                _sphere.draw(true);
            }
            _light._shadow_dirty = false;
        }

        // draw color
            // bind graphics pipeline containing vertex and fragment shaders
            _pipeline.bind();
            glViewport(0, 0, 1280, 720); // TODO: dynamically set to size of window
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            // choose color to clear screen with
            glClearColor(0.05f, 0.07f, 0.12f, 1.0f); // dark blue night
            // clear image before drawing to it
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _light.bind_shadow_read();

            // bind camera to the pipeline
            _camera.bind();
            if (_runner_game_over) {
                SDL_SetWindowTitle(_window._window_p, "GAME OVER - Press R to Restart");
            }
            if (_runner_mode) {
                for (auto &t : _ground_tiles) {
                    _runner_ground_tile._transform = t;
                    _runner_ground_tile.draw();
                }
                _runner_player.draw();
                for (auto &o : _obstacles) {
                    _runner_obstacle_cube._transform._position = glm::vec3(lane_x(o.lane), 0.6f, o.z);
                    _runner_obstacle_cube._transform._scale = o.scale;
                    _runner_obstacle_cube.draw();
                }
            } else {
                _sponza.draw();
                _cube_textured.draw();
                _cube_vertcols.draw();
                _sphere.draw();
            }

        // present drawn image to screen
        SDL_GL_SwapWindow(_window._window_p);
        // clear single-frame inputs
        Input::flush();
        return SDL_AppResult::SDL_APP_CONTINUE;
    }

    // ---RUNNER UPDATE---
    int rand_int(int a, int b) { // inclusive
    return a + (std::rand() % (b - a + 1));
    }

    float randf() const {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    // Build obstacle rows: each row blocks 1–2 lanes, always leaving at least 1 open.
    void randomize_obstacle_rows() {
        std::vector<RunnerObstacle> newObs;
        newObs.reserve(24);

        float z0 = -30.0f;
        const int rows = 12;

        for (int r = 0; r < rows; r++) {
            float rowZ = z0 - r * _spawn_spacing;

            int count = rand_int(_obstacles_per_row_min, _obstacles_per_row_max);

            int laneA = rand_int(0, 2);
            int laneB = rand_int(0, 2);
            while (laneB == laneA) laneB = rand_int(0, 2);

            RunnerObstacle o1;
            o1.lane = laneA;
            o1.z = rowZ;
            o1.scale = glm::vec3(0.9f, 1.2f, 0.9f);
            newObs.push_back(o1);

            if (count == 2) {
                RunnerObstacle o2;
                o2.lane = laneB;
                o2.z = rowZ;
                o2.scale = glm::vec3(0.9f, 1.2f, 0.9f);
                newObs.push_back(o2);
            }
        }

        _obstacles = std::move(newObs);
    }

    float lane_x(int lane) const{
        return (float(lane) - 1.0f) * _lane_width;
    }

    float smooth_to(float current, float target, float dt, float snap_seed){
        //exponential smoothing: stable and frame-rate independent
        float k = 1.0f - std::exp(-snap_seed * dt);
        return current + k * (target - current) ;
    }

    void runner_reset() {
    _runner_game_over = false;
    _distance = 0.0f;
    _run_speed = 12.0f;

    _player_lane = 1;
    _player_x = 0.0f;
    _player_target_x = 0.0f;

    _player_y = _player_ground_y;
    _player_vy = 0.0f;
    _player_on_ground = true;


    // reset obstacles to start far away again
    float startZ = -30.0f;
    for (auto &ob : _obstacles) {
        ob.z = startZ;
        startZ -= _spawn_spacing;
        ob.lane = rand_int(0, 2);
        ob.scale = glm::vec3(0.9f, 1.2f, 0.9f);
        ob.active = true;
    }

    // reset ground strip if you want (optional)
    float z = 0.0f;
    for (auto &t : _ground_tiles) {
        t._position.z = z;
        z -= 2.0f;
        }
    }
    void runner_handle_input(float dt){
        if (Keys::pressed(SDLK_A) || Keys::pressed(SDLK_LEFT)){
            _player_lane = std::max(0, _player_lane - 1);
        }
        if (Keys::pressed(SDLK_D) || Keys::pressed(SDLK_RIGHT)){
            _player_lane = std::min(2, _player_lane + 1);
        }
        if (Keys::pressed(SDLK_R)) {
            runner_reset();
        }
        _player_target_x = lane_x(_player_lane);

        if ((Keys::pressed(SDLK_SPACE) || Keys::pressed(SDLK_UP)) && _player_on_ground && !_runner_game_over) {
            _player_vy = _jump_velocity;
            _player_on_ground = false;
        }
    }

    bool lane_hit(float playerZ, float obstacleZ, float hitDist = 0.9f) {
        return std::abs(obstacleZ - playerZ) < hitDist;
    }

    void runner_update(float dt){
        runner_handle_input(dt);
        if (_runner_game_over) return;

        // smooth lane movement
        _player_x = smooth_to(_player_x, _player_target_x, dt, 18.0f);
        
        // forward distance/score
        _distance += _run_speed * dt;

        static float titleTimer = 0.0f;
        titleTimer += dt;

        // speed ramps up with time
        _run_speed += 0.6f * dt;    // slow increase
        _run_speed = std::min(_run_speed, 26.0f);

        if (titleTimer > 0.2f) { // update 5x/sec
            titleTimer = 0.0f;
            char buf[128];
            std::snprintf(buf, sizeof(buf), "Temple Run Mini | Distance: %.0f", _distance);
            SDL_SetWindowTitle(_window._window_p, buf);
        }


        //move ground tiles toward camer/player (endless strip)
        for (auto &t : _ground_tiles){
            t._position.z += _run_speed * dt;
            if(t._position.z > 2.0f) {
                // recycle to the back - most negative z)
                float minZ = t._position.z;
                for (auto &o : _ground_tiles) minZ = std::min(minZ, o._position.z);
                t._position.z = minZ - 2.0f; 
            }
        }
        // ----- Move obstacles toward player -----
        for (auto &o : _obstacles) {
            o.z += _run_speed * dt;

            // recycle when past player
            if (o.z > (_player_z + 4.0f)) {
                float minZ = o.z;
                for (auto &k : _obstacles) minZ = std::min(minZ, k.z);
                o.z = minZ - _spawn_spacing;

                // re-roll lane and height when recycled
                o.lane = rand_int(0, 2);
                o.scale.y = (rand_int(0, 3) == 0) ? 1.8f : 1.2f;
            }
        }
        // ----- Collision (same lane + z threshold) -----
        const float playerZ = 0.0f;

        for (const auto &ob : _obstacles) {
            if (!ob.active) continue;
            if (ob.lane != _player_lane) continue;

            if (lane_hit(playerZ, ob.z, 0.7f)) {
                if (_player_y < (_player_ground_y + 0.6f)) { // not high enough => hit
                    _runner_game_over = true;
                    break;
                }
            }
        }
        // vertical motion
        _player_vy += _gravity * dt;
        _player_y  += _player_vy * dt;

        // ground clamp
        if (_player_y <= _player_ground_y) {
            _player_y = _player_ground_y;
            _player_vy = 0.0f;
            _player_on_ground = true;
        }

        // update player transform (after physics)
        _runner_player._transform._position = glm::vec3(_player_x, _player_y, 0.0f);

        // --- camera: behind runner (use tunables below) ---
        glm::vec3 desired(
            _player_x,
            _player_ground_y + _cam_height,
            _cam_back
        );

        _cam_pos = glm::mix(_cam_pos, desired, 1.0f - std::exp(-_cam_follow * dt));
        _camera._position = _cam_pos;
        _camera._rotation = glm::vec3(_cam_pitch, 0.0f, 0.0f);
    }

    Time _time;
    Model _cube_vertcols;
    Model _cube_textured;
    Model _sphere;
    Model _sponza;
    Window _window;
    Camera _camera;
    Pipeline _pipeline;
    Pipeline _shadow_pipeline;
    Light _light;
    // audio
    SDL_AudioStream* audio_stream;
    Audio _doom;


    // ---RUNNER GAME STATE---
    struct RunnerObstacle {
    int lane = 1;                 // 0,1,2
    float z = -30.0f;             // along track
    glm::vec3 scale = {0.9f, 1.0f, 0.9f};  // obstacle cube size
    bool active = true;
    };

    // spawn tuning
    float _spawn_spacing = 8.0f;      // distance between obstacle rows
    int _obstacles_per_row_min = 1;
    int _obstacles_per_row_max = 2;

    float _player_z = 0.0f;           // keep player around z=0

    bool _runner_mode = true;
    bool _runner_game_over = false;

    float _lane_width = 2.0f;
    int _player_lane = 1;  // 0.2
    float _player_x = 0.0f; // current x (smooth)
    float _player_target_x = 0.0f;

    float _player_y = 0.5f;  // standing height (half cube above ground)
    float _player_half = 0.45f;

    float _run_speed = 10.0f; // units per second
    float _distance = 0.0f;

    float _player_ground_y = 0.5f;   // standing height

    float _player_vy = 0.0f;         // vertical velocity
    bool  _player_on_ground = true;

    float _jump_velocity = 7.8f;   // up from ~6.5
    float _gravity       = -24.0f; // stronger pull-down

    glm::vec3 _cam_pos = glm::vec3(0.0f);
    // camera tuning (runner POV)
    float _cam_height = 1.6f;   // lower => less sky
    float _cam_back   = 3.4f;   // closer => bigger obstacles (keep + if your camera looks toward -Z)
    float _cam_pitch  = -0.55f;  // radians (~40deg) looking down at the track
    float _cam_follow = 14.0f;  // higher => snappier follow


    Model _runner_player;
    Model _runner_ground_tile;
    Model _runner_obstacle_cube;
    std::vector<Transform> _ground_tiles;
    std::vector<RunnerObstacle> _obstacles;

};
