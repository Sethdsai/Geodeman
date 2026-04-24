#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include "../ai/Pathfinder.hpp"

using namespace geode::prelude;

class $modify(EditorPauseLayer) {
    void onResume(CCObject* sender) {
        // If the bot is enabled, automatically take over playtests started
        // from the editor pause menu. The hook itself is a no-op beyond
        // forwarding; the PlayLayer post-update hook does the actual driving.
        EditorPauseLayer::onResume(sender);
    }
};
