// F6 = ON, F7 = OFF, F8 = SETTINGS. Fixed meanings, never a toggle cycle.
#include "Settings.hpp"
#include "SettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

// NOTE: if a newer Geometry Dash/Geode adds a 4th parameter (timestamp) to this
// function, the compiler will say so - add it here and pass it through.
class $modify(BufferKeys, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat) {
        // Act only on a real key-down; ignore key-up and OS key-repeat so holding
        // a key can never fire the action more than once.
        if (down && !repeat) {
            switch (key) {
                case KEY_F6: ibuf::cfg::setEnabled(true); break;
                case KEY_F7: ibuf::cfg::setEnabled(false); break;
                case KEY_F8: SettingsPopup::open(); break;
                default: break;
            }
        }
        // F-keys produce no text, so text fields are unaffected; the event is
        // passed on so other mods/GD behave as before.
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
};
