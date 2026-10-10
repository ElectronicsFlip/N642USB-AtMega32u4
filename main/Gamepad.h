/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#pragma once

#include <Arduino.h>

#include "HID.h"
#include "HID-Settings.h"

#include "GamepadAPI.h"


static const uint8_t GamepadDescriptor[] PROGMEM =
{
    /* Gamepad with 10 buttons, 1 D-pad and 1 analog stick (N64 layout) */
    0x05, 0x01,                 /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x04,                 /* USAGE (Joystick) */
    0xa1, 0x01,                 /* COLLECTION (Application) */
    0x85, HID_REPORTID_GAMEPAD, /*   REPORT_ID */

    /* 10 Buttons */
    0x05, 0x09,                 /*   USAGE_PAGE (Button) */
    0x19, 0x01,                 /*   USAGE_MINIMUM (Button 1) */
    0x29, 0x0a,                 /*   USAGE_MAXIMUM (Button 10) */
    0x15, 0x00,                 /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,                 /*   LOGICAL_MAXIMUM (1) */
    0x75, 0x01,                 /*   REPORT_SIZE (1) */
    0x95, 0x0a,                 /*   REPORT_COUNT (10) */
    0x81, 0x02,                 /*   INPUT (Data,Var,Abs) */
    0x95, 0x01,                 /*   REPORT_COUNT (1) - pad remaining 6 bits */
    0x75, 0x06,                 /*   REPORT_SIZE (6) */
    0x81, 0x03,                 /*   INPUT (Const,Var,Abs) */

    /* 1 D-pad (hat switch) */
    0x05, 0x01,                 /*   USAGE_PAGE (Generic Desktop) */
    0x09, 0x39,                 /*   USAGE (Hat switch) */
    0x15, 0x01,                 /*   LOGICAL_MINIMUM (1) */
    0x25, 0x08,                 /*   LOGICAL_MAXIMUM (8) */
    0x95, 0x01,                 /*   REPORT_COUNT (1) */
    0x75, 0x04,                 /*   REPORT_SIZE (4) */
    0x81, 0x02,                 /*   INPUT (Data,Var,Abs) */
    0x95, 0x01,                 /*   REPORT_COUNT (1) - pad remaining 4 bits */
    0x75, 0x04,                 /*   REPORT_SIZE (4) */
    0x81, 0x03,                 /*   INPUT (Const,Var,Abs) */

    /* Analog stick: X and Y, signed 8-bit */
    0x09, 0x30,                 /*   USAGE (X) */
    0x09, 0x31,                 /*   USAGE (Y) */
    0x15, 0x81,                 /*   LOGICAL_MINIMUM (-127) */
    0x25, 0x7f,                 /*   LOGICAL_MAXIMUM (127) */
    0x75, 0x08,                 /*   REPORT_SIZE (8) */
    0x95, 0x02,                 /*   REPORT_COUNT (2) */
    0x81, 0x02,                 /*   INPUT (Data,Var,Abs) */

    0xc0                         /* END_COLLECTION */
};


class Gamepad : public GamepadAPI
{
public:
    Gamepad(void)
    {
        static HIDSubDescriptor node(GamepadDescriptor, sizeof(GamepadDescriptor));
        HID().AppendDescriptor(&node);
    }

protected: 
    virtual inline void SendReport(void* data, int length) override
    {
        HID().SendReport(HID_REPORTID_GAMEPAD, data, length);
    }
};
