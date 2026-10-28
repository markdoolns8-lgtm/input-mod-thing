#pragma once
#include <Geode/Geode.hpp>

class SettingsPopup : public geode::Popup {
public:
    static void open();            // no-op if the menu is already open
    static void refreshIfOpen();   // keeps the menu in sync with F6/F7 and setters
    ~SettingsPopup() override;

protected:
    bool init();   // Geode v5: Popup is not templated; init() replaces setup()
    void onClose(cocos2d::CCObject* sender) override;

private:
    static SettingsPopup* create();
    void refresh();
    void onMaster(cocos2d::CCObject* sender);
    void onHold(cocos2d::CCObject* sender);
    void onWindow(cocos2d::CCObject* sender);

    cocos2d::CCLabelBMFont* m_status = nullptr;
    ButtonSprite* m_masterOn = nullptr;
    ButtonSprite* m_masterOff = nullptr;
    ButtonSprite* m_holdOn = nullptr;
    ButtonSprite* m_holdOff = nullptr;
    std::vector<std::pair<int, ButtonSprite*>> m_windowButtons;
};
