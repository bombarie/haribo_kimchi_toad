/**
 * This example demonstrates using CrsfSerial to send channels data to a
 * full-duplex tranmitter module. It does not implement CRSFShot to sync the
 * mixer to the module's TX timing. The module must be configured separately,
 * as this example does not set a packet rate / telemetry ratio etc.
 */

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <HardwareSerial.h>
#include <CrsfSerial.h>

// #define DEBUG // if not commented out, Serial.print() is active! For debugging only!!

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void printBytes(const byte *data, unsigned int size);

void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte data1, byte data2);

// Basic setup
#define CRSF_MAX_CHANNEL 16

// Define AUX channel input limite
#define CRSF_DIGITAL_CHANNEL_MIN 1000 // 172
#define CRSF_DIGITAL_CHANNEL_MAX 2000

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
    usb_midi.setStringDescriptor("TinyUSB MIDI");

    // Initialize MIDI, and listen to all MIDI channels
    // This will also call usb_midi's begin()
    MIDI.begin(MIDI_CHANNEL_OMNI);

    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);

    // wait until device mounted
    while (!TinyUSBDevice.mounted())
    {
        // Serial.println("Waiting for USB MIDI device to be mounted...");
        delay(1);
    }

    delay(500);

#ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Starting CRSF Transmitter Example");
#else
    crsf.begin();
#endif

    // Attach any callbacks
    crsf.onPacketLinkStatistics = &packetLinkStatistics;
    setupCrsfChannels();

    delay(500);

#ifdef DEBUG
    Serial.println("Starting CRSF Transmitter Example - Setup done");
#endif

    // Set channels [0-4] (ie override left/right motor and left/right eye).
    // Channel order: AETR
    crsf.setChannel(1, (CRSF_DIGITAL_CHANNEL_MIN + CRSF_DIGITAL_CHANNEL_MAX) / 2);
    crsf.setChannel(2, (CRSF_DIGITAL_CHANNEL_MIN + CRSF_DIGITAL_CHANNEL_MAX) / 2);
    crsf.setChannel(3, CRSF_DIGITAL_CHANNEL_MIN);
    crsf.setChannel(4, CRSF_DIGITAL_CHANNEL_MIN);

    // turn LED off to indicate ready
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

long prevMillis = 0;
uint16_t channelsPrintInterval = 1000 / 5;

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();
    checkSendChannels();

    if (millis() - prevMillis > channelsPrintInterval)
    {

        prevMillis = millis();
#ifdef DEBUG
        Serial.println("Current channel values:");
        for (unsigned ch = 1; ch <= CRSF_NUM_CHANNELS; ++ch)
        {
            Serial.print("Channel ");
            Serial.print(ch);
            Serial.print(": ");
            Serial.print(crsf.getChannel(ch));
            Serial.print("us, mapped: ");
            Serial.print(map(crsf.getChannel(ch), CRSF_DIGITAL_CHANNEL_MIN, CRSF_DIGITAL_CHANNEL_MAX, 0, 255));
            Serial.println(" (0-255)");
        }
        Serial.println();
#endif
    }

    // read any new MIDI messages
    // DO NOT USE 'DELAY()' -> the usbMIDI.read() needs to be called rapidly from loop()
    MIDI.read();
}

void handleControlChange(byte channel, byte data1, byte data2)
{
    // Serial.println("Receive CC >>  channel: " + String(channel) + ", data1: " + String(data1) + ", data2: " + String(data2));

    if (channel == 1)
    {
        for (int i = 1; i < CRSF_MAX_CHANNEL; i++)
        {
            if (data1 == i)
            {
                crsf.setChannel(i, map(data2, 0, 127, CRSF_DIGITAL_CHANNEL_MIN, CRSF_DIGITAL_CHANNEL_MAX));
            }
        }
    }
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
#ifdef DEBUG
    // Log when a note is pressed.
    Serial.print("Note on: channel = ");
    Serial.print(channel);

    Serial.print(" pitch = ");
    Serial.print(pitch);

    Serial.print(" velocity = ");
    Serial.println(velocity);
#endif
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
#ifdef DEBUG
    // Log when a note is released.
    Serial.print("Note off: channel = ");
    Serial.print(channel);

    Serial.print(" pitch = ");
    Serial.print(pitch);

    Serial.print(" velocity = ");
    Serial.println(velocity);
#endif
}