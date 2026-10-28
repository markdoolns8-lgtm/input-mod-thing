#include "SettingsPopup.hpp"
#include "Settings.hpp"

using namespace geode::prelude;

namespace {
    SettingsPopup* s_current = nullptr;
    constexpr char const* kSelected = "GJ_button_01.png";   // green
    constexpr char const* kNormal   = "GJ_button_04.png";   // grey

    ButtonSprite* makeBtn(char const* text, int width) {
        return ButtonSprite::create(text, width, true, "bigFont.fnt", kNormal, 26.f, .6f);
    }
    CCLabelBMFont* makeLabel(CCNode* parent, char const* text, CCPoint pos, float scale,
                             char const* font = "chatFont.fnt") {
        auto l = CCLabelBMFont::create(text, font);
        l->setScale(scale);
        l->setPosition(pos);
        parent->addChild(l);
        return l;
    }
}

SettingsPopup* SettingsPopup::create() {
    auto ret = new SettingsPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void SettingsPopup::open() {
    if (s_current && s_current->getParent()) return;      // no duplicate menus
    if (auto p = create()) {
        s_current = p;
        p->show();
    }
}

void SettingsPopup::refreshIfOpen() {
    if (s_current && s_current->getParent()) s_current->refresh();
}

SettingsPopup::~SettingsPopup() {
    if (s_current == this) s_current = nullptr;
}

void SettingsPopup::onClose(CCObject* sender) {
    if (s_current == this) s_current = nullptr;
    Popup::onClose(sender);
}

bool SettingsPopup::init() {
    if (!Popup::init(370.f, 340.f)) return false;
    setTitle("Input Buffer");
    CCSize size{370.f, 340.f};
    float cx = size.width / 2;
    float y = size.height;

    m_status = makeLabel(m_mainLayer, "", {cx, y - 48}, .45f, "bigFont.fnt");

    // Row: master ON / OFF (same meaning as F6 / F7 - never a toggle)
    makeLabel(m_mainLayer, "Buffering:", {70, y - 80}, .7f);
    m_masterOn = makeBtn("ON", 50);
    m_masterOff = makeBtn("OFF", 50);
    auto b1 = CCMenuItemSpriteExtra::create(m_masterOn, this, menu_selector(SettingsPopup::onMaster));
    b1->setTag(1); b1->setPosition({160, y - 80});
    auto b2 = CCMenuItemSpriteExtra::create(m_masterOff, this, menu_selector(SettingsPopup::onMaster));
    b2->setTag(0); b2->setPosition({225, y - 80});
    m_buttonMenu->addChild(b1); m_buttonMenu->addChild(b2);

    // Row: hold protection ON / OFF
    makeLabel(m_mainLayer, "Hold protection:", {85, y - 112}, .7f);
    m_holdOn = makeBtn("ON", 50);
    m_holdOff = makeBtn("OFF", 50);
    auto h1 = CCMenuItemSpriteExtra::create(m_holdOn, this, menu_selector(SettingsPopup::onHold));
    h1->setTag(1); h1->setPosition({190, y - 112});
    auto h2 = CCMenuItemSpriteExtra::create(m_holdOff, this, menu_selector(SettingsPopup::onHold));
    h2->setTag(0); h2->setPosition({255, y - 112});
    m_buttonMenu->addChild(h1); m_buttonMenu->addChild(h2);

    // Window selector: the 13 allowed values, in the required order.
    makeLabel(m_mainLayer, "Window (physics ticks, 240 per second by default)", {cx, y - 145}, .55f);
    constexpr int perRow = 7;
    float startX = cx - (perRow - 1) * 25.f;
    int i = 0;
    for (int v : ibuf::kWindowOptions) {
        auto spr = makeBtn(std::to_string(v).c_str(), 36);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SettingsPopup::onWindow));
        btn->setTag(v);
        btn->setPosition({startX + (i % perRow) * 50.f, y - 175 - (i / perRow) * 34.f});
        m_buttonMenu->addChild(btn);
        m_windowButtons.emplace_back(v, spr);
        ++i;
    }

    // Short explanations + shortcuts
    makeLabel(m_mainLayer, "Early-press buffering: a click made a few ticks too early", {cx, y - 248}, .5f);
    makeLabel(m_mainLayer, "is kept and fired once the jump / orb becomes possible.", {cx, y - 260}, .5f);
    makeLabel(m_mainLayer, "Hold protection: a very short hold in ship / robot is kept", {cx, y - 278}, .5f);
    makeLabel(m_mainLayer, "down until it has lasted as long as the window.", {cx, y - 290}, .5f);
    makeLabel(m_mainLayer, "F6 = ON     F7 = OFF     F8 = SETTINGS", {cx, y - 315}, .6f, "bigFont.fnt");

    refresh();
    return true;
}

void SettingsPopup::refresh() {
    bool on = ibuf::cfg::enabled();
    bool hold = ibuf::cfg::holdProtectionSetting();
    int win = ibuf::cfg::window();

    m_status->setString(fmt::format("Buffering {}  |  Hold {}  |  Window {}",
                                    on ? "ON" : "OFF", hold ? "ON" : "OFF", win).c_str());
    m_status->setColor(on ? ccColor3B{120, 255, 120} : ccColor3B{255, 120, 120});

    m_masterOn->updateBGImage(on ? kSelected : kNormal);
    m_masterOff->updateBGImage(on ? kNormal : kSelected);
    m_holdOn->updateBGImage(hold ? kSelected : kNormal);
    m_holdOff->updateBGImage(hold ? kNormal : kSelected);
    for (auto& [v, spr] : m_windowButtons) spr->updateBGImage(v == win ? kSelected : kNormal);
}

void SettingsPopup::onMaster(CCObject* s) { ibuf::cfg::setEnabled(static_cast<CCNode*>(s)->getTag() == 1); }
void SettingsPopup::onHold(CCObject* s)   { ibuf::cfg::setHoldProtection(static_cast<CCNode*>(s)->getTag() == 1); }
void SettingsPopup::onWindow(CCObject* s) { ibuf::cfg::setWindow(static_cast<CCNode*>(s)->getTag()); }
