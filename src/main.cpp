#include <Arduino.h>
void setup() {
// write your initialization code here
    Serial.begin(9600);
    Serial.println("Started");
}

void loop() {
// write your code here
    Serial.println("Loop");
    delay(500);
}