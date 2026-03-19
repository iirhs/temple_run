#pragma once

struct Time {
    using Clock = std::chrono::high_resolution_clock;
    void init() {
        timestamp_init = Clock::now();
        timestamp_prev = Clock::now();
        _delta = 0.0;
        _total = 0.0;
    }
    void update() {
        // update time since last frame
        Clock::time_point current = Clock::now();
        std::chrono::microseconds delta = duration_cast<std::chrono::microseconds>(current - timestamp_prev);
        _delta = double(delta.count()) / 1000000.0; // convert to seconds
        // update timestamp to reflect new frame
        timestamp_prev = current;

        // update total time
        std::chrono::microseconds total = duration_cast<std::chrono::microseconds>(current - timestamp_init);
        _total = double(total.count()) / 1000000.0; // convert to seconds
    }

    double _delta; // seconds since last frame (multiply with "speeds" for frame-independant speeds)
    double _total; // seconds since program launch
private:
    Clock::time_point timestamp_init; // timestamp set at program launch
    Clock::time_point timestamp_prev; // timestamp set in previous frame
};