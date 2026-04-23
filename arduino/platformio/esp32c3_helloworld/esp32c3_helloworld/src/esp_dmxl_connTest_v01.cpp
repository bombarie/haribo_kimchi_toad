/***
 * A sketch to test connection from ESP32 to Dynamixel motors, without any other boards on the host PCB.
 * The Serial connection is used to control the motors.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Dynamixel.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

// SET FALSE FOR PRODUCTION MODE
#define USE_SERIAL_DEBUG true

#define DXL_BAUDRATE 57600
#define DXL_PROTOCOL 2
#define DXL_DIR_PIN 7 // 37

#define AX12RESOLUTION 1023
#define DYNARESOLUTION 4096
#define MAXPOSITION 4000
#define MINPOSITION 100
#define OFFSET 10

#define MOTORS_DEADBAND 20

int16_t motor1Value = 0;
int16_t motor2Value = 0;

EspSoftwareSerial::UART softSerial;

const uint8_t DXL_MOTOR_1 = 1;
const uint8_t DXL_MOTOR_2 = 2;

short motor1_velocity;
short motor1_position;
String motor1_model;

short motor2_position;
short motor2_velocity;
String motor2_model;

void updateMotors();
void updateLEDs();
void setMotors();
void setLEDs();
void updateSerialDebugPrint();
void readSerial();

long prevSerialPrint = 0;
uint16_t serialPrintInterval = 1000 / 15;

void setup()
{

  if (USE_SERIAL_DEBUG)
    Serial.begin(115200); // initialize serial communication at 115200 bps, Enable CDC on Boot

  softSerial.begin(DXL_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, 5, 6, false, 95, 11);
  Dynamixel.begin(&softSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);

  if (USE_SERIAL_DEBUG)
    Serial.println("setup ready");
  delay(1000);

  // set up the motor IS NECESSARY for it to work
  Dynamixel.setTorque(DXL_MOTOR_1, OFF);
  Dynamixel.setOperationMode(DXL_MOTOR_1, VELOCITY_CTRL);
  // Dynamixel.setProfileVelocity(DXL_MOTOR_1, 145);
  Dynamixel.setTorque(DXL_MOTOR_1, ON);

  Dynamixel.setTorque(DXL_MOTOR_2, OFF);
  Dynamixel.setOperationMode(DXL_MOTOR_2, VELOCITY_CTRL);
  // Dynamixel.setProfileVelocity(DXL_MOTOR_2, 95);
  Dynamixel.setTorque(DXL_MOTOR_2, ON);

  Dynamixel.move(DXL_MOTOR_1, MINPOSITION);
  Dynamixel.move(DXL_MOTOR_2, MINPOSITION);

  motor1Value = 0;
  motor2Value = 0;
}

void loop()
{
  readSerial();

  updateMotors();

  setMotors();

  if (USE_SERIAL_DEBUG)
  {
    updateSerialDebugPrint();
  }

  delay(1000 / 250);
}

int16_t chData[16];

void readSerial()
{
  if (Serial.available())
  {
    char startByte = Serial.read();
    switch (startByte)
    {
      // MOTOR 1
    case '1':
      Serial.println("Received start byte '1' >> setting motor 1 to -255");
      motor1Value = -255;
      break;
    case '2':
      Serial.println("Received start byte '2' >> setting motor 1 to -170");
      motor1Value = -170;
      break;
    case '3':
      Serial.println("Received start byte '3' >> setting motor 1 to -85");
      motor1Value = -85;
      break;
    case '4':
      Serial.println("Received start byte '4' >> setting motor 1 to 0");
      motor1Value = 0;
      break;
    case '5':
      Serial.println("Received start byte '5' >> setting motor 1 to 85");
      motor1Value = 85;
      break;
    case '6':
      Serial.println("Received start byte '6' >> setting motor 1 to 170");
      motor1Value = 170;
      break;
    case '7':
      Serial.println("Received start byte '7' >> setting motor 1 to 255");
      motor1Value = 255;
      break;
    case 'a':
      Serial.println("Received start byte 'a' >> reading motor 1 state");
      Serial.println(Dynamixel.readRegisterData(DXL_MOTOR_1, 0x46, 1)); // hw error status
      break;
    case 's':
      Serial.println("Received start byte 's' >> resetting motor 1");
      Dynamixel.reset(DXL_MOTOR_1, RESET_NOID_NOBR);                    // soft reboot clears alarm
      break;

      // MOTOR 2
    case 'q':
      Serial.println("Received start byte 'q' >> setting motor 2 to -255");
      motor2Value = -255;
      break;
    case 'w':
      Serial.println("Received start byte 'w' >> setting motor 2 to -170");
      motor2Value = -170;
      break;
    case 'e':
      Serial.println("Received start byte 'e' >> setting motor 2 to -85");
      motor2Value = -85;
      break;
    case 'r':
      Serial.println("Received start byte 'r' >> setting motor 2 to 0");
      motor2Value = 0;
      break;
    case 't':
      Serial.println("Received start byte 't' >> setting motor 2 to 85");
      motor2Value = 85;
      break;
    case 'y':
      Serial.println("Received start byte 'y' >> setting motor 2 to 170");
      motor2Value = 170;
      break;
    case 'u':
      Serial.println("Received start byte 'u' >> setting motor 2 to 255");
      motor2Value = 255;
      break;
    case 'z':
      Serial.println("Received start byte 'z' >> reading motor 2 state");
      Serial.println(Dynamixel.readRegisterData(DXL_MOTOR_2, 0x46, 1)); // hw error status
      break;
    case 'x':
      Serial.println("Received start byte 'x' >> resetting motor 2");
      Dynamixel.reset(DXL_MOTOR_2, RESET_NOID_NOBR);                    // soft reboot clears alarm
      break;

    }
  }
  while (Serial.available())
  {
    // clear the buffer if there are more than 16 channel values (to prevent overflow)
    Serial.read();
  }
}

void updateMotors()
{
  // motor1Value = map(chData[0], SBUS_VAL_MIN, SBUS_VAL_MAX, -255, 255);
  // motor2Value = map(chData[1], SBUS_VAL_MIN, SBUS_VAL_MAX, 255, -255); // reversed

  // apply deadband
  if (abs(motor1Value) < MOTORS_DEADBAND)
  {
    motor1Value = 0;
  }
  if (abs(motor2Value) < MOTORS_DEADBAND)
  {
    motor2Value = 0;
  }

  motor1_position = Dynamixel.readPosition(DXL_MOTOR_1);
  motor1_velocity = Dynamixel.readSpeed(DXL_MOTOR_1);
  motor1_model = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_MOTOR_1)));

  motor2_position = Dynamixel.readPosition(DXL_MOTOR_2);
  motor2_velocity = Dynamixel.readSpeed(DXL_MOTOR_2);
  motor2_model = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_MOTOR_2)));
}

void setMotors()
{
  Dynamixel.setGoalVelocity(DXL_MOTOR_1, motor1Value);
  Dynamixel.setGoalVelocity(DXL_MOTOR_2, motor2Value);
}

void updateSerialDebugPrint()
{
  if (millis() - prevSerialPrint > serialPrintInterval)
  {
    Serial.print(DXL_MOTOR_1);
    Serial.print(" -> motor1Value: ");
    Serial.print(motor1Value);
    Serial.print(" -> Position (calced): ");
    Serial.print((motor1_position * 360) / DYNARESOLUTION);
    Serial.print("°.");
    Serial.print(", velocity: ");
    Serial.print(motor1_velocity);
    Serial.print(" -> model: ");
    Serial.println(motor1_model);

    Serial.print(DXL_MOTOR_2);
    Serial.print(" -> motor2Value: ");
    Serial.print(motor2Value);
    Serial.print(" -> Position (calced): ");
    Serial.print((motor2_position * 360) / DYNARESOLUTION);
    Serial.print("°.");
    Serial.print(", velocity: ");
    Serial.print(motor2_velocity);
    Serial.print(" -> model: ");
    Serial.println(motor2_model);

    Serial.println("");

    prevSerialPrint = millis();
  }
}
