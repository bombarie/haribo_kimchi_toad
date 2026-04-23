/**
 * This example demonstrates using CrsfSerial to send channels data to a
 * full-duplex tranmitter module. It does not implement CRSFShot to sync the
 * mixer to the module's TX timing. The module must be configured separately,
 * as this example does not set a packet rate / telemetry ratio etc.
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include <CrsfSerial.h>

// How often to send channels to the TX module in us, usually 1000000 / Rate e.g. 250Hz = 1000000/250 = 4000us
#define CHANNEL_SEND_INTERVAL_US 4000U
#define CRSF_BAUD_RATE 420000
#define port Serial1

static CrsfSerial crsf(port, CRSF_BAUD_RATE); // 921600

/***
 * This callback is called whenever linkstats is received from the TX module
 ***/
static void packetLinkStatistics(crsfLinkStatistics_t *ls)
{
    Serial.print("RFMD=");
    Serial.print(ls->rf_Mode, DEC);
    Serial.print(" LQ=");
    Serial.print(ls->uplink_Link_quality, DEC);
    Serial.print(" RSS1=");
    Serial.println(ls->uplink_RSSI_1, DEC);
}

static void checkSendChannels()
{
    static uint32_t lastSend;
    uint32_t now = micros();
    if (now - lastSend < CHANNEL_SEND_INTERVAL_US)
        return;
    lastSend = now;

    // insert mixer logic here and use crsf.setChannel() to set all the channel values
    // This just increments channel 1/8 value by 1/2us every time, wrapping around
    auto mixer = [](unsigned int ch, unsigned int step)
    {
        unsigned val = crsf.getChannel(ch) + step;
        if (val > 2012)
            val = 988;
        crsf.setChannel(ch, val);
    };
    mixer(1, 1);
    mixer(8, 2);
    // End mixer code

    crsf.queuePacketChannels();
}

static void setupCrsfChannels()
{
    // Initialize all channels to 1500us
    for (unsigned ch = 1; ch <= CRSF_NUM_CHANNELS; ++ch)
        crsf.setChannel(ch, 1500);
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("Starting CRSF Transmitter Example");

    crsf.begin();

    // Attach any callbacks
    crsf.onPacketLinkStatistics = &packetLinkStatistics;
    setupCrsfChannels();

    delay(500);
    Serial.println("Starting CRSF Transmitter Example - Setup done");
}

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();
    checkSendChannels();
}