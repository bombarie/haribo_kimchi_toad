#include <Arduino.h>

#define rcOutPin1 5
#define rcOutPin2 4

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // define pwm out pins
    ledcSetup(0, 10000, 8); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution
    ledcSetup(1, 10000, 8); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution

    ledcAttachPin(noodOutPin1, 0);
    ledcAttachPin(noodOutPin2, 1);

    Serial.println("hello world - ready for action");
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
    for (int i = 0; i < 256; i++)
    {
        ledcWrite(0, i);
        ledcWrite(1, 255 - i);
        Serial.println("writing pwm values: " + String(i) + ", " + String(255 - i));
        delay(10);
    }
}
