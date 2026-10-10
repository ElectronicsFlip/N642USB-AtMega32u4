#pragma once

// N64 controller driver for a 16 MHz ATmega32U4 (5V SparkFun Pro Micro / Leonardo).
// 
// The N64 uses a single open-drain data line at 3.3V, 1us = 16 CPU cycles:
//   '0' bit = 3us low, 1us high
//   '1' bit = 1us low, 3us high
// 
// A command ends with a console stop bit (1us low), the reply with a controller
// stop bit. The timing is too tight for digitalWrite(), so the bit-banging is
// cycle-counted AVR assembly run with interrupts off (~170us per poll).

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "N64Buttons.h"

#if F_CPU != 16000000UL
#error "N64Controller timing is cycle-counted for a 16 MHz AVR"
#endif

// Arduino pin 4 on the Pro Micro / Leonardo is PD4. If you move the data wire,
// change all four of these to match the new pin's port and bit.
#define N64_DDR   DDRD
#define N64_PORT  PORTD
#define N64_PIN   PIND
#define N64_BIT   4


class N64Controller
{
public:
    void begin()
    {
        N64_PORT &= ~_BV(N64_BIT);  // no internal pull-up (that would pull to 5V)
        N64_DDR  &= ~_BV(N64_BIT);  // released = high via external 3.3V pull-up
        _connected = false;
    }

    // Polls the controller. Returns false (and leaves `out` neutral) if nothing answered.
    bool read(N64Buttons &out)
    {
        // (Re)identify after power-up or hot-plug. This also makes OEM
        // controllers reset their stick centre.
        if (!_connected)
        {
            const uint8_t info = 0x00;
            uint8_t reply[3];
            if (!transact(&info, 1, reply, sizeof(reply)))
            {
                out = N64Buttons();
                return false;
            }
            _connected = true;
            delayMicroseconds(100);  // let the controller settle before polling
        }

        const uint8_t poll = 0x01;
        uint8_t reply[N64Buttons::ReplyLength];
        if (!transact(&poll, 1, reply, sizeof(reply)))
        {
            _connected = false;
            out = N64Buttons();
            return false;
        }

        out = N64Buttons(reply);
        return true;
    }

    N64Buttons poll()
    {
        if (!_connected)
        {
            const uint8_t info = 0x00;
            uint8_t reply[3];
            if (!transact(&info, 1, reply, sizeof(reply)))
            {
                return {};
            }

            _connected = true;
            delayMicroseconds(100);  // let the controller settle before polling
        }

        const uint8_t poll = 0x01;
        uint8_t reply[N64Buttons::ReplyLength];
        if (!transact(&poll, 1, reply, sizeof(reply)))
        {
            _connected = false;
            return {};
        }
        return N64Buttons(reply);
    }

    bool connected() const { return _connected; }

    // Sends `cmdLen` bytes, then reads `replyLen` bytes. True if the full reply arrived.
    static bool transact(const uint8_t *cmd, uint8_t cmdLen, uint8_t *reply, uint8_t replyLen)
    {
        const uint8_t sreg = SREG;
        cli();
        send(cmd, cmdLen);
        const uint8_t missing = receive(reply, replyLen);
        SREG = sreg;
        return missing == 0;
    }

private:
    bool _connected { false };

