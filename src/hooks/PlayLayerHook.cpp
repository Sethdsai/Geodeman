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
    CCMenu* m_botMenu = nullptr;
    CCMenuItemToggler* m_botToggle = nullptr;
    bool m_botButtonReady = false;

    void onBotToggle(CCObject* sender) {
        auto mod = Mod::get();
        bool current = mod->getSettingValue<bool>("bot-enabled");
        mod->setSettingValue<bool>("bot-enabled", !current);

        if (m_botToggle) {
            m_botToggle->setToggle(!current);
        }

        Notification::create(!current ? "Bot enabled" : "Bot disabled",
                             NotificationIcon::Info)->show();
    }

    void addBotToggleButton() {
        if (m_botButtonReady) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create toggle button - small checkbox like other GD game buttons
        auto toggleOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto toggleOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

        if (!toggleOnSpr || !toggleOffSpr) {
            log::warn("Bot toggle sprites not found");
            return;
        }

        m_botToggle = CCMenuItemToggler::create(
            toggleOffSpr,
            toggleOnSpr,
            this,
            menu_selector(MyPlayLayer::onBotToggle)
        );

        // Scale to match typical GD button size for touch-friendly interaction
        m_botToggle->setScale(0.55f);
        m_botToggle->setID("bot-toggle-btn"_spr);

        bool botEnabled = Mod::get()->getSettingValue<bool>("bot-enabled");
        m_botToggle->setToggle(botEnabled);

        // Create menu positioned near bottom-right, to the LEFT of practice mode
        // This places the button right next to practice mode button
        m_botMenu = CCMenu::create();
        m_botMenu->addChild(m_botToggle);
        m_botMenu->setPosition({ winSize.width - 85.0f, 28.0f }); // 28 pts from bottom
        m_botMenu->setID("bot-toggle-menu"_spr);

        this->addChild(m_botMenu);
        m_botButtonReady = true;
    }

    void resetBotButton() {
        m_botButtonReady = false;
        if (m_botMenu) {
            m_botMenu->removeFromParentAndCleanup(true);
            m_botMenu = nullptr;
        }
        m_botToggle = nullptr;
        addBotToggleButton();
    }

    void postInit() {
        PlayLayer::postInit();

        // Delay to ensure UI is fully set up
        this->scheduleOnce(schedule_selector(MyPlayLayer::delayedInit), 0.15f);
    }

    void delayedInit(float) {
        addBotToggleButton();
    }

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

    // Reset button when resuming from practice mode
    void resume() {
        PlayLayer::resume();
        resetBotButton();
    }

    // Reset button on level restart
    void onExit() override {
        PlayLayer::onExit();
        m_botButtonReady = false;
    }
};
