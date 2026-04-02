#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

/*

FYI: Serial Monitor commands:
1 -> outputting only nood1a
2 -> outputting only nood1b
3 -> outputting only nood2a
4 -> outputting only nood2b
a -> outputting all noods (DEFAULT)
q -> outputting noodOutVal1
s -> toggle serial print values

*/

/*

WHAT RC VALUES DID I USE HERE????

*/

// create an instance of the Adafruit_USBD_MIDI class
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// define the MIDI callbacks
void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte data1, byte data2);

// methods
void writeValuesToOutputs();
void overrideNoodOutputValues();
void updateSelectedNoodSendIndex();
void calcNoodOutputValues();
void checkIncomingSerial();
void updateSerialPrintValues();
void serialPrintDebugValues();
void processMIDI(void);
uint8_t mapToActualMinMax_256(uint8_t val, uint8_t range);
uint16_t mapToActualMinMax_1024(uint16_t val, uint16_t range);

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

enum DEADBAND_INPUT_RANGE
{
  _256 = 127,
  _1024 = 1023
};

struct DeadbandMinMax
{
  uint16_t min;
  uint16_t max;
};

// I manually explored when the values at the receiver start moving. The values are capped off at the lower and upper end of the range.
// So, for both a 256 and 1024 range, I found the values where the receiver starts to move and where it stops moving.
DeadbandMinMax deadbandMinMax256 = {10, 117};
DeadbandMinMax deadbandMinMax1024 = {0, 1024}; // {130, 936}

byte channelToPrint = 255;

unsigned long prevSerialPrintMills;
unsigned long serialPrintInterval = 200;

unsigned long prevBitmashChangeChannelMills;
unsigned long bitmashChangeChannelInterval = 1000 / 5;
byte bitmashSendChannel = 0;

uint16_t bitmashed_out = 0;
uint16_t bitmashed_outs[] = {0, 0, 0, 0};

// this is the value amount that we subtract from 127, to allow some deadband between the nood values.
// Effectively, determines the resolution of the noods. 0 is no deadband, 127 is max deadband.
#define NOOD_VALUES_TRANSMISSION_BANDWIDTH 16

uint16_t n00dSegmentIdentifiers[] = {512, 640, 768, 896}; // corresponds to upper bits 100, 101, 110, 111 (based on a 10-bits range)
byte n00dSegmentMaxValue = (127 - NOOD_VALUES_TRANSMISSION_BANDWIDTH); // was 55 (== 63 - 8) -> maybe try 127 - 16? -> update: yes, this works fine!

byte bitmash_nood1a = 0;
byte bitmash_nood1b = 0;
byte bitmash_nood2a = 0;
byte bitmash_nood2b = 0;

bool bSerialPrintValues = false;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  initializeMidi();

  initializeNoodOutputs();

  Serial.println("ready for action");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  checkIncomingSerial();

  // handles cycling through the n00d channels at fixed intervals
  updateSelectedNoodSendIndex();

  calcNoodOutputValues();

  // DEBUG -> hard overwrite -> use to test the reliability of the approach
  if (channelToPrint != 255)
  {
    overrideNoodOutputValues();
  }

  if (bSerialPrintValues)
  {
    updateSerialPrintValues();
  }

  // set the value of the noods every so often (ie: not EVERY loop - not necessary)
  if (millis() - prevNoodWriteTime > noodWriteInterval)
  {

    ledcWrite(0, bitmashed_out); // bitmashed_out
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
  ledcSetup(0, 10000, 10); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution
  ledcSetup(1, 10000, 10); // Setup channel at specified Hz with 8 (0-255), 12 (0-4095), or 16 (0-65535) bit resolution

  ledcAttachPin(noodOutPin1, 0);
  ledcAttachPin(noodOutPin2, 1);
}

void updateSelectedNoodSendIndex()
{
  if (millis() - prevBitmashChangeChannelMills > bitmashChangeChannelInterval)
  {
    bitmashSendChannel++;
    if (bitmashSendChannel > 3)
    {
      bitmashSendChannel = 0;
    }

    prevBitmashChangeChannelMills = millis();
  }
}

void calcNoodOutputValues()
{
  switch (bitmashSendChannel)
  {
  case 0:
    bitmashed_out = n00dSegmentIdentifiers[0];
    bitmashed_out += map(bitmash_nood1a, 0, 127, 0, n00dSegmentMaxValue);
    break;
  case 1:
    bitmashed_out = n00dSegmentIdentifiers[1];
    bitmashed_out += map(bitmash_nood1b, 0, 127, 0, n00dSegmentMaxValue);
    break;
  case 2:
    bitmashed_out = n00dSegmentIdentifiers[2];
    bitmashed_out += map(bitmash_nood2a, 0, 127, 0, n00dSegmentMaxValue);
    break;
  case 3:
    bitmashed_out = n00dSegmentIdentifiers[3];
    bitmashed_out += map(bitmash_nood2b, 0, 127, 0, n00dSegmentMaxValue);
    break;
  }

  bitmashed_out = mapToActualMinMax_1024(bitmashed_out, DEADBAND_INPUT_RANGE::_1024);
  bitmashed_outs[bitmashSendChannel] = bitmashed_out;
}

