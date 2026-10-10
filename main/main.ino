// N64 controller (Joybus, single data line) -> USB Gamepad via GamepadAPI
// Board: 5V / 16 MHz ATmega32U4 (SparkFun Pro Micro). Timing is cycle-counted for 16 MHz.
// 
// HID map:
//   Buttons 1-10: A, B, Z, Start, L, R, C-Up, C-Down, C-Left, C-Right
//   Hat:          D-pad
//   X / Y axes:   analog stick
#include "Gamepad.h"
#include "N64Controller.h"


namespace
{
  constexpr auto PowerLedPin = 0;

  constexpr unsigned long PollInterval = 16; // ~60Hz

  // An OEM stick tops out around +/-80 (worn ones less, some third-party more).
  // Raise this if your stick saturates early, lower it if it can't reach full tilt.
  constexpr int StickRange = 80;

  N64Controller Controller;
  Gamepad gamepad;

  /**
    * Array index map
    *
    * 0:  none                  -> centered
    * 1:  down                  -> down
    * 2:  right                 -> right
    * 3:  down+right            -> down-right
    * 4:  up                    -> up
    * 5:  down+up (invalid)     -> down
    * 6:  up+right              -> up-right
    * 7:  down+up+right (inv.)  -> right
    * 8:  left                  -> left
    * 9:  down+left             -> down-left
    * 10: left+right (invalid)  -> right
    * 11: (invalid)             -> down
    * 12: up+left               -> up-left
    * 13: (invalid)             -> left
    * 14: (invalid)             -> up
    * 15: (invalid)             -> centered
    */

  constexpr int8_t DpadFromDirectionIndex[16] = { 0, 5, 3, 4, 1, 5, 2, 3, 7, 6, 3, 5, 8, 7, 1, 0 };

  int directionIndex(const N64Buttons &buttons)
  {
    return (buttons.down ? 1 : 0) + (buttons.right ? 2 : 0) + (buttons.up ? 4 : 0) + (buttons.left ? 8 : 0);
  }

  int buttonsValue(const N64Buttons &buttons)
  {
    return (buttons.a      ?   1 : 0)
         + (buttons.b      ?   2 : 0)
         + (buttons.z      ?   4 : 0)
         + (buttons.start  ?   8 : 0)
         + (buttons.l      ?  16 : 0)
         + (buttons.r      ?  32 : 0)
         + (buttons.cUp    ?  64 : 0)
         + (buttons.cDown  ? 128 : 0)
         + (buttons.cLeft  ? 256 : 0)
         + (buttons.cRight ? 512 : 0);
  }

  // Scale the N64's roughly +/-StickRange to the full HID -127..127.
  int scaleAxis(int value)
  {
    const long scaled = static_cast<long>(value) * 127 / StickRange;
    return constrain(scaled, -127, 127);
  }


  struct InputState
  {
    int ButtonsValue { -1 };
    int DirectionIndex { -1 };
    int x { 0 };
    int y { 0 };

    InputState() = default;
  
    InputState(const N64Buttons& buttons):
      ButtonsValue{buttonsValue(buttons)},
      DirectionIndex{ directionIndex(buttons) },
      x{ scaleAxis(buttons.stickX) },
      y{ -scaleAxis(buttons.stickY) }
    {}

    bool operator!=(const InputState& other) const
    {
      return other.ButtonsValue != ButtonsValue ||
            other.DirectionIndex != DirectionIndex ||
            other.x != x ||
            other.y != y;
    }
  };

  InputState LastInputState;
}


void pushHidReport(const InputState& currentState)
{
  // Only push a new HID report when something actually changed.
  if (currentState != LastInputState)
  {
    gamepad.buttons(currentState.ButtonsValue);
    gamepad.dPad(DpadFromDirectionIndex[currentState.DirectionIndex]);
    gamepad.xAxis(currentState.x);
    gamepad.yAxis(currentState.y);
    gamepad.write();

    LastInputState = currentState;
  }
}


void enforcePollTiming(unsigned long elapsed)
{
  if (elapsed < PollInterval)
  {
    delay(PollInterval - elapsed);
  }
}


void setup()
{
  Controller.begin();
  gamepad.begin();
  pinMode(PowerLedPin, OUTPUT);
  digitalWrite(PowerLedPin, HIGH);
}


void loop()
{
  const auto pollStart = millis();

  pushHidReport(InputState{ Controller.poll() });
  enforcePollTiming(millis() - pollStart);
}
