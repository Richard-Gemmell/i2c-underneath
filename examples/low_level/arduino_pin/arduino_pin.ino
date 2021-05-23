// Copyright © 2021 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)
//
// This example shows how a Pin object can control a pin and raise
// events when line state changes.
// It's possible to do all this with Arduino library function such
// as digitalRead(). The main difference is that event callbacks
// are not required to be static functions. This makes it easier
// to build more complicated behaviour.

#include <Arduino.h>
#include <functional>
#include "common/hardware/arduino_pin.h"

IntervalTimer blink_timer;
void blink();

// A class that can handler events.
// See use of std::bind() to use a method to an event handler.
class PinEventHandler
{
    volatile bool line_level = false;
    volatile uint32_t edges_detected = 0;

public:
    void handle_on_edge(bool rising) {
        line_level = rising;
        edges_detected++;
    }

    void log() const {
        Serial.print("Line level is ");Serial.print(line_level);
        Serial.print(". Detected ");Serial.print(edges_detected);Serial.println(" line level changes.");
    }
};
PinEventHandler test_pin_handler;

uint8_t TEST_PIN = 14;

// Create an ArduinoPin. Note the need to register a static
// function to act as an interrupt service routing.
// This is annoying but unavoidable due because attachInterrupt()
// requires a static callback.
common::hardware::ArduinoPin pin(TEST_PIN);
static void on_edge_isr() {
    pin.raise_on_edge();
}

static void static_event_handler(bool rising) {
    Serial.print("Static handler called. Line level is ");Serial.println(rising);
}

void setup() {
    // Blink the LED to show we're alive
    pinMode(LED_BUILTIN, OUTPUT);
    blink_timer.begin(blink, 500'000);

    // Configure the raw pin in the usual way.
    pinMode(TEST_PIN, INPUT_PULLUP);

    // Setup the callback.
    pin.set_on_edge_isr(on_edge_isr);
    pin.on_edge(std::bind(&PinEventHandler::handle_on_edge, &test_pin_handler, std::placeholders::_1));
//    pin.on_edge(static_event_handler);  // You don't need std::bind() for a static function
    delay(200);

    Serial.begin(9600);
    Serial.println("Started ArduinoPin Demo");
}

void loop() {
    pin.write_pin(false);
    delay(100);
    test_pin_handler.log();

    pin.write_pin(true);
    delay(100);
    test_pin_handler.log();

    Serial.println();
    delay(1000);
}

void blink() {
    digitalToggle(LED_BUILTIN);
}
