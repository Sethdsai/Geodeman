#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "../ui/PathfinderPopup.hpp"

using namespace geode::prelude;

// Use named $modify so we can reference MyMenuLayer in menu_selector.
// This matches the official Geode example-mod pattern.
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = CCSprite::create("pathfinder-btn.png"_spr);
        if (!sprite) return true;
        sprite->setScale(0.85f);

        auto btn = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(MyMenuLayer::onPathfinderHub)
        );
        btn->setID("pathfinder-button"_spr);

        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            auto fallback = CCMenu::create();
            fallback->setPosition({ CCDirector::sharedDirector()->getWinSize().width - 40.0f, 40.0f });
            fallback->addChild(btn);
            this->addChild(fallback);
        }
        return true;
    }

    void onPathfinderHub(CCObject*) {
        if (auto popup = pathfinder::PathfinderPopup::create()) {
            popup->show();
        }
    }
};
