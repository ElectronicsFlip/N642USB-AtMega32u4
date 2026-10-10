# N642USB

An Atmega32U4 based USB Adapter for Nintendo 64 controllers. Physical prototypes are built using the Sparkfun Pro Micro based generic boards commonly available on Amazon and AliExpress, etc.

## License

MIT License. See LICENSE for details.

## How To Build

### Materials Needed

__Required__
* Atmega32u4 based arduino board
* AMS1117-3.3 DC/DC step down converter
* Female N64 controller socket
* 1KΩ Resistor

__Optional__
* 5v LED for Power
* 220Ω Resistor

For the build that is used here, the following boards from Amazon were used: [Example 1](https://www.amazon.com/dp/B09J2FLLD7)

These are based on the Sparkfun Pro Micro but with a USB-C port instead of MicroUSB. The USB-C port was preferred for our builds as we have tons of these cables lying around and it was easier to shape our 3D printed housings (not part of this repository).

You will need a DC-DC 5V - 3.3V step down module. I used these: [Example 1](https://www.amazon.com/dp/B01HXU1NQY)

Use any available Nintendo 64 female socket. An extension cable is an easy way to get one with attached wiring. [Example 1](https://www.amazon.com/dp/B00GPE5GG8) [Example 2](https://www.amazon.com/dp/B0HJMDFP66)

*NOTE: The links here are examples of what to get. These are not direct recommendations, amz is just really easy to reference. Make your own choice based on your needs and budget. Good marketplaces for these items include Amazon, eBay and AliExpress.*

### Wiring

![N64 Controller Pinout](/doc/n64-pinout.gif)

**Pinout (console side)**
* Pin 1: GND
* Pin 2: Data
* Pin 3: 3.3V

### Wiring to the Arduino

The boards linked above are by default set to 5v. The Nintendo 64 controller runs on 3.3V so you will need to wire the VCC from the arduino to the AMS1117-3.3 VIN and VOUT to the port's 3.3V pin (3).

The Pro Micro boards and clones that I use for my builds have multiple GND pins so you can wire all of the ground pins together and use one GND pin or use separate GND pins for each wire. Ultimately there are three here (one to AMS1117-3.3, one to controller socket, one to Power LED).

![N64 to Arduino Wiring Guide](/doc/n64-adapter-wiring.png)

**Wiring**

*N64 Port -> Arduino Pin*

* GND -> GND
* Pin 4 -> Data

*Arduino Pin -> AMS1117-3.3*

* VCC -> VIN
* VOUT -> Controller Socket 3.3V (pin 3)
* GND -> GND

__IMPORTANT:__ The 1KΩ pullup resistor between Data and 3.3V is not optional. Without this resistor you will not get correct readings.

*OPTIONAL: RXI pin is used for LED+ as it's right next to a ground pin for a pin header that we use here to indicate power if you build this into a case. This is entirely optional.*

*WARNING: If you use the optional power LED, ensure you install a 220Ω resistor on either one of the wires to prevent damage to the LED or MCU*
