#include <Arduino.h>
#include <bus_trace/bus_trace.h>

IntervalTimer blink_timer;
void blink();

void setup() {
    // Blink the LED to show we're alive
    pinMode(LED_BUILTIN, OUTPUT);
    blink_timer.begin(blink, 500'000);

    Serial.begin(9600);
    Serial.println("Started#");
}

void log_size(const char* type, size_t size) {
    Serial.print("sizeof(");
    Serial.print(type);
    Serial.print(") = ");
    Serial.println(size);
}

void loop() {
    Serial.println("Loop");

    log_size("BusEvent", sizeof(BusEvent));
    log_size("BusEvent[72] - one byte of data", sizeof(BusEvent[72]));
    log_size("256 byte I2C message", BusTrace::max_events_required(256, true) * 4);
    log_size("4 byte I2C message", BusTrace::max_events_required(4, true) * 4);

    delay(1000);
}

void blink() {
    digitalToggle(LED_BUILTIN);
}
