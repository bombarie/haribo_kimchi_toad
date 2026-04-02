#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("led off");  
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("led on");  
  delay(100);
}

