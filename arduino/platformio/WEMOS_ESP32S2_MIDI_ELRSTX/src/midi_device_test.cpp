#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// DEBUG
// #define DEBUG

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create a new instance of the Arduino MIDI Library,
// and attach usb_midi as the transport.
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte data1, byte data2);
void BlinkLed(byte num);

int16_t rcChannels[4] = {0, 0, 0, 0};
#define AILERON 0
#define ELEVATOR 1
#define THROTTLE 2
#define RUDDER 3

void BlinkLed(byte num) // Basic blink function
{
    for (byte i = 0; i < num; i++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
    }
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef DEBUG
    Serial.begin(115200);
#endif

    usb_midi.setStringDescriptor("TinyUSB MIDI");

    // Initialize MIDI, and listen to all MIDI channels
    // This will also call usb_midi's begin()
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // Attach the handleNoteOn function to the MIDI Library. It will
    // be called whenever the Bluefruit receives MIDI Note On messages.
    MIDI.setHandleNoteOn(handleNoteOn);

    // Do the same for MIDI Note Off messages.
    MIDI.setHandleNoteOff(handleNoteOff);

    MIDI.setHandleControlChange(handleControlChange);

    // Serial.begin(115200);

    // wait until device mounted
    while (!TinyUSBDevice.mounted())
        delay(1);
}

void loop()
{
    // read any new MIDI messages
    // DO NOT INTRODUCE A DELAY -> the usbMIDI.read() needs to be called rapidly from loop()
    MIDI.read();
}

void handleControlChange(byte channel, byte data1, byte data2)
{
    // Serial.println("Receive CC >>  channel: " + String(channel) + ", data1: " + String(data1) + ", data2: " + String(data2));

    if (channel == 1 && data1 == 0)
    {
        rcChannels[AILERON] = data2;
    }

    if (channel == 1 && data1 == 1)
    {
        rcChannels[ELEVATOR] = data2;
    }

    if (channel == 1 && data1 == 2)
    {
        rcChannels[THROTTLE] = data2;
    }

    if (channel == 1 && data1 == 3)
    {
        rcChannels[RUDDER] = data2;
    }
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Log when a note is pressed.
    Serial.print("Note on: channel = ");
    Serial.print(channel);

    Serial.print(" pitch = ");
    Serial.print(pitch);

    Serial.print(" velocity = ");
    Serial.println(velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Log when a note is released.
    Serial.print("Note off: channel = ");
    Serial.print(channel);

    Serial.print(" pitch = ");
    Serial.print(pitch);

    Serial.print(" velocity = ");
    Serial.println(velocity);
}