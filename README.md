# Pathfinder (Geode mod)

Mod ID: `pathfinder.ueker.geode`

A learning auto-play bot + procedural level generator for Geometry Dash, built on the [Geode](https://geode-sdk.org/) mod loader.

## Build

You'll need the Geode SDK installed and the `GEODE_SDK` environment variable pointing at it.

### Desktop (Windows / macOS / Linux)

```bash
# from the pathfinder-mod/ directory
geode build
```

That produces `pathfinder.ueker.geode.geode` in `build/`. Drop it in your `geode/mods/` folder or install via the Geode CLI:

```bash
geode install build/pathfinder.ueker.geode.geode
```

### Android (arm64 + armv7)

You need the Android NDK installed and `ANDROID_NDK_ROOT` (or `ANDROID_NDK_HOME`) set. Then from the `pathfinder-mod/` directory:

```bash
# 64-bit phones (almost all modern Android devices)
geode build -p android64

# 32-bit phones (older devices)
geode build -p android32
```

Both produce the same `pathfinder.ueker.geode.geode` file in `build-android64/` (or `build-android32/`). The `.geode` file is a zip archive containing a `.so` for the chosen architecture plus the resources — that's what makes the ZArchiver workflow below work.

## Installing on Android with ZArchiver

[ZArchiver](https://play.google.com/store/apps/details?id=ru.zdevs.zarchiver) is the standard way to manage `.geode` files on Android because Geometry Dash on Android keeps its mods folder in app-private storage that the system file picker can't see.

1. **Copy** `pathfinder.ueker.geode.geode` to your phone (USB, Google Drive, Discord — whatever works).
2. Open **ZArchiver** and navigate to where you put the file.
3. Long-press it and choose **Copy**.
4. Navigate to:
   ```
   Android/media/com.geode.launcher/game/geode/mods/
   ```
   *(If you're using the official Geometry Dash app instead of the Geode launcher, the path is `Android/media/com.robtopx.geometryjump/geode/mods/`.)*
5. Paste the file there. Make sure the extension stays `.geode` — do **not** rename it to `.zip`.
6. Launch Geometry Dash. Pathfinder will appear in the in-game mods list, and the compass button will show up in the bottom-right of the main menu.

### If the `Android/media/...` folder is missing

Launch Geode/Geometry Dash once first — it creates the folder on first run. Then go back and paste.

### Updating the mod

Replace the existing `pathfinder.ueker.geode.geode` in the mods folder with the new build. ZArchiver will ask if you want to overwrite — say yes. Restart GD.

### Uninstalling

Delete `pathfinder.ueker.geode.geode` from the mods folder via ZArchiver. Restart GD.

## Android-specific notes

- Touch input is routed through the same `PlayLayer::handleButton` path as desktop clicks, so the bot drives the player identically on phone and PC.
- Online learning persists in memory only — closing GD wipes the session overlay. The shipped policy table (`policy.bin`) is read-only and untouched.
- Generated levels run at the same physics rate (240 Hz planning) on Android; on lower-end devices you can drop **Generator Difficulty** to ease the planner load.

## Project layout

```
pathfinder-mod/
├── mod.json              # Geode manifest, settings, resources
├── CMakeLists.txt        # build config
├── about.md              # shown in the in-game mod page
├── changelog.md
├── resources/            # logo, button sprite, banner
└── src/
    ├── main.cpp          # mod entry, registers everything
    ├── ai/
    │   ├── Pathfinder.hpp/.cpp     # the planner + learned policy
    │   ├── PhysicsModel.hpp/.cpp   # forward-sim of GD physics per gamemode
    │   └── LevelGenerator.hpp/.cpp # procedural level synthesis
    ├── hooks/
    │   ├── PlayLayerHook.cpp       # bot takeover + path overlay
    │   ├── MenuLayerHook.cpp       # hub button on the main menu
    │   └── EditorPauseLayerHook.cpp # "let the bot test this" in the editor
    └── ui/
        └── PathfinderPopup.hpp/.cpp # the hub popup
```

## A note on the "AI"

The bot is a deterministic pathfinder over a forward-simulated GD physics model, scored by a pre-baked action-cost table that was distilled offline from thousands of completion replays. Online learning folds session deaths back into a session-local overlay table. It will beat the vast majority of levels but it is not, and does not claim to be, a neural net trained end-to-end on pixels.
