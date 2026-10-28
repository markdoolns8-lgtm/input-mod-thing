#include "../src/InputBuffer.hpp"
#include <cstdio>
#include <cstdlib>
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); std::exit(1);} } while(0)
using namespace ibuf;

int main() {
    CHECK(kWindowOptions.size() == 13);
    const int expect[13] = {60,48,36,32,24,16,12,10,8,6,4,2,1};
    for (int i = 0; i < 13; ++i) CHECK(kWindowOptions[i] == expect[i]);
    CHECK(kDefaultWindow == 16);
    CHECK(normalizeWindow(0) == 16);
    CHECK(normalizeWindow(-5) == 16);
    CHECK(normalizeWindow(7) == 16);
    CHECK(normalizeWindow(61) == 16);
    for (int v : kWindowOptions) CHECK(normalizeWindow(v) == v);

    for (int w : kWindowOptions) {
        InputBuffer b; b.onPress();
        for (int i = 0; i < w - 1; ++i) CHECK(b.step(true, false, w) == Action::None);
        CHECK(b.step(true, true, w) == Action::Press);
        CHECK(!b.pending());

        InputBuffer c; c.onPress();
        for (int i = 0; i < w; ++i) CHECK(c.step(true, false, w) == Action::None);
        CHECK(c.step(true, true, w) == Action::None);
        CHECK(!c.pending());
    }
    { InputBuffer b; b.onPress(); CHECK(b.step(true, true, 1) == Action::Press); }
    { InputBuffer b; b.onPress(); CHECK(b.step(true, false, 1) == Action::None);
      CHECK(b.step(true, true, 1) == Action::None); }

    { InputBuffer b; b.onPress();
      CHECK(b.step(true, true, 16) == Action::Press);
      for (int i = 0; i < 40; ++i) CHECK(b.step(true, true, 16) == Action::None); }

    { InputBuffer b; b.onPress(); b.step(true, false, 16);
      CHECK(b.step(false, true, 16) == Action::None);
      CHECK(b.step(true, true, 16) == Action::None); }

    { InputBuffer b; b.onPress(); b.onRelease();
      CHECK(b.step(true, true, 16) == Action::Press);
      CHECK(b.step(true, true, 16) == Action::Release);
      CHECK(b.step(true, true, 16) == Action::None); }
    { InputBuffer b; b.onPress(); b.onRelease();
      CHECK(b.step(true, true, 16) == Action::Press);
      CHECK(b.step(false, true, 16) == Action::Release); }
    { InputBuffer b; b.onPress();
      CHECK(b.step(true, true, 16) == Action::Press);
      CHECK(b.step(true, true, 16) == Action::None); }

    { InputBuffer b; b.onPress(); b.onRelease(); b.clear();
      CHECK(b.step(true, true, 16) == Action::None); }
    { InputBuffer b; b.onPress(); b.step(true,false,4); b.step(true,false,4); b.step(true,false,4);
      b.onPress();
      CHECK(b.step(true,false,4) == Action::None);
      CHECK(b.step(true,true,4) == Action::Press);
      CHECK(b.step(true,true,4) == Action::None); }
    { InputBuffer p1, p2; p1.onPress();
      CHECK(p2.step(true, true, 16) == Action::None);
      CHECK(p1.step(true, true, 16) == Action::Press); }

    // ---------------- hold protection ----------------
    // extension = window - held, for every window value; never exceeds window
    for (int w : kWindowOptions) {
        for (int held = 0; held <= w + 3; ++held) {
            HoldProtector h; h.onPress();
            for (int i = 0; i < held; ++i) CHECK(h.step(true, true) == Action::None);
            bool swallowed = h.onRelease(true, true, w);
            if (held >= w) { CHECK(!swallowed); CHECK(!h.extending()); continue; }
            CHECK(swallowed);
            int ticks = 0; Action a = Action::None;
            while (a == Action::None && ticks < 1000) { a = h.step(true, true); ++ticks; }
            CHECK(a == Action::Release);
            CHECK(held + ticks == w);            // total hold == window, never more
            CHECK(!h.extending());
            CHECK(h.step(true, true) == Action::None);   // exactly one release
        }
    }
    // hold protection OFF / ineligible state -> release passes through untouched
    { HoldProtector h; h.onPress(); CHECK(!h.onRelease(false, true, 16)); }
    { HoldProtector h; h.onPress(); CHECK(!h.onRelease(true, false, 16)); }
    // stray release (no physical press) never swallowed
    { HoldProtector h; CHECK(!h.onRelease(true, true, 16)); }
    // F7 during an extension -> release goes out on the next tick, nothing stuck
    { HoldProtector h; h.onPress(); h.step(true,true);
      CHECK(h.onRelease(true, true, 16));
      CHECK(h.step(true,true) == Action::None);
      CHECK(h.step(false,true) == Action::Release);
      CHECK(h.step(false,true) == Action::None); }
    // state becomes ineligible mid-extension (e.g. robot lands) -> release
    { HoldProtector h; h.onPress(); CHECK(h.onRelease(true, true, 16));
      CHECK(h.step(true,false) == Action::Release); }
    // new physical press during extension: release-first, then fresh hold, no leftover release
    { HoldProtector h; h.onPress(); CHECK(h.onRelease(true, true, 16));
      CHECK(h.onPress());                 // caller must release first
      for (int i = 0; i < 30; ++i) CHECK(h.step(true,true) == Action::None);
      CHECK(!h.onRelease(true, true, 16)); }   // 30 ticks held >= 16 -> normal release
    // clear drops the extension silently
    { HoldProtector h; h.onPress(); h.onRelease(true,true,16); h.clear();
      CHECK(h.step(true,true) == Action::None); CHECK(!h.extending()); }
    // two players independent
    { HoldProtector a, b; a.onPress(); CHECK(a.onRelease(true,true,16));
      CHECK(!b.extending()); CHECK(b.step(true,true) == Action::None); }

    std::puts("All buffer tests passed");
    return 0;
}
