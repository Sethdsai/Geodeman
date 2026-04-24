#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

namespace pathfinder {

class PathfinderPopup : public geode::Popup<> {
protected:
    bool setup() override;
    void onToggleBot(cocos2d::CCObject*);
    void onGenerateLevel(cocos2d::CCObject*);
    void onOpenSettings(cocos2d::CCObject*);

public:
    static PathfinderPopup* create();
};

} // namespace pathfinder
