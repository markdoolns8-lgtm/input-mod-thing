# Input Buffer (Geometry Dash / Geode)

Early-press buffering + early-release hold protection.

| Key | Action |
|-----|--------|
| F6  | Buffering ON (never turns it off) |
| F7  | Buffering OFF - also stops hold protection and clears pending inputs |
| F8  | Opens the settings menu |

Window options (the same value is used by both features): 60, 48, 36, 32, 24, 16, 12, 10, 8, 6, 4, 2, 1. Default 16.

## Timing unit
The unit is a **physics tick** (one `GJBaseGameLayer::processCommands` call; 240 per second at default TPS). It is not a rendered frame: at 60 FPS the game runs about 4 ticks per frame.

## How it works
* **Early press:** every press is passed to the game normally. If the player could not act at that moment (cube/ball/robot/spider in the air and not touching an orb), the press is remembered. If the player can act within the window, it is fired once. Otherwise it is dropped.
* **Hold protection:** a hold in ship or in the robot's jump that lasts fewer than `window` ticks is kept down until it totals `window` ticks (extension = window - ticks held). Longer holds are released normally.

## Limitations (not verified in-game)
* Hold protection is intentionally NOT applied to cube, ball, UFO, spider, swing (click modes: it would invent a jump) or wave (release is a deliberate direction change).
* Orbs while in ship/UFO/wave/swing are not buffered (a press there is always "valid" for the mode itself).
* In dual mode, if the game routes one click to both players through one call, only that call's player is tracked.
* Nothing here has been run inside Geometry Dash yet.

## Build
Push to GitHub; the workflow `.github/workflows/build.yml` runs unit tests, builds on Windows and uploads the `.geode` as an artifact.
