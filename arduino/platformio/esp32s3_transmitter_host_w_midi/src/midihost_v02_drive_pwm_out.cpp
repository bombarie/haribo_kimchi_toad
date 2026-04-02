#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// create an instance of the Adafruit_USBD_MIDI class
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// define the MIDI callbacks
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte data1, byte data2);

// define init functions
void initializeMidi();
void initializeNoodOutputs();

// which (GPIO) pins are connected to the noods?
#define noodOutPin1 5
#define noodOutPin2 4

// will contain the pwm values for the noods
uint16_t noodOutVal1 = 0;
uint16_t noodOutVal2 = 0;

// these variables are involved with the timing of the nood updates
long prevNoodWriteTime = 0;
uint16_t noodWriteInterval = 1000 / 60;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  initializeMidi();

  initializeNoodOutputs();

  Serial.println("ready for action");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  // set the value of the noods every so often (ie: not EVERY loop - not necessary)
  if (millis() - prevNoodWriteTime > noodWriteInterval)
  {

    ledcWrite(0, noodOutVal1);
    ledcWrite(1, noodOutVal2);

    prevNoodWriteTime = millis();
  }

  // read any new MIDI messages
  // DO NOT INTRODUCE A DELAY -> the usbMIDI.read() needs to be called rapidly from loop()
  MIDI.read();
}

void initializeMidi()
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
    delay(1);
}

void initializeNoodOutputs()
{
  // define pwm out pins
  ledcSetup(0, 10000, 11); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution
  ledcSetup(1, 10000, 11); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution

  ledcAttachPin(noodOutPin1, 0);
  ledcAttachPin(noodOutPin2, 1);
}

void handleControlChange(byte channel, byte data1, byte data2)
{
  // Serial.println("Receive CC >>  channel: " + String(channel) + ", data1: " + String(data1) + ", data2: " + String(data2));

  // MIDI CC 0
  if (channel == 1 && data1 == 0)
  {
    noodOutVal1 = map(data2, 0, 127, 0, 2047);
    // Serial.println("Receive CC0 >> value: " + String(noodOutVal1));
  }

  // MIDI CC 1
  if (channel == 1 && data1 == 1)
  {
    noodOutVal2 = map(data2, 0, 127, 0, 2047);
    // Serial.println("Receive CC1 >> value: " + String(noodOutVal2));
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
