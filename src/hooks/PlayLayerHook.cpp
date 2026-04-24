#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "../ai/Pathfinder.hpp"
#include "../ai/PhysicsModel.hpp"

using namespace geode::prelude;

namespace {

pathfinder::GameMode modeFromPlayer(PlayerObject* p) {
    if (!p) return pathfinder::GameMode::Cube;
    if (p->m_isShip)    return pathfinder::GameMode::Ship;
    if (p->m_isBall)    return pathfinder::GameMode::Ball;
    if (p->m_isBird)    return pathfinder::GameMode::Ufo;
    if (p->m_isDart)    return pathfinder::GameMode::Wave;
    if (p->m_isRobot)   return pathfinder::GameMode::Robot;
    if (p->m_isSpider)  return pathfinder::GameMode::Spider;
    if (p->m_isSwing)   return pathfinder::GameMode::Swing;
    return pathfinder::GameMode::Cube;
}

// Trail buffer for credit assignment on death.
thread_local std::vector<pathfinder::SituationKey> s_keyTrail;
thread_local std::vector<pathfinder::Action>       s_actTrail;

} // namespace

class $modify(MyPlayLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!Mod::get()->getSettingValue<bool>("bot-enabled")) return;
        if (!m_player1) return;

        pathfinder::PlayerState s{};
        s.x = m_player1->getPositionX();
        s.y = m_player1->getPositionY();
        s.vx = pathfinder::PhysicsModel::forwardSpeedFor(modeFromPlayer(m_player1));
        s.vy = m_player1->m_yVelocity;
        s.grounded = m_player1->m_isOnGround;
        s.upsideDown = m_player1->m_isUpsideDown;
        s.mode = modeFromPlayer(m_player1);

        // Sample obstacle heights ~12 ticks ahead. In a complete impl this
        // walks PlayLayer's section/object index; here we leave a stub that
        // the bot tolerates (empty -> open air).
        std::vector<float> heightsAhead(12, 0.0f);

        auto action = pathfinder::Pathfinder::get().chooseAction(s, heightsAhead);
        auto key    = pathfinder::Pathfinder::get().lastPlannedPath().empty()
                         ? 0u
                         : 1u; // makeKey is private; trail uses a coarse sentinel
        s_keyTrail.push_back(key);
        s_actTrail.push_back(action);
        if (s_keyTrail.size() > 240) {
            s_keyTrail.erase(s_keyTrail.begin());
            s_actTrail.erase(s_actTrail.begin());
        }

        switch (action) {
            case pathfinder::Action::Hold:
                this->handleButton(true, 1, true);
                break;
            case pathfinder::Action::Tap:
                this->handleButton(true, 1, true);
                this->handleButton(false, 1, true);
                break;
            case pathfinder::Action::Release:
            default:
                this->handleButton(false, 1, true);
                break;
        }
    }

    void destroyPlayer(PlayerObject* p, GameObject* o) {
        if (Mod::get()->getSettingValue<bool>("learning-enabled")
            && Mod::get()->getSettingValue<bool>("bot-enabled")) {
            pathfinder::Pathfinder::get().recordDeath(s_keyTrail, s_actTrail);
        }
        s_keyTrail.clear();
        s_actTrail.clear();
        PlayLayer::destroyPlayer(p, o);
    }
};
