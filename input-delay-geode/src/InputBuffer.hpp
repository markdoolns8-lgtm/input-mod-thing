#pragma once
// Pure buffering logic. No Geode/GD dependency, so it can be unit-tested anywhere.
//
// TIMING UNIT: one "tick" = one call to GJBaseGameLayer::processCommands, which
// the game runs once per physics step (240 per second at the default TPS).
// This is NOT a rendered frame: at 60 FPS the game runs ~4 ticks per frame.
#include <array>

namespace ibuf {

inline constexpr std::array<int, 13> kWindowOptions{60, 48, 36, 32, 24, 16, 12, 10, 8, 6, 4, 2, 1};
inline constexpr int kDefaultWindow = 16;

inline bool isValidWindow(int v) {
    for (int o : kWindowOptions) if (o == v) return true;
    return false;
}
// Anything not in the 13 allowed values (missing, corrupt, outdated) -> default.
inline int normalizeWindow(int v) { return isValidWindow(v) ? v : kDefaultWindow; }

enum class Action { None, Press, Release };

// One instance per player (player 1 / player 2) so dual mode stays isolated.
class InputBuffer {
public:
    // The player pressed while the action was NOT possible: remember it.
    // A newer press replaces an older pending one (a press is consumed at most once).
    void onPress() {
        m_pending = true;
        m_age = 0;
        m_releasedEarly = false;
        m_releaseDue = false;
    }
    // Player let go while a press is still pending: remember it so we can
    // give a short tap instead of a stuck hold.
    void onRelease() {
        if (m_pending) m_releasedEarly = true;
    }
    void clear() { m_pending = m_releasedEarly = m_releaseDue = false; m_age = 0; }

    // Called once per tick. Tells the caller what (if anything) to send to the game.
    Action step(bool enabled, bool eligible, int window) {
        // A synthetic release must always go out, even if buffering was just
        // disabled, otherwise the button could stay held.
        if (m_releaseDue) { m_releaseDue = false; return Action::Release; }
        if (!m_pending) return Action::None;
        if (!enabled) { m_pending = false; return Action::None; }   // F7: discard
        if (m_age >= window) { m_pending = false; return Action::None; } // expired
        if (eligible) {
            m_pending = false;
            m_releaseDue = m_releasedEarly;  // tap: release on the next tick
            m_releasedEarly = false;
            return Action::Press;
        }
        ++m_age;
        return Action::None;
    }

    bool pending() const { return m_pending; }

private:
    bool m_pending = false;
    bool m_releasedEarly = false;
    bool m_releaseDue = false;
    int m_age = 0;
};

// Early-release hold protection (one instance per player).
//
// Semantics: a hold shorter than `window` ticks is treated as "released too
// early" and is kept down until it has lasted `window` ticks in total.
//   extension = window - ticksHeld        (never more than `window`, never a fresh window)
// A hold that already lasted >= window is released normally.
// The game-visible button stays down only while m_extending; the physical
// state (m_down) is tracked separately.
class HoldProtector {
public:
    // Physical press. Returns true if the game still believes the button is held
    // from an extended hold: the caller must send a release to the game FIRST so
    // the game never sees two presses in a row.
    bool onPress() {
        bool releaseFirst = m_extending;
        m_down = true;
        m_extending = false;
        m_held = 0;
        m_remaining = 0;
        return releaseFirst;
    }
    // Physical release. Returns true if the release must be SWALLOWED (not passed
    // to the game); the protector will emit Release later from step().
    bool onRelease(bool enabled, bool eligible, int window) {
        if (!m_down) return false;            // stray release: pass through
        m_down = false;
        if (enabled && eligible && m_held < window) {
            m_extending = true;
            m_remaining = window - m_held;    // always >= 1 and <= window
            return true;
        }
        return false;
    }
    // Once per tick. Release => caller must send a release to the game.
    Action step(bool enabled, bool eligible) {
        if (m_down) { if (m_held < 1000000) ++m_held; return Action::None; }
        if (!m_extending) return Action::None;
        --m_remaining;
        // End the extension when time is up, the feature was turned off (F7 /
        // hold toggle), or the state no longer allows it. Always release so
        // the game-visible input can never stay stuck.
        if (m_remaining <= 0 || !enabled || !eligible) {
            m_extending = false;
            return Action::Release;
        }
        return Action::None;
    }
    // Death / restart / level change: forget everything WITHOUT sending events
    // (the game resets its own buttons on those transitions).
    void clear() { m_down = m_extending = false; m_held = m_remaining = 0; }

    bool extending() const { return m_extending; }

private:
    bool m_down = false;
    bool m_extending = false;
    int m_held = 0;
    int m_remaining = 0;
};

} // namespace ibuf