void overrideNoodOutputValues()
{
  switch (channelToPrint)
  {
  case 0:
    bitmashed_out = n00dSegmentIdentifiers[0];
    bitmashed_out += map(bitmash_nood1a, 0, 127, 0, n00dSegmentMaxValue);
    bitmashed_out = mapToActualMinMax_1024(bitmashed_out, DEADBAND_INPUT_RANGE::_1024);
    bitmashed_outs[0] = bitmashed_out;
    break;
  case 1:
    bitmashed_out = n00dSegmentIdentifiers[1];
    bitmashed_out += map(bitmash_nood1b, 0, 127, 0, n00dSegmentMaxValue);
    bitmashed_out = mapToActualMinMax_1024(bitmashed_out, DEADBAND_INPUT_RANGE::_1024);
    bitmashed_outs[1] = bitmashed_out;
    break;
  case 2:
    bitmashed_out = n00dSegmentIdentifiers[2];
    bitmashed_out += map(bitmash_nood2a, 0, 127, 0, n00dSegmentMaxValue);
    bitmashed_out = mapToActualMinMax_1024(bitmashed_out, DEADBAND_INPUT_RANGE::_1024);
    bitmashed_outs[2] = bitmashed_out;
    break;
  case 3:
    bitmashed_out = n00dSegmentIdentifiers[3];
    bitmashed_out += map(bitmash_nood2b, 0, 127, 0, n00dSegmentMaxValue);
    bitmashed_out = mapToActualMinMax_1024(bitmashed_out, DEADBAND_INPUT_RANGE::_1024);
    bitmashed_outs[3] = bitmashed_out;
    break;
  case 10: // override
    bitmashed_out = noodOutVal1;
    break;
  }
}

void checkIncomingSerial()
{
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    switch (inChar)
    {
    case '1':
      Serial.println("outputting only nood1a");
      channelToPrint = 0;
      break;
    case '2':
      Serial.println("outputting only nood1b");
      channelToPrint = 1;
      break;
    case '3':
      Serial.println("outputting only nood2a");
      channelToPrint = 2;
      break;
    case '4':
      Serial.println("outputting only nood2b");
      channelToPrint = 3;
      break;
    case 'a':
      Serial.println("outputting all noods");
      channelToPrint = 255;
      break;

    case 'q':
      Serial.println("outputting noodOutVal1");
      channelToPrint = 10;
      break;

    case 's':
      bSerialPrintValues = !bSerialPrintValues;
      break;
    }

    // don't know if this is necessary but I always flush the serial buffer
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }
}

void updateSerialPrintValues()
{
  if (millis() - prevSerialPrintMills > serialPrintInterval)
  {
    serialPrintDebugValues();
    prevSerialPrintMills = millis();
  }
}
void serialPrintDebugValues()
{
  //*
  if (channelToPrint == 10)
  {
    Serial.print("noodOutVal1: ");
    Serial.println(noodOutVal1);
  }
  Serial.print("noods: ");
  Serial.print(bitmash_nood1a + String(", "));
  Serial.print(bitmash_nood1b + String(", "));
  Serial.print(bitmash_nood2a + String(", "));
  Serial.print(bitmash_nood2b + String(",\t "));
  Serial.print("bitmashed: nood1a: ");
  Serial.print(bitmashed_outs[0]);
  Serial.print(", nood1b: ");
  Serial.print(bitmashed_outs[1]);
  Serial.print(", nood2a: ");
  Serial.print(bitmashed_outs[2]);
  Serial.print(", nood2b: ");
  Serial.print(bitmashed_outs[3]);
  Serial.print("\t -> bitmashed_outs: ");
  Serial.print(bitmashed_outs[0], BIN);
  Serial.print(", ");
  Serial.print(bitmashed_outs[1], BIN);
  Serial.print(", ");
  Serial.print(bitmashed_outs[2], BIN);
  Serial.print(", ");
  Serial.println(bitmashed_outs[3], BIN);
  //*/

  Serial.println();
}

uint8_t mapToActualMinMax_256(uint8_t val, uint8_t range)
{
  switch (range)
  {
  case DEADBAND_INPUT_RANGE::_256:
    return constrain(map(val, 0, 127, deadbandMinMax256.min, deadbandMinMax256.max), 0, 127);
    break;
  }
}

uint16_t mapToActualMinMax_1024(uint16_t val, uint16_t range)
{
  switch (range)
  {
  case DEADBAND_INPUT_RANGE::_1024:
    // return constrain(map(val, 0, 1023, 127, 936), 0, 1023);
    return constrain(map(val, 0, 1023, deadbandMinMax1024.min, deadbandMinMax1024.max), 0, 1023);
    break;
  }
}

void printBytes(const byte *data, unsigned int size)
{
  while (size > 0)
  {
    byte b = *data++;
    if (b < 16)
      Serial.print('0');
    Serial.print(b, HEX);
    if (size > 1)
      Serial.print(' ');
    size = size - 1;
  }
}

void handleControlChange(byte channel, byte data1, byte data2)
{
  // Serial.println("Receive CC >>  channel: " + String(channel) + ", data1: " + String(data1) + ", data2: " + String(data2));

  // MIDI CC 0
  if (channel == 1 && data1 == 0)
  {
    noodOutVal1 = map(data2, 0, 127, 0, 1023);
    // Serial.println("Receive CC0 >> value: " + String(noodOutVal1));
  }

  // MIDI CC 1
  if (channel == 1 && data1 == 1)
  {
    noodOutVal2 = map(data2, 0, 127, 0, 1023);
    // Serial.println("Receive CC1 >> value: " + String(noodOutVal2));
  }

  // n00d individually addressing
  if (channel == 1 && data1 == 30)
  {
    bitmash_nood1a = data2;
  }
  if (channel == 1 && data1 == 31)
  {
    bitmash_nood1b = data2;
  }
  if (channel == 1 && data1 == 32)
  {
    bitmash_nood2a = data2;
  }
  if (channel == 1 && data1 == 33)
  {
    bitmash_nood2b = data2;
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
