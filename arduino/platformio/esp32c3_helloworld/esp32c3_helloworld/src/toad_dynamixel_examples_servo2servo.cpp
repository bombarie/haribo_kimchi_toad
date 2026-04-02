#include <Arduino.h>
#include <Wire.h>
#include <Dynamixel.h>

#define DXL_BAUDRATE 57600
#define DXL_SERIAL dxlSerial
#define DXL_PROTOCOL 2
#define DXL_DIR_PIN 7

#define AX12RESOLUTION 1023
#define DYNARESOLUTION  4096

HardwareSerial dxlSerial(0);

const uint8_t DXL_ID_1 = 1;
const uint8_t DXL_ID_2 = 2;

short Position_1, Position_2;
String motorModel_1, motorModel_2;

void setup()
{

  dxlSerial.begin(DXL_BAUDRATE, SERIAL_8N1, RX, TX); // Define your RX and TX pins "Feather S3 44-RX 43-TX"
  Serial.begin(115200);                              // initialize serial communication at 115200 bps, Enable CDC on Boot

  Dynamixel.begin(&dxlSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);

  Serial.println("setup ready");
  delay(1000);

  Dynamixel.setTorque(DXL_ID_1, OFF);

  // set up the motor IS NECESSARY for it to work
  Dynamixel.setTorque(DXL_ID_2, OFF);
  Dynamixel.setOperationMode(DXL_ID_2, POSITION_CTRL);
  Dynamixel.setProfileVelocity(DXL_ID_2, 45);
  Dynamixel.setTorque(DXL_ID_2, ON);

}

void loop()
{

  Position_1 = Dynamixel.readPosition(DXL_ID_1);
  motorModel_1 = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_ID_1)));

  Position_2 = Dynamixel.readPosition(DXL_ID_2);
  motorModel_2 = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_ID_2)));

  Serial.print(motorModel_1);
  Serial.print(" -> Position: ");
  Serial.print((Position_1 * 360) / DYNARESOLUTION);
  Serial.print("°. - ");

  Serial.print(motorModel_2);
  Serial.print(" -> Position: ");
  Serial.print((Position_2 * 360) / DYNARESOLUTION);
  Serial.println("°.");

  Dynamixel.move(DXL_ID_2, Dynamixel.readPosition(DXL_ID_1));

  delay(100);
}
