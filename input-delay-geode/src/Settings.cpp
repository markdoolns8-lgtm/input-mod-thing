#include "Settings.hpp"
#include "SettingsPopup.hpp"
#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

namespace {
    constexpr auto kKeyEnabled = "buffering-enabled";
    constexpr auto kKeyWindow  = "buffer-window";
    constexpr auto kKeyHold    = "hold-protection";

    bool s_loaded = false;
    bool s_enabled = true;
    int  s_window = ibuf::kDefaultWindow;
    bool s_hold = true;

    void load() {
        if (s_loaded) return;
        s_loaded = true;
        auto* mod = Mod::get();
        s_enabled = mod->getSavedValue<bool>(kKeyEnabled, true);
        s_hold    = mod->getSavedValue<bool>(kKeyHold, true);
        int raw   = static_cast<int>(mod->getSavedValue<int64_t>(kKeyWindow, ibuf::kDefaultWindow));
        s_window  = ibuf::normalizeWindow(raw);          // missing/corrupt/outdated -> 16
        if (s_window != raw) mod->setSavedValue<int64_t>(kKeyWindow, s_window);
    }

    void toast(std::string const& text, NotificationIcon icon) {
        Notification::create(text, icon, 1.f)->show();
    }
}

namespace ibuf::cfg {

bool enabled() { load(); return s_enabled; }
int window() { load(); return s_window; }
bool holdProtectionSetting() { load(); return s_hold; }

void setEnabled(bool on) {
    load();
    if (s_enabled != on) {
        s_enabled = on;
        Mod::get()->setSavedValue<bool>(kKeyEnabled, on);
    }
    // Feedback even when the state did not change, so the player sees F6/F7 worked.
    toast(on ? "Input buffering: ON" : "Input buffering: OFF",
          on ? NotificationIcon::Success : NotificationIcon::Info);
    SettingsPopup::refreshIfOpen();
}

void setWindow(int ticks) {
    load();
    s_window = ibuf::normalizeWindow(ticks);
    Mod::get()->setSavedValue<int64_t>(kKeyWindow, s_window);
    toast("Buffer window: " + std::to_string(s_window) + " ticks", NotificationIcon::Success);
    SettingsPopup::refreshIfOpen();
}

void setHoldProtection(bool on) {
    load();
    if (s_hold != on) {
        s_hold = on;
        Mod::get()->setSavedValue<bool>(kKeyHold, on);
    }
    toast(on ? "Hold protection: ON" : "Hold protection: OFF",
          on ? NotificationIcon::Success : NotificationIcon::Info);
    SettingsPopup::refreshIfOpen();
}

} // namespace ibuf::cfg
