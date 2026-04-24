#include "PathfinderPopup.hpp"
#include "../ai/LevelGenerator.hpp"

#include <Geode/Geode.hpp>

#include <random>

using namespace geode::prelude;

namespace pathfinder {

bool PathfinderPopup::setup() {
    this->setTitle("Pathfinder");

    auto winSize = m_mainLayer->getContentSize();

    auto banner = CCSprite::create("pathfinder-banner.png"_spr);
    if (banner) {
        banner->setScale(0.55f);
        banner->setPosition({ winSize.width / 2.0f, winSize.height - 70.0f });
        m_mainLayer->addChild(banner);
    }

    auto menu = CCMenu::create();
    menu->setPosition({ winSize.width / 2.0f, winSize.height / 2.0f - 30.0f });

    // IconButtonSprite is Geode's own button class with confirmed API:
    // create(bgSpriteName, icon, text, font)
    auto botSpr = IconButtonSprite::create("GJ_button_01.png", nullptr, "Toggle Bot", "bigFont.fnt");
    botSpr->setScale(0.7f);
    auto botBtn = CCMenuItemSpriteExtra::create(
        botSpr, this, menu_selector(PathfinderPopup::onToggleBot));
    botBtn->setPositionY(40.0f);
    menu->addChild(botBtn);

    auto genSpr = IconButtonSprite::create("GJ_button_02.png", nullptr, "Generate Level", "bigFont.fnt");
    genSpr->setScale(0.7f);
    auto genBtn = CCMenuItemSpriteExtra::create(
        genSpr, this, menu_selector(PathfinderPopup::onGenerateLevel));
    menu->addChild(genBtn);

    auto setSpr = IconButtonSprite::create("GJ_button_04.png", nullptr, "Settings", "bigFont.fnt");
    setSpr->setScale(0.7f);
    auto setBtn = CCMenuItemSpriteExtra::create(
        setSpr, this, menu_selector(PathfinderPopup::onOpenSettings));
    setBtn->setPositionY(-40.0f);
    menu->addChild(setBtn);

    m_mainLayer->addChild(menu);
    return true;
}

void PathfinderPopup::onToggleBot(CCObject*) {
    auto mod = Mod::get();
    bool current = mod->getSettingValue<bool>("bot-enabled");
    mod->setSettingValue<bool>("bot-enabled", !current);
    Notification::create(!current ? "Bot enabled" : "Bot disabled",
                         NotificationIcon::Info)->show();
}

void PathfinderPopup::onGenerateLevel(CCObject*) {
    auto mod = Mod::get();
    int difficulty = static_cast<int>(mod->getSettingValue<int64_t>("generator-difficulty"));
    std::string mix = mod->getSettingValue<std::string>("generator-mode-mix");

    std::vector<GameMode> allowed;
    if (mix == "all") {
        for (int i = 0; i < static_cast<int>(GameMode::COUNT); ++i) {
            allowed.push_back(static_cast<GameMode>(i));
        }
    } else if (mix == "cube")   allowed = { GameMode::Cube };
    else if (mix == "ship")     allowed = { GameMode::Ship };
    else if (mix == "ball")     allowed = { GameMode::Ball };
    else if (mix == "ufo")      allowed = { GameMode::Ufo };
    else if (mix == "wave")     allowed = { GameMode::Wave };
    else if (mix == "robot")    allowed = { GameMode::Robot };
    else if (mix == "spider")   allowed = { GameMode::Spider };
    else if (mix == "swing")    allowed = { GameMode::Swing };

    std::random_device rd;
    auto level = LevelGenerator::generate(difficulty, allowed, rd());

    log::info("Generated '{}' ({} objects)", level.title, level.objects.size());

    Notification::create(
        fmt::format("Generated '{}' ({} objects)", level.title, level.objects.size()),
        NotificationIcon::Success)->show();
}

void PathfinderPopup::onOpenSettings(CCObject*) {
    geode::ModSettingsPopup::create(Mod::get())->show();
}

PathfinderPopup* PathfinderPopup::create() {
    auto ret = new PathfinderPopup();
    if (ret->initAnchored(360.0f, 280.0f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

} // namespace pathfinder
