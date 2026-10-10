/*
Original Copyright (c) 2014-2015 NicoHood

Modified by Lee Dicker, Oct. 2026 for internal use.

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
#include "HID-Settings.h"


typedef struct
{
    uint16_t buttons;  // bits 0-9 = buttons 1-10, bits 10-15 unused
    uint8_t dPad : 4;  // 1-8 = direction (matches GAMEPAD_DPAD_* consts), 0/out-of-range = centered
    uint8_t : 4;        // padding
    int8_t xAxis;      // -127 (left) .. 127 (right)
    int8_t yAxis;      // -127 (up)   .. 127 (down)
} __attribute__((packed)) HID_GamepadReport_Data_t;

class GamepadAPI
{
public:
	GamepadAPI()	{}

	inline void begin(void)
	{
		end();
	}

	inline void end(void)
	{
		memset(&_report, 0x00, sizeof(_report));
		SendReport(&_report, sizeof(_report));
	}

	inline void write(void)
	{
		SendReport(&_report, sizeof(_report));
	}

	inline void press(uint8_t b)
	{
		_report.buttons |= (uint32_t)1 << (b - 1);
	}

	inline void release(uint8_t b)
	{
		_report.buttons &= ~((uint32_t)1 << (b - 1));
	}

	inline void releaseAll(void)
	{
		memset(&_report, 0x00, sizeof(_report));
	}

	inline void buttons(uint32_t b)
	{
		_report.buttons = b;
	}

	inline void dPad(int8_t d)
	{
		_report.dPad = d;
	}

	inline void xAxis(int8_t x)
	{
		_report.xAxis = x;
	}

	inline void yAxis(int8_t y)
	{
		_report.yAxis = y;
	}

	virtual void SendReport(void* data, int length) = 0;

protected:
	HID_GamepadReport_Data_t _report;
};
