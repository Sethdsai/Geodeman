# Pathfinder

**Pathfinder** is a learning bot + level generator for Geometry Dash, built on [Geode](https://geode-sdk.org/).

## What it does

- **Auto-plays levels** in every gamemode (cube, ship, ball, UFO, wave, robot, spider, swing) using a deterministic pathfinder seeded with a pre-baked policy table that was trained offline against thousands of community levels.
- **Learns as it plays** — every death frame in your session is folded back into the policy weights, so the bot gets noticeably better the more you let it run on a level.
- **Generates levels for fun** — hit the dice button on the main menu to drop into a freshly synthesized level. You can pick a difficulty (1-10) and which gamemodes are allowed.
- **Path overlay** — the planned path is drawn live on top of the level so you can actually see *why* the bot does what it does.

## How to use

1. Install Geode if you haven't.
2. Drop the built `.geode` file into your `geode/mods` folder, or install via the in-game mod browser once published.
3. Launch GD. There's a new compass icon in the bottom-right of the main menu — that's the Pathfinder hub.
4. Toggle **Enable Pathfinder Bot** in mod settings, then start any level.
5. Hit **Generate Level** in the hub to play something the generator just made.

## Notes

- The pre-trained policy lives in `resources/policy.bin` (compact lookup table, ~120 KB). It's not a neural net — it's an action-cost table over (gamemode, local-obstacle-pattern, velocity bucket) triples, distilled from completion replays. This is the same trick xBot-style auto-completers have always used; the "AI" is honest pathfinding, not magic.
- Online learning is purely additive: it never overwrites the shipped table, so disabling it just falls back to stock behavior.
- This mod does not interfere with attempts/leaderboards uploads — using the bot will not submit completions.
