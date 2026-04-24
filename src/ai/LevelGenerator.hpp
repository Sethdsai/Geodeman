#pragma once

#include "PhysicsModel.hpp"

#include <string>
#include <vector>

namespace pathfinder {

struct LevelObject {
    int id;       // GD object ID (e.g. 1 = block, 8 = spike, 35 = yellow pad)
    float x;
    float y;
    int  rot;
};

struct GeneratedLevel {
    std::vector<LevelObject> objects;
    int colorBg;
    int colorGround;
    std::string title;
};

// Procedurally synthesizes a level. The generator works in "chunks" - each
// chunk is a verified-playable pattern (jump-orb sequence, ship corridor,
// wave gap, etc.) sampled from a curated bank, then stitched with portal
// transitions. Pathfinder forward-sims every chunk before it ships, so
// every generated level is guaranteed completable with a valid input
// trace - which is what makes the bot reliably beat them.
class LevelGenerator {
public:
    static GeneratedLevel generate(int difficulty,
                                   std::vector<GameMode> const& allowedModes,
                                   uint32_t seed);
};

} // namespace pathfinder
