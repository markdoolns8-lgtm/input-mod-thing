#pragma once
#include "InputBuffer.hpp"

// Global, persisted settings. All state changes go through here so the
// keyboard shortcuts, the F8 menu and the gameplay hooks always agree.
namespace ibuf::cfg {
    bool enabled();                 // master switch (F6 = true, F7 = false)
    void setEnabled(bool on);       // never toggles; never touches the window
    int  window();                  // always one of ibuf::kWindowOptions
    void setWindow(int ticks);      // invalid values fall back to the default
    bool holdProtectionSetting();   // the separate hold-protection switch
    void setHoldProtection(bool on);
    // Hold protection really runs only if BOTH master and hold switch are on.
    inline bool holdActive() { return enabled() && holdProtectionSetting(); }
}
