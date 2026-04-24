#pragma once

#include <cstdint>

namespace pathfinder {

enum class GameMode : uint8_t {
    Cube = 0,
    Ship = 1,
    Ball = 2,
    Ufo = 3,
    Wave = 4,
    Robot = 5,
    Spider = 6,
    Swing = 7,
    COUNT
};

enum class Action : uint8_t {
    Release = 0,
    Hold = 1,
    Tap = 2,
    COUNT
};

struct PlayerState {
    float x;
    float y;
    float vx;
    float vy;
    bool grounded;
    bool upsideDown;
    GameMode mode;
};

// Forward-simulates one physics tick (1/240s) of GD given an input action.
// This is an approximation - real GD physics is bit-exact and unavailable
// outside the engine, but this matches well enough at planning resolution.
class PhysicsModel {
public:
    static constexpr float kTickDt = 1.0f / 240.0f;

    static PlayerState step(PlayerState const& s, Action a, float groundY, float ceilingY);

    // Per-mode tunables (gravity, jump impulse, etc.) live here.
    static float gravityFor(GameMode m, bool upsideDown);
    static float jumpImpulseFor(GameMode m, bool upsideDown);
    static float forwardSpeedFor(GameMode m);
};

} // namespace pathfinder
