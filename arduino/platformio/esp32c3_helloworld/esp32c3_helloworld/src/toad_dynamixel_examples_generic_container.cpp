#include <Arduino.h>
// #include <SoftwareSerial.h>
#include <Wire.h>
#include <Dynamixel2Arduino.h>

#define DXL_BAUDRATE 57600
#define DXL_DIR_PIN 7 // 37

#define DEBUG_SERIAL Serial
HardwareSerial DXL_SERIAL(0);
// EspSoftwareSerial::UART DXL_SERIAL;

const uint8_t DXL_ID = 1;
// const uint8_t DXL_ID_2 = 1;
const float DXL_PROTOCOL_VERSION = 2.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

// This namespace is required to use Control table item names
using namespace ControlTableItem;

void setup()
{
  // put your setup code here, to run once:

  // Use UART port of DYNAMIXEL Shield to debug.
  DEBUG_SERIAL.begin(115200);
  while (!DEBUG_SERIAL)
    ;

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(DXL_BAUDRATE);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(DXL_ID);
  // dxl.ping(DXL_ID_2);

  // Turn off torque when configuring items in EEPROM area
  dxl.torqueOff(DXL_ID);
  dxl.setOperatingMode(DXL_ID, OP_POSITION);
  dxl.torqueOn(DXL_ID);

  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 30);
}

void loop()
{
  // put your main code here, to run repeatedly:

  // Please refer to e-Manual(http://emanual.robotis.com/docs/en/parts/interface/dynamixel_shield/) for available range of value.
  // Set Goal Position in RAW value
  dxl.setGoalPosition(DXL_ID, 1000);

  int i_present_position = 0;
  float f_present_position = 0.0;

  while (abs(1000 - i_present_position) > 10)
  {
    i_present_position = dxl.getPresentPosition(DXL_ID);
    DEBUG_SERIAL.print("Present_Position(raw) : ");
    DEBUG_SERIAL.println(i_present_position);
  }
  delay(1000);

  // Set Goal Position in DEGREE value
  dxl.setGoalPosition(DXL_ID, 5.7, UNIT_DEGREE);

  while (abs(5.7 - f_present_position) > 2.0)
  {
    f_present_position = dxl.getPresentPosition(DXL_ID, UNIT_DEGREE);
    DEBUG_SERIAL.print("Present_Position(degree) : ");
    DEBUG_SERIAL.println(f_present_position);
  }
  delay(1000);
}
