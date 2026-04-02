#include <Arduino.h>
#include <Wire.h>
#include <Dynamixel.h>
#include <SoftwareSerial.h>

#define DXL_BAUDRATE 57600
#define DXL_SERIAL dxlSerial
#define DXL_PROTOCOL 2
#define DXL_DIR_PIN 7 // 37

#define AX12RESOLUTION 1023
#define DYNARESOLUTION  4096
#define MAXPOSITION 4000
#define MINPOSITION 100
#define OFFSET 10

HardwareSerial dxlSerial(0);
EspSoftwareSerial::UART softSerial;

const uint8_t DXL_ID = 2;

short Position;
String motorModel;

void setup()
{
  Serial.begin(115200);                              // initialize serial communication at 115200 bps, Enable CDC on Boot

  // dxlSerial.begin(DXL_BAUDRATE, SERIAL_8N1, RX, TX); // Define your RX and TX pins "Feather S3 44-RX 43-TX"
  softSerial.begin(DXL_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, 5, 6, false, 95, 11);

  // Dynamixel.begin(&dxlSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);
  Dynamixel.begin(&softSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);

  Serial.println("setup ready");
  delay(1000);

  // set up the motor IS NECESSARY for it to work
  Dynamixel.setTorque(DXL_ID, OFF);
  Dynamixel.setOperationMode(DXL_ID, POSITION_CTRL);
  Dynamixel.setProfileVelocity(DXL_ID, 45);
  Dynamixel.setTorque(DXL_ID, ON);

  Dynamixel.move(DXL_ID, MINPOSITION);
}

void loop()
{
  Position = Dynamixel.readPosition(DXL_ID);
  motorModel = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_ID)));

  Serial.print(motorModel);
  Serial.print(" -> Position (raw): ");
  Serial.print(Position);
  Serial.print(" -> Position (calced): ");
  Serial.print((Position * 360) / DYNARESOLUTION);
  Serial.println("°.");

  if ((Dynamixel.readPosition(DXL_ID) > MAXPOSITION - OFFSET) && (Dynamixel.readPosition(DXL_ID) < MAXPOSITION + OFFSET))
  {
    Serial.println("move to MINPOSITION");
    Dynamixel.move(DXL_ID, MINPOSITION);
  }

  if ((Dynamixel.readPosition(DXL_ID) > MINPOSITION - OFFSET) && (Dynamixel.readPosition(DXL_ID) < MINPOSITION + OFFSET))
  {
    Serial.println("move to MAXPOSITION");
    Dynamixel.move(DXL_ID, MAXPOSITION);
  }

  delay(100);
}
