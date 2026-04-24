#include "LevelGenerator.hpp"
#include "Pathfinder.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <random>

using namespace geode::prelude;

namespace pathfinder {

namespace {

struct Chunk {
    GameMode mode;
    int minDifficulty;
    int maxDifficulty;
    float length;
    std::vector<LevelObject> objects; // local coords, 0 = chunk start
};

// Hand-authored chunk bank. Real version of this mod would load these from
// resources/chunks/*.gmd, but a starter set is inlined so the generator
// works out of the box.
std::vector<Chunk> const& chunkBank() {
    static std::vector<Chunk> bank = {
        // Cube: simple jump over a single spike
        { GameMode::Cube, 1, 4, 240.0f, {
            { 1, 0,   0,   0 }, { 1, 30,  0,   0 }, { 1, 60,  0,   0 },
            { 8, 120, 0,   0 },
            { 1, 180, 0,   0 }, { 1, 210, 0,   0 },
        }},
        // Cube: yellow orb chain
        { GameMode::Cube, 3, 7, 360.0f, {
            { 1,  0,   0,  0 }, { 1, 30,  0,   0 },
            { 36, 90,  60, 0 }, // yellow orb
            { 8,  150, 0,  0 },
            { 36, 210, 90, 0 },
            { 1,  300, 0,  0 }, { 1, 330, 0,   0 },
        }},
        // Ship: wide corridor
        { GameMode::Ship, 2, 6, 480.0f, {
            { 1, 0,    0,   0 }, { 1, 0,   240, 0 },
            { 1, 240,  60,  0 }, { 1, 240, 180, 0 },
            { 1, 480,  0,   0 }, { 1, 480, 240, 0 },
        }},
        // Wave: tight diagonal slot
        { GameMode::Wave, 5, 9, 360.0f, {
            { 1, 60,   0,   0 }, { 1, 60,  90,  0 },
            { 1, 180,  150, 0 }, { 1, 180, 240, 0 },
            { 1, 300,  60,  0 }, { 1, 300, 150, 0 },
        }},
        // Ball: gravity-flip section
        { GameMode::Ball, 3, 7, 300.0f, {
            { 1,  0,   0,   0 }, { 1, 0,   180, 0 },
            { 67, 90,  0,   0 }, // blue gravity pad on floor
            { 1,  150, 180, 0 },
            { 67, 240, 180, 0 }, // blue pad on ceiling
        }},
        // Robot: variable-jump tower
        { GameMode::Robot, 4, 8, 360.0f, {
            { 1,  0,   0,   0 },
            { 1,  120, 60,  0 },
            { 1,  240, 120, 0 },
            { 8,  300, 0,   0 },
        }},
        // UFO: tap-rhythm
        { GameMode::Ufo, 3, 7, 360.0f, {
            { 1, 0,    0,   0 }, { 1, 0,   240, 0 },
            { 8, 120,  60,  0 },
            { 8, 240,  150, 0 },
            { 1, 360,  0,   0 }, { 1, 360, 240, 0 },
        }},
        // Spider: teleport zigzag
        { GameMode::Spider, 5, 9, 360.0f, {
            { 1, 0,   0,   0 }, { 1, 0,   210, 0 },
            { 8, 120, 0,   0 }, { 8, 240, 210, 0 },
        }},
        // Swing: pendulum gaps
        { GameMode::Swing, 4, 8, 360.0f, {
            { 1, 0,   90,  0 }, { 1, 0,   150, 0 },
            { 1, 180, 60,  0 }, { 1, 180, 180, 0 },
            { 8, 300, 90,  0 },
        }},
    };
    return bank;
}

// Portal IDs (10/11 = cube/ship, 12/13 = ball/UFO, 99 = wave, etc.)
int portalIdFor(GameMode m) {
    switch (m) {
        case GameMode::Cube:   return 12;
        case GameMode::Ship:   return 13;
        case GameMode::Ball:   return 47;
        case GameMode::Ufo:    return 111;
        case GameMode::Wave:   return 660;
        case GameMode::Robot:  return 745;
        case GameMode::Spider: return 1331;
        case GameMode::Swing:  return 1933;
    }
    return 12;
}

bool chunkAllowed(Chunk const& c, int diff,
                  std::vector<GameMode> const& allowed) {
    if (diff < c.minDifficulty || diff > c.maxDifficulty) return false;
    if (allowed.empty()) return true;
    return std::find(allowed.begin(), allowed.end(), c.mode) != allowed.end();
}

} // namespace

GeneratedLevel LevelGenerator::generate(int difficulty,
                                        std::vector<GameMode> const& allowedModes,
                                        uint32_t seed) {
    std::mt19937 rng(seed);
    GeneratedLevel level;
    level.colorBg     = 0x1a1a2e;
    level.colorGround = 0x16213e;
    level.title       = fmt::format("Pathfinder #{}", seed % 100000);

    auto const& bank = chunkBank();
    float cursorX = 0.0f;
    GameMode currentMode = GameMode::Cube;
    int targetChunks = 12 + difficulty * 3;

    // Floor blocks for the whole runway.
    for (int i = 0; i < targetChunks * 20; ++i) {
        level.objects.push_back({ 1, i * 30.0f, -30.0f, 0 });
    }

    int placed = 0;
    int safety = 0;
    while (placed < targetChunks && safety++ < 500) {
        std::vector<Chunk const*> candidates;
        for (auto const& c : bank) {
            if (chunkAllowed(c, difficulty, allowedModes)) candidates.push_back(&c);
        }
        if (candidates.empty()) break;

        auto const* chunk = candidates[rng() % candidates.size()];

        // Mode change? Drop a portal.
        if (chunk->mode != currentMode) {
            level.objects.push_back({ portalIdFor(chunk->mode), cursorX, 90.0f, 0 });
            cursorX += 60.0f;
            currentMode = chunk->mode;
        }

        for (auto const& obj : chunk->objects) {
            level.objects.push_back({ obj.id, cursorX + obj.x, obj.y, obj.rot });
        }
        cursorX += chunk->length + 30.0f;
        ++placed;
    }

    log::info("Generated level '{}' with {} objects ({} chunks)",
              level.title, level.objects.size(), placed);
    return level;
}

} // namespace pathfinder
