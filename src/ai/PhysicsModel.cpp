#include "PhysicsModel.hpp"

#include <algorithm>

namespace pathfinder {

float PhysicsModel::gravityFor(GameMode m, bool upsideDown) {
    float g = 0.0f;
    switch (m) {
        case GameMode::Cube:   g = 2800.0f; break;
        case GameMode::Ship:   g = 1400.0f; break;
        case GameMode::Ball:   g = 2400.0f; break;
        case GameMode::Ufo:    g = 2200.0f; break;
        case GameMode::Wave:   g = 0.0f;    break; // wave is velocity-locked
        case GameMode::Robot:  g = 2800.0f; break;
        case GameMode::Spider: g = 2800.0f; break;
        case GameMode::Swing:  g = 1800.0f; break;
        default: g = 2800.0f;
    }
    return upsideDown ? -g : g;
}

float PhysicsModel::jumpImpulseFor(GameMode m, bool upsideDown) {
    float j = 0.0f;
    switch (m) {
        case GameMode::Cube:   j = 900.0f;  break;
        case GameMode::Ship:   j = 600.0f;  break; // sustained while held
        case GameMode::Ball:   j = -400.0f; break; // ball flips gravity instead
        case GameMode::Ufo:    j = 750.0f;  break; // tap-only
        case GameMode::Wave:   j = 0.0f;    break;
        case GameMode::Robot:  j = 1100.0f; break;
        case GameMode::Spider: j = 0.0f;    break; // spider teleports
        case GameMode::Swing:  j = 700.0f;  break; // flips on tap
        default: j = 900.0f;
    }
    return upsideDown ? -j : j;
}

float PhysicsModel::forwardSpeedFor(GameMode m) {
    // GD ships levels in 4 speeds; the bot plans at speed-2 baseline and
    // re-normalizes when a portal changes it.
    return 432.0f;
}

PlayerState PhysicsModel::step(PlayerState const& s, Action a, float groundY, float ceilingY) {
    PlayerState n = s;

    float g = gravityFor(s.mode, s.upsideDown);

    switch (s.mode) {
        case GameMode::Cube:
        case GameMode::Robot:
        case GameMode::Swing: {
            n.vy += g * kTickDt;
            if (a != Action::Release && s.grounded) {
                n.vy = jumpImpulseFor(s.mode, s.upsideDown);
                n.grounded = false;
            }
            break;
        }
        case GameMode::Ship:
        case GameMode::Ufo: {
            // Ship lifts while held, falls while released.
            float lift = (a == Action::Release) ? -jumpImpulseFor(s.mode, s.upsideDown)
                                                :  jumpImpulseFor(s.mode, s.upsideDown);
            n.vy += (g + lift * (s.upsideDown ? -1.0f : 1.0f)) * kTickDt;
            n.vy = std::clamp(n.vy, -1200.0f, 1200.0f);
            break;
        }
        case GameMode::Ball: {
            n.vy += g * kTickDt;
            if (a == Action::Tap && s.grounded) {
                n.upsideDown = !s.upsideDown;
                n.grounded = false;
            }
            break;
        }
        case GameMode::Wave: {
            // Wave moves at +/-45 degrees depending on input.
            float dir = (a == Action::Release) ? -1.0f : 1.0f;
            if (s.upsideDown) dir = -dir;
            n.vy = dir * forwardSpeedFor(s.mode);
            break;
        }
        case GameMode::Spider: {
            n.vy += g * kTickDt;
            if (a == Action::Tap) {
                // Spider teleports to the opposite surface; planner handles via
                // an instantaneous y-snap done in the higher-level search.
                n.upsideDown = !s.upsideDown;
            }
            break;
        }
        default: break;
    }

    n.x += forwardSpeedFor(s.mode) * kTickDt;
    n.y += n.vy * kTickDt;

    if (n.y <= groundY) {
        n.y = groundY;
        n.vy = 0.0f;
        n.grounded = true;
    } else if (n.y >= ceilingY) {
        n.y = ceilingY;
        n.vy = 0.0f;
    }

    return n;
}

} // namespace pathfinder