    // Cycle counts (at 16 MHz) are noted per instruction; t=0 is the falling edge.
    static inline __attribute__((always_inline)) void send(const uint8_t *data, uint8_t length)
    {
        const uint8_t low  = N64_DDR |  _BV(N64_BIT);  // pin output (port bit is 0) = drive low
        const uint8_t high = N64_DDR & ~_BV(N64_BIT);  // pin input = released
        uint8_t byte, bits, mid, delay;

        asm volatile(
            "ld   %[byte], Z+          \n\t"
            "ldi  %[bits], 8           \n\t"
        "1:                              \n\t"
            "out  %[ddr], %[low]       \n\t"  // t=0   line low
            "mov  %[mid], %[low]       \n\t"  // t=1
            "sbrc %[byte], 7           \n\t"  // t=2   bit 1? -> release at t=16
            "mov  %[mid], %[high]      \n\t"  // t=3   (2 cycles either way)
            "ldi  %[delay], 4          \n\t"  // t=4
        "2: dec  %[delay]              \n\t"  // t=5..15
            "brne 2b                   \n\t"
            "out  %[ddr], %[mid]       \n\t"  // t=16  release if bit is 1
            "lsl  %[byte]              \n\t"  // t=17
            "ldi  %[delay], 9          \n\t"  // t=18
        "3: dec  %[delay]              \n\t"  // t=19..44
            "brne 3b                   \n\t"
            "nop                       \n\t"  // t=45
            "nop                       \n\t"  // t=46
            "nop                       \n\t"  // t=47
            "out  %[ddr], %[high]      \n\t"  // t=48  release (end of a 0 bit)
            "dec  %[bits]              \n\t"  // t=49
            "brne 4f                   \n\t"  // t=50  -> t=52 if more bits in this byte
            "ld   %[byte], Z+          \n\t"  // t=51  next byte
            "ldi  %[bits], 8           \n\t"  // t=53
            "dec  %[len]               \n\t"  // t=54
            "breq 5f                   \n\t"  // t=55  -> t=57 if that was the last byte
            "nop                       \n\t"  // t=56
            "nop                       \n\t"  // t=57
            "nop                       \n\t"  // t=58
            "nop                       \n\t"  // t=59
            "nop                       \n\t"  // t=60
            "nop                       \n\t"  // t=61
            "rjmp 1b                   \n\t"  // t=62  next bit at t=64
        "4: ldi  %[delay], 3           \n\t"  // t=52
        "6: dec  %[delay]              \n\t"  // t=53..60
            "brne 6b                   \n\t"
            "nop                       \n\t"  // t=61
            "rjmp 1b                   \n\t"  // t=62  next bit at t=64
        "5: nop                        \n\t"  // t=57  stop bit
            "nop                       \n\t"  // t=58
            "nop                       \n\t"  // t=59
            "nop                       \n\t"  // t=60
            "nop                       \n\t"  // t=61
            "nop                       \n\t"  // t=62
            "nop                       \n\t"  // t=63
            "out  %[ddr], %[low]       \n\t"  // t=64  stop bit: 1us low
            "ldi  %[delay], 5          \n\t"  // t=65
        "7: dec  %[delay]              \n\t"  // t=66..79
            "brne 7b                   \n\t"
            "out  %[ddr], %[high]      \n\t"  // t=80  release, controller replies next
            : [byte] "=&r" (byte), [bits] "=&d" (bits), [mid] "=&r" (mid),
              [delay] "=&d" (delay), [len] "+r" (length), "+z" (data)
            : [ddr] "I" (_SFR_IO_ADDR(N64_DDR)), [low] "r" (low), [high] "r" (high)
            : "memory"
        );
    }

    // Returns the number of bytes NOT received (0 = success). Every bit re-syncs on
    // its falling edge and samples ~2us later, so the controller's own clock drift
    // doesn't matter. Each wait gives up after ~80us of silence.
    static inline __attribute__((always_inline)) uint8_t receive(uint8_t *data, uint8_t length)
    {
        uint8_t byte = 0, bits, timeout, delay;

        asm volatile(
            "ldi  %[bits], 8           \n\t"
        "1: ldi  %[to], 0              \n\t"  // 256 x 5 cycles = 80us
        "2: sbis %[pin], %[bit]        \n\t"  // wait for falling edge
            "rjmp 3f                   \n\t"
            "dec  %[to]                \n\t"
            "brne 2b                   \n\t"
            "rjmp 9f                   \n\t"  // timed out
        "3: ldi  %[delay], 8           \n\t"  // ~28 cycles after the edge was seen
        "4: dec  %[delay]              \n\t"
            "brne 4b                   \n\t"
            "lsl  %[byte]              \n\t"
            "sbic %[pin], %[bit]       \n\t"  // sample: still low = 0, high = 1
            "ori  %[byte], 1           \n\t"
            "ldi  %[to], 0             \n\t"
        "5: sbic %[pin], %[bit]        \n\t"  // wait for the line to go high again
            "rjmp 6f                   \n\t"
            "dec  %[to]                \n\t"
            "brne 5b                   \n\t"
            "rjmp 9f                   \n\t"  // stuck low
        "6: dec  %[bits]               \n\t"
            "brne 1b                   \n\t"
            "st   Z+, %[byte]          \n\t"
            "ldi  %[bits], 8           \n\t"
            "dec  %[len]               \n\t"
            "brne 1b                   \n\t"
        "9:                              \n\t"
            : [byte] "+d" (byte), [bits] "=&d" (bits), [to] "=&d" (timeout),
              [delay] "=&d" (delay), [len] "+r" (length), "+z" (data)
            : [pin] "I" (_SFR_IO_ADDR(N64_PIN)), [bit] "I" (N64_BIT)
            : "memory"
        );

        return length;
    }
};
