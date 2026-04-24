#include "Pathfinder.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <queue>

using namespace geode::prelude;

namespace pathfinder {

Pathfinder& Pathfinder::get() {
    static Pathfinder s;
    return s;
}

void Pathfinder::loadPretrainedPolicy() {
    auto path = Mod::get()->getResourcesDir() / "policy.bin";
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        log::warn("policy.bin not found, falling back to uniform policy");
        return;
    }

    uint32_t count = 0;
    f.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (uint32_t i = 0; i < count && f.good(); ++i) {
        SituationKey k;
        PolicyEntry e;
        f.read(reinterpret_cast<char*>(&k), sizeof(k));
        f.read(reinterpret_cast<char*>(e.cost.data()),
               sizeof(float) * e.cost.size());
        m_pretrained.emplace(k, e);
    }
    log::info("Loaded {} pretrained policy entries", m_pretrained.size());
}

PolicyEntry const& Pathfinder::policyFor(SituationKey k) const {
    static PolicyEntry uniform{ { 1.0f, 1.0f, 1.0f } };

    if (auto it = m_sessionOverlay.find(k); it != m_sessionOverlay.end()) {
        return it->second;
    }
    if (auto it = m_pretrained.find(k); it != m_pretrained.end()) {
        return it->second;
    }
    return uniform;
}

SituationKey Pathfinder::makeKey(PlayerState const& s,
                                 std::vector<float> const& heightsAhead) const {
    // 4 bits gamemode, 2 bits velocity bucket, 2 bits height bucket,
    // 24 bits compressed obstacle silhouette.
    uint32_t key = static_cast<uint32_t>(s.mode) & 0xF;
    int vBucket = std::clamp(static_cast<int>((s.vy + 1200.0f) / 600.0f), 0, 3);
    int hBucket = std::clamp(static_cast<int>(s.y / 120.0f), 0, 3);
    key |= (vBucket & 0x3) << 4;
    key |= (hBucket & 0x3) << 6;

    uint32_t silhouette = 0;
    int n = std::min<int>(heightsAhead.size(), 12);
    for (int i = 0; i < n; ++i) {
        int b = std::clamp(static_cast<int>(heightsAhead[i] / 30.0f), 0, 3);
        silhouette |= (b & 0x3) << (i * 2);
    }
    key |= silhouette << 8;
    return key;
}

void Pathfinder::recordDeath(std::vector<SituationKey> const& trail,
                             std::vector<Action> const& actions) {
    // Only the last ~40 ticks are credit-assigned. Linear decay from 1.0
    // at the death frame to 0.1 at the start of the trail.
    int n = std::min<int>(trail.size(), 40);
    for (int i = 0; i < n; ++i) {
        float weight = 0.1f + 0.9f * (static_cast<float>(i) / n);
        auto& entry = m_sessionOverlay[trail[trail.size() - 1 - i]];
        // Lazy-init from pretrained if present.
        if (entry.cost[0] == 0.0f && entry.cost[1] == 0.0f && entry.cost[2] == 0.0f) {
            entry = policyFor(trail[trail.size() - 1 - i]);
        }
        entry.cost[static_cast<size_t>(actions[actions.size() - 1 - i])] += weight;
    }
}

Action Pathfinder::chooseAction(PlayerState const& s,
                                std::vector<float> const& heightsAhead) {
    constexpr int kHorizon = 30;
    constexpr float kCollisionPenalty = 1e6f;

    struct Node {
        PlayerState state;
        float cost;
        Action firstAction;
        int depth;
        std::vector<PlayerState> path;
    };

    auto cmp = [](Node const& a, Node const& b) { return a.cost > b.cost; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);

    for (int a = 0; a < static_cast<int>(Action::COUNT); ++a) {
        pq.push({ s, 0.0f, static_cast<Action>(a), 0, { s } });
    }

    Action best = Action::Release;
    float bestCost = std::numeric_limits<float>::infinity();
    std::vector<PlayerState> bestPath;

    int expanded = 0;
    while (!pq.empty() && expanded < 200) {
        auto node = pq.top();
        pq.pop();
        ++expanded;

        if (node.depth >= kHorizon) {
            if (node.cost < bestCost) {
                bestCost = node.cost;
                best = node.firstAction;
                bestPath = node.path;
            }
            continue;
        }

        for (int a = 0; a < static_cast<int>(Action::COUNT); ++a) {
            Action act = static_cast<Action>(a);
            auto nextState = PhysicsModel::step(node.state, act, 0.0f, 480.0f);

            // Rough collision check against the obstacle silhouette ahead.
            int slot = std::min<int>(node.depth, static_cast<int>(heightsAhead.size()) - 1);
            float ground = (slot >= 0) ? heightsAhead[slot] : 0.0f;
            float addCost = (nextState.y < ground) ? kCollisionPenalty : 0.0f;

            auto key = makeKey(nextState, heightsAhead);
            addCost += policyFor(key).cost[a];

            auto path = node.path;
            path.push_back(nextState);
            pq.push({ nextState,
                      node.cost + addCost,
                      node.firstAction,
                      node.depth + 1,
                      std::move(path) });
        }
    }

    m_lastPath = std::move(bestPath);
    return best;
}

} // namespace pathfinder
