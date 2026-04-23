#include <Arduino.h>

void setup() {
  Serial.begin(115200); // initialize serial communication at 115200 bps, Enable CDC on Boot
  Serial.println("Serial communication started");
}

void loop() {
  Serial.print("Hello.... ");
  Serial.println(millis() / (float)1000); // print the number of seconds since the program started
  delay(250); // wait for a second
}