#include <Arduino.h>
#include <Wire.h>

#define EYE_LEFT_PIN 7
#define EYE_LEFT_CHANNEL 0
#define EYE_RIGHT_PIN 6
#define EYE_RIGHT_CHANNEL 1

byte eye_left_val = 0;
byte eye_right_val = 0;

long prevSerialPrint = 0;
uint16_t serialPrintInterval = 1000 / 15;

void updateLEDs();

void setup()
{

  Serial.begin(115200); // initialize serial communication at 115200 bps, Enable CDC on Boot

  Serial.println("setup ready");
  delay(1000);

  ledcSetup(EYE_LEFT_CHANNEL, 5000, 8);
  ledcAttachPin(EYE_LEFT_PIN, EYE_LEFT_CHANNEL);

  ledcSetup(EYE_RIGHT_CHANNEL, 5000, 8);
  ledcAttachPin(EYE_RIGHT_PIN, EYE_RIGHT_CHANNEL);

  ledcWrite(EYE_LEFT_CHANNEL, 0);
  ledcWrite(EYE_RIGHT_CHANNEL, 0);
}

void loop()
{
  // updateLEDs();
  Serial.println("HIGH");
  ledcWrite(EYE_LEFT_CHANNEL, 255);
  ledcWrite(EYE_RIGHT_CHANNEL, 255);
  delay(1000);

  Serial.println("LOW");
  ledcWrite(EYE_LEFT_CHANNEL, 0);
  ledcWrite(EYE_RIGHT_CHANNEL, 0);
  delay(1000);

  // updateSerialDebugPrint();
  // delay(1000 / 250);
}

void updateLEDs()
{
  // eye_left_val = (byte)map(data.ch[2], 172, 1811, 0, 255);
  // eye_right_val = (byte)map(data.ch[3], 172, 1811, 0, 255);

  // ledcWrite(EYE_LEFT_CHANNEL, eye_left_val);
  // ledcWrite(EYE_RIGHT_CHANNEL, eye_right_val);
}

void updateSerialDebugPrint()
{
  if (millis() - prevSerialPrint > serialPrintInterval)
  {
    prevSerialPrint = millis();
  }
}
