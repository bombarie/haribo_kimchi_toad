#include <Arduino.h>
#include <HardwareSerial.h>
#include <CrsfSerial.h>

// Pass any HardwareSerial port
// "Arduino" users (atmega328) can not use CRSF_BAUDRATE, as the atmega does not support it
// and should pass 250000, but then also must flash the receiver with RCVR_UART_BAUD=250000
// Also note the atmega only has one Serial, so logging to Serial must be removed
CrsfSerial crsf(Serial1, CRSF_BAUDRATE);

/***
 * This callback is called whenever new channel values are available.
 * Use crsf.getChannel(x) to get us channel values (1-16).
 ***/
void packetChannels()
{
    // print channels 1 through 16
    for (int i = 1; i <= 16; i++)
    {
        Serial.print("CH");
        Serial.print(i);
        Serial.print("=");
        Serial.print(crsf.getChannel(i));
        Serial.print(" ");
    }
    Serial.println();

    // Serial.print("CH1=");
    // Serial.println(crsf.getChannel(1));
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting CRSF Receiver Example");

    crsf.begin();

    // Attach the channels callback
    crsf.onPacketChannels = &packetChannels;
    
    Serial.println("Channels callback attached");
}

long prevMillis = 0;
uint16_t printInterval = 1000 / 10;

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();

}