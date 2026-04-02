#include <Arduino.h>
#include <Wire.h>
#include <Dynamixel.h>
#include <SoftwareSerial.h>
#include "sbus.h"
#include <HardwareSerial.h>

// MAKE THIS FALSE IN PRODUCTION
#define USE_SERIAL_DEBUG true

#define DXL_BAUDRATE 57600
#define DXL_PROTOCOL 2
#define DXL_DIR_PIN 7 // 37

#define AX12RESOLUTION 1023
#define DYNARESOLUTION 4096
#define MAXPOSITION 4000
#define MINPOSITION 100
#define OFFSET 10

#define EYE_LEFT_PIN 2
#define EYE_LEFT_CHANNEL 0
#define EYE_RIGHT_PIN 3
#define EYE_RIGHT_CHANNEL 1

#define SBUS_VAL_MIN 172  // 191
#define SBUS_VAL_MAX 1811 // 1793
#define SBUS_VAL_CENTER 992
#define SBUS_VAL_DEADBAND 6
#define SBUS_LOST_TIMEOUT 100

int16_t motor1Value = 0;
int16_t motor2Value = 0;

byte eye_left_val = 0;
byte eye_right_val = 0;

EspSoftwareSerial::UART softSerial;

const uint8_t DXL_MOTOR_1 = 1;
const uint8_t DXL_MOTOR_2 = 2;

short motor1_velocity;
short motor1_position;
String motor1_model;

short motor2_position;
short motor2_velocity;
String motor2_model;

HardwareSerial sbusSerial(0);

/* SBUS object, reading SBUS */
bfs::SbusRx sbus_rx(&sbusSerial, D7, D6, true, false);
/* SBUS object, writing SBUS */
// bfs::SbusTx sbus_tx(&Serial2);
/* SBUS data */
bfs::SbusData data;

#define SBUS_PACKET_PRINT_INTERVAL 1000 / 100 // ms
u_long sbusPacketPrintPrevTime = 0;

u_long sbusPrevPacketTime;
// bool sbusLost = false;

#define SBUS_LOST_TIMEOUT 100

void parseSBUS(bool serialPrint);
void updateMotors();
void updateLEDs();
void setMotors();
void setLEDs();
void updateSerialDebugPrint();
void resetSbusData();
void updateLEDs_disconnected();

long prevSerialPrint = 0;
uint16_t serialPrintInterval = 1000 / 15;

// The CONNECTION_ESTABLISHED and CONNECTION_LOST states are kind of vanity states, in case I want to
// implement some kind of unique visual feedback for when the connection is lost or established.
enum CONNECTION_STATE
{
  DISCONNECTED = 0,
  CONNECTION_ESTABLISHED = 1,
  CONNECTION_LOST = 2,
  CONNECTED = 3
};
CONNECTION_STATE connectionState = DISCONNECTED;

void setup()
{

  if (USE_SERIAL_DEBUG)
    Serial.begin(115200); // initialize serial communication at 115200 bps, Enable CDC on Boot

  softSerial.begin(DXL_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, 5, 6, false, 95, 11);
  Dynamixel.begin(&softSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);

  /* Begin the SBUS communication */
  sbus_rx.Begin();

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

  ledcSetup(EYE_LEFT_CHANNEL, 5000, 8);
  ledcAttachPin(EYE_LEFT_PIN, EYE_LEFT_CHANNEL);

  ledcSetup(EYE_RIGHT_CHANNEL, 5000, 8);
  ledcAttachPin(EYE_RIGHT_PIN, EYE_RIGHT_CHANNEL);

  ledcWrite(EYE_LEFT_CHANNEL, 0);
  ledcWrite(EYE_RIGHT_CHANNEL, 0);

  resetSbusData();
}

void loop()
{
  parseSBUS(false);

  // determine how to set the head and body lights
  switch (connectionState)
  {
  case DISCONNECTED:
    resetSbusData();
    updateMotors();
    updateLEDs_disconnected();
    break;
  case CONNECTION_ESTABLISHED:
    // maybe do some temporary transition lighting here from disconnected to connected, but for now just go to connected
    connectionState = CONNECTED;
    break;
  case CONNECTION_LOST:
    // maybe do some temporary transition lighting here from connected to disconnected, but for now just go to disconnected
    connectionState = DISCONNECTED;
    break;
  case CONNECTED:
    updateMotors();
    updateLEDs();
    break;
  }

  setMotors();
  setLEDs();

  if (USE_SERIAL_DEBUG)
  {
    // updateSerialDebugPrint();
  }

  delay(1000 / 250);
}

