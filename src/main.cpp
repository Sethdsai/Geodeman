#include <Geode/Geode.hpp>
#include "ai/Pathfinder.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("Pathfinder loaded - bot ready, generator armed.");
    pathfinder::Pathfinder::get().loadPretrainedPolicy();
}
