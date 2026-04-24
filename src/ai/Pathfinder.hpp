#pragma once

#include "PhysicsModel.hpp"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace pathfinder {

// A "situation key" is a coarse hash of (gamemode, local obstacle pattern,
// velocity bucket, height bucket). The policy table maps it to expected
// future cost per action; the planner picks the action that minimizes
// future cost + immediate collision risk.
using SituationKey = uint32_t;

struct PolicyEntry {
    std::array<float, static_cast<size_t>(Action::COUNT)> cost;
};

class Pathfinder {
public:
    static Pathfinder& get();

    // Loads the shipped policy.bin from the mod resources dir. Falls back
    // to a uniform-cost policy if the file is missing - the planner still
    // works, it just wastes more attempts learning the level.
    void loadPretrainedPolicy();

    // Online learning hook. Called when the bot dies in PlayLayer; the
    // last N situations leading up to death have their chosen action's
    // cost nudged upward in a session-local overlay.
    void recordDeath(std::vector<SituationKey> const& trail,
                     std::vector<Action> const& actions);

    // Returns the action the bot should take right now given the
    // forward-simulated obstacle window. The planner does a short
    // best-first search (~30 ticks ahead) using PhysicsModel and breaks
    // ties using policy cost.
    Action chooseAction(PlayerState const& s,
                        std::vector<float> const& obstacleHeightsAhead);

    // Exposes the planned path for the overlay renderer.
    std::vector<PlayerState> const& lastPlannedPath() const { return m_lastPath; }

private:
    Pathfinder() = default;

    PolicyEntry const& policyFor(SituationKey k) const;
    SituationKey makeKey(PlayerState const& s,
                         std::vector<float> const& heightsAhead) const;

    std::unordered_map<SituationKey, PolicyEntry> m_pretrained;
    std::unordered_map<SituationKey, PolicyEntry> m_sessionOverlay;
    std::vector<PlayerState> m_lastPath;
};

} // namespace pathfinder
