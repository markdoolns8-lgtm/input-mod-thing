// Gameplay hooks: early-press buffering + early-release hold protection.
//
// Input path used: GJBaseGameLayer::handleButton(down, button, isPlayer1) is the
// single place where GD 2.2 feeds a press/release to a player. Per-tick work is
// done in GJBaseGameLayer::processCommands, which runs once per PHYSICS step
// (so "ticks", not rendered frames).
#include "Settings.hpp"
#include "InputBuffer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <type_traits>

using namespace geode::prelude;

namespace {
    // Index 0 = player 1, 1 = player 2: fully separate state in dual mode.
    ibuf::InputBuffer   s_buf[2];
    ibuf::HoldProtector s_hold[2];
    bool s_replaying = false;            // our own events must bypass the hooks' logic
    GJBaseGameLayer* s_layer = nullptr;  // detect a new level/layer

    void clearAll() {
        for (int i = 0; i < 2; ++i) { s_buf[i].clear(); s_hold[i].clear(); }
    }

    // m_touchingRings may be a CCArray* or a vector pointer depending on SDK version.
    template <class T>
    bool nonEmpty(T const& c) {
        if constexpr (std::is_pointer_v<T>) return c && nonEmpty(*c);
        else if constexpr (requires { c.size(); }) return c.size() > 0;
        else if constexpr (requires { c.count(); }) return c.count() > 0;
        else static_assert(!sizeof(T), "unsupported m_touchingRings type");
    }

    // Can a fresh press do something RIGHT NOW? If yes, nothing needs buffering.
    // Modes where a press is always meaningful (ship, UFO, wave, swing) never buffer.
    // Ground modes (cube, ball, robot, spider) need to be on the ground, or touching an orb/ring.
    bool canActNow(PlayerObject* p) {
        if (!p) return true;
        if (nonEmpty(p->m_touchingRings)) return true;
        if (p->m_isShip || p->m_isBird || p->m_isDart || p->m_isSwing) return true;
        return p->m_isOnGround;
    }

    // Release extension is only used where HOLDING is the action and a too-short
    // hold is a plausible mistake. Click-style modes (cube, ball, UFO, spider, swing)
    // would get an invented extra jump, and the wave's release is a deliberate
    // direction change, so they are excluded.
    bool holdEligible(PlayerObject* p) {
        if (!p) return false;
        if (p->m_isShip) return true;
        if (p->m_isRobot) return !p->m_isOnGround;   // variable jump height while airborne
        return false;
    }
}

class $modify(BufferGameLayer, GJBaseGameLayer) {
    void sendRaw(bool down, bool isPlayer1) {
        s_replaying = true;
        GJBaseGameLayer::handleButton(down, 1, isPlayer1);
        s_replaying = false;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        // Only the main (jump/hold) button is handled; platformer left/right untouched.
        if (s_replaying || button != 1) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }
        int idx = isPlayer1 ? 0 : 1;
        PlayerObject* p = isPlayer1 ? m_player1 : m_player2;
        bool on = ibuf::cfg::enabled();

        if (down) {
            // If the game still sees an extended hold, end it first so it never gets two presses in a row.
            if (s_hold[idx].onPress()) sendRaw(false, isPlayer1);
            bool couldAct = canActNow(p);          // evaluated BEFORE the game reacts
            GJBaseGameLayer::handleButton(down, button, isPlayer1);   // normal behaviour always preserved
            if (on && !couldAct) s_buf[idx].onPress();
        } else {
            bool swallow = s_hold[idx].onRelease(ibuf::cfg::holdActive(), holdEligible(p), ibuf::cfg::window());
            if (on) s_buf[idx].onRelease();
            if (!swallow) GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }
    }

    void processCommands(float dt) {
        if (s_layer != this) { s_layer = this; clearAll(); }

        for (int idx = 0; idx < 2; ++idx) {
            bool isP1 = idx == 0;
            PlayerObject* p = isP1 ? m_player1 : m_player2;
            if (!p) continue;
            if (p->m_isDead) { s_buf[idx].clear(); s_hold[idx].clear(); continue; }

            auto a = s_buf[idx].step(ibuf::cfg::enabled(), canActNow(p), ibuf::cfg::window());
            if (a == ibuf::Action::Press) sendRaw(true, isP1);
            else if (a == ibuf::Action::Release) sendRaw(false, isP1);

            auto h = s_hold[idx].step(ibuf::cfg::holdActive(), holdEligible(p));
            if (h == ibuf::Action::Release) sendRaw(false, isP1);
        }
        GJBaseGameLayer::processCommands(dt);
    }
};

// Restart / respawn: drop all pending state. GD resets its own buttons here, so no events are sent.
class $modify(BufferPlayLayer, PlayLayer) {
    void resetLevel() {
        clearAll();
        PlayLayer::resetLevel();
    }
};
