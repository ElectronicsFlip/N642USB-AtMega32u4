#pragma once

#include <stdint.h>

// Decoded N64 controller state from the 4-byte reply to the poll command (0x01).
//
//   Byte 0: A  B  Z  Start  DUp  DDown  DLeft  DRight     (MSB first)
//   Byte 1: Reset  (0)  L  R  CUp  CDown  CLeft  CRight
//   Byte 2: stick X, signed (right = positive)
//   Byte 3: stick Y, signed (up    = positive)
struct N64Buttons
{
    static constexpr int ReplyLength = 4;

    N64Buttons() = default;

    explicit N64Buttons(const uint8_t reply[ReplyLength]) :
        a(reply[0] & 0x80),
        b(reply[0] & 0x40),
        z(reply[0] & 0x20),
        start(reply[0] & 0x10),
        up(reply[0] & 0x08),
        down(reply[0] & 0x04),
        left(reply[0] & 0x02),
        right(reply[0] & 0x01),
        // reply[1] bit 7 is "reset" (L+R+Start held), bit 6 unused
        l(reply[1] & 0x20),
        r(reply[1] & 0x10),
        cUp(reply[1] & 0x08),
        cDown(reply[1] & 0x04),
        cLeft(reply[1] & 0x02),
        cRight(reply[1] & 0x01),
        stickX(static_cast<int8_t>(reply[2])),
        stickY(static_cast<int8_t>(reply[3]))
    {}

    bool a { false };
    bool b { false };
    bool z { false };
    bool start { false };
    bool up { false };
    bool down { false };
    bool left { false };
    bool right { false };
    bool l { false };
    bool r { false };
    bool cUp { false };
    bool cDown { false };
    bool cLeft { false };
    bool cRight { false };
    int8_t stickX { 0 };
    int8_t stickY { 0 };
};
