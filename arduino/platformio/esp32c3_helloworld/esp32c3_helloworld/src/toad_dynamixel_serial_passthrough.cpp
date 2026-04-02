#include <Arduino.h>
#include <Wire.h>
#define DXL_BAUDRATE 57600
#define DXL_DIR_PIN 7 // 37
#define RX_MODE 0
#define TX_MODE 1

#define AX12RESOLUTION 1023
#define MAXPOSITION 800
#define MINPOSITION 200
#define OFFSET 10

#define switchCom(DirPin, Mode) (digitalWrite(DirPin, Mode)) // Switch to TX/RX Mode

HardwareSerial dxlSerial(0);

const uint8_t DXL_ID = 2;

short Position;
String motorModel;

uint16_t serialBufferLength;

const int BUFFER_SIZE = 100; // Define the size of the char array

char receivedData[BUFFER_SIZE]; // Char array to store received data
int dataLength = 0;             // Variable to store the length of received data

#define DELAY_TX_TIME_OFFSET 15
#define DELAY_TX_TIME_MULTP 1600

uint16_t timeDelay;

void setup()
{

  dxlSerial.begin(DXL_BAUDRATE, SERIAL_8N1, RX, TX); // Define your RX and TX pins "Feather S3 44-RX 43-TX"
  Serial.begin(DXL_BAUDRATE);                              // initialize serial communication at 115200 bps, Enable CDC on Boot

  pinMode(DXL_DIR_PIN, OUTPUT);

  Serial.println("setup ready");
  delay(1000);

  timeDelay = abs(((9600 * DELAY_TX_TIME_MULTP) / DXL_BAUDRATE) - DELAY_TX_TIME_OFFSET);

  serialBufferLength = dxlSerial.availableForWrite();
}

char incomingByte;
void loop()
{
  // Check if data is available to read from serial port
  if (Serial.available() > 0)
  {
    // read the while serial buffer
    while (Serial.available() > 0)
    {
      // Read the incoming byte
      incomingByte = Serial.read();

      // Check if received byte is not a newline character or the buffer is full
      // if (incomingByte != '\n' && dataLength < BUFFER_SIZE - 1)
      if (dataLength < BUFFER_SIZE - 1)
      {
        // Store the incoming byte into the char array
        receivedData[dataLength] = incomingByte;
        dataLength++; // Increment data length
      }
    }

    // now write to the dxlSerial
    switchCom(DXL_DIR_PIN, TX_MODE);
    dxlSerial.write(receivedData, dataLength);
    while (dxlSerial.availableForWrite() != serialBufferLength)
    {
      delayMicroseconds(timeDelay);
    }
    switchCom(DXL_DIR_PIN, RX_MODE);

    // Reset data length for the next iteration
    dataLength = 0;
    // Clear the char array
    memset(receivedData, 0, sizeof(receivedData));

  }

  // Check if data is available to read from serial port
  if (dxlSerial.available() > 0)
  {
    // read the while serial buffer
    while (dxlSerial.available() > 0)
    {
      // Read the incoming byte
      incomingByte = Serial.read();

      // Check if received byte is not a newline character or the buffer is full
      if (incomingByte != '\n' && dataLength < BUFFER_SIZE - 1)
      {
        // Store the incoming byte into the char array
        receivedData[dataLength] = incomingByte;
        dataLength++; // Increment data length
      }
    }

    // now write to the dxlSerial
    switchCom(DXL_DIR_PIN, TX_MODE);
    Serial.write(receivedData, dataLength);

    // Reset data length for the next iteration
    dataLength = 0;
    // Clear the char array
    memset(receivedData, 0, sizeof(receivedData));

  }

  // delay(1000 / 250);
}
