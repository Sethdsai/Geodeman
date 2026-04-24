#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "../ui/PathfinderPopup.hpp"

using namespace geode::prelude;

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = CCSprite::create("pathfinder-btn.png"_spr);
        if (!sprite) return true;
        sprite->setScale(0.85f);

        auto btn = CCMenuItemSpriteExtra::create(
            sprite, this,
            menu_selector(MenuLayer::onMoreGames)
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

    // Intercept onMoreGames to check if the pathfinder button was pressed.
    // This avoids needing to set m_pfnSelector (protected member) or use
    // menu_selector on a method that doesn't exist in the original class.
    void onMoreGames(CCObject* sender) {
        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
            if (btn->getID() == "pathfinder-button"_spr) {
                onPathfinderHub(sender);
                return;
            }
        }
        MenuLayer::onMoreGames(sender);
    }

    void onPathfinderHub(CCObject*) {
        if (auto popup = pathfinder::PathfinderPopup::create()) {
            popup->show();
        }
    }
};