void updateMotors()
{
  motor1Value = map(data.ch[0], SBUS_VAL_MIN, SBUS_VAL_MAX, -255, 255);
  motor2Value = map(data.ch[1], SBUS_VAL_MIN, SBUS_VAL_MAX, 255, -255); // reversed

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

void updateLEDs()
{
  eye_left_val = (byte)map(data.ch[2], 172, 1811, 0, 255);
  eye_right_val = (byte)map(data.ch[3], 172, 1811, 0, 255);

  ledcWrite(EYE_LEFT_CHANNEL, eye_left_val);
  ledcWrite(EYE_RIGHT_CHANNEL, eye_right_val);
}

void setLEDs()
{
  eye_left_val = (byte)map(data.ch[2], 172, 1811, 0, 255);
  eye_right_val = (byte)map(data.ch[3], 172, 1811, 0, 255);

  ledcWrite(EYE_LEFT_CHANNEL, eye_left_val);
  ledcWrite(EYE_RIGHT_CHANNEL, eye_right_val);
}

void resetSbusData()
{
  for (int8_t i = 0; i < data.NUM_CH; i++)
  {
    data.ch[i] = SBUS_VAL_MIN;
    if (i < 2)
    {
      data.ch[i] = (SBUS_VAL_MIN + SBUS_VAL_MAX) / 2;
    }
  }
}

#define LEFT_MOTOR_CHANNEL 0
#define RIGHT_MOTOR_CHANNEL 1
#define LEFT_EYE_CHANNEL 2
#define RIGHT_EYE_CHANNEL 3

void updateLEDs_disconnected()
{
  // blink eyes
  if ((millis() / 500) % 2 == 0)
  {
    data.ch[LEFT_EYE_CHANNEL] = 255;
    data.ch[RIGHT_EYE_CHANNEL] = 255;
  }
  else
  {
    data.ch[LEFT_EYE_CHANNEL] = 0;
    data.ch[RIGHT_EYE_CHANNEL] = 0;
  }
}

void updateSerialDebugPrint()
{
  if (millis() - prevSerialPrint > serialPrintInterval)
  {
    Serial.print(DXL_MOTOR_1);
    Serial.print(" -> Position (calced): ");
    Serial.print((motor1_position * 360) / DYNARESOLUTION);
    Serial.print("°.");
    Serial.print(", velocity: ");
    Serial.println(motor1_velocity);

    Serial.print(DXL_MOTOR_2);
    Serial.print(" -> Position (calced): ");
    Serial.print((motor2_position * 360) / DYNARESOLUTION);
    Serial.print("°.");
    Serial.print(", velocity: ");
    Serial.println(motor2_velocity);

    prevSerialPrint = millis();
  }
}

void parseSBUS(bool serialPrint)
{
  if (sbus_rx.Read())
  {
    sbusPrevPacketTime = millis();
    if (connectionState == CONNECTION_LOST || connectionState == DISCONNECTED)
    {
      if (USE_SERIAL_DEBUG)
        Serial.println("Regained SBUS connection");
      connectionState = CONNECTED;
    }

    /* Grab the received data */
    data = sbus_rx.data();

    /* Display the received data */
    if (millis() - sbusPacketPrintPrevTime > SBUS_PACKET_PRINT_INTERVAL)
    {
      //*
      if (Serial && serialPrint)
      {
        for (int8_t i = 0; i < data.NUM_CH; i++)
        {
          Serial.print(data.ch[i]);
          Serial.print("\t");
        }
        Serial.println();
      }
      //*/

      sbusPacketPrintPrevTime = millis();
    }
  }

  // if SBUS lost, reset the channels
  if (millis() - sbusPrevPacketTime > SBUS_LOST_TIMEOUT)
  {
    if (connectionState != DISCONNECTED)
    {
      if (USE_SERIAL_DEBUG)
        Serial.println("Lost SBUS connection");

      connectionState = DISCONNECTED;
    }
  }
}
