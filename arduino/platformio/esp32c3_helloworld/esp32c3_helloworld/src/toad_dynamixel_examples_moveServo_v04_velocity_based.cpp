#include <Arduino.h>
#include <Wire.h>
#include <Dynamixel.h>
#include <SoftwareSerial.h>
#include "sbus.h"
#include <HardwareSerial.h>

#define DXL_BAUDRATE 57600
#define DXL_PROTOCOL 2
#define DXL_DIR_PIN 7 // 37

#define AX12RESOLUTION 1023
#define DYNARESOLUTION 4096
#define MAXPOSITION 4000
#define MINPOSITION 100
#define OFFSET 10

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
bool sbusLost = false;

#define SBUS_LOST_TIMEOUT 100

void parseSBUS(bool serialPrint);

void setup()
{

  Serial.begin(115200); // initialize serial communication at 115200 bps, Enable CDC on Boot

  softSerial.begin(DXL_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, 5, 6, false, 95, 11);
  Dynamixel.begin(&softSerial, DXL_BAUDRATE, DXL_DIR_PIN, DXL_PROTOCOL);

  /* Begin the SBUS communication */
  sbus_rx.Begin();

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
}

void loop()
{
  parseSBUS(false);

  Dynamixel.setGoalVelocity(DXL_MOTOR_1, map(data.ch[0], 172, 1811, -255, 255));
  Dynamixel.setGoalVelocity(DXL_MOTOR_2, map(data.ch[1], 172, 1811, -255, 255));
  // Dynamixel.move(DXL_MOTOR_1, map(data.ch[0], 172, 1811, MINPOSITION, MAXPOSITION));
  // Dynamixel.move(DXL_MOTOR_2, map(data.ch[1], 172, 1811, MINPOSITION, MAXPOSITION));

  sbusPrevPacketTime = millis();

  motor1_position = Dynamixel.readPosition(DXL_MOTOR_1);
  motor1_velocity = Dynamixel.readSpeed(DXL_MOTOR_1);
  motor1_model = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_MOTOR_1)));

  motor2_position = Dynamixel.readPosition(DXL_MOTOR_2);
  motor2_velocity = Dynamixel.readSpeed(DXL_MOTOR_2);
  motor2_model = Dynamixel.mapMotorModel((Dynamixel.readModel(DXL_MOTOR_2)));

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

  // if ((Dynamixel.readPosition(DXL_MOTOR_1) > MAXPOSITION - OFFSET) && (Dynamixel.readPosition(DXL_MOTOR_1) < MAXPOSITION + OFFSET))
  // {
  //   Serial.println("move to MINPOSITION");
  //   Dynamixel.move(DXL_MOTOR_1, MINPOSITION);
  // }

  // if ((Dynamixel.readPosition(DXL_MOTOR_1) > MINPOSITION - OFFSET) && (Dynamixel.readPosition(DXL_MOTOR_1) < MINPOSITION + OFFSET))
  // {
  //   Serial.println("move to MAXPOSITION");
  //   Dynamixel.move(DXL_MOTOR_1, MAXPOSITION);
  // }

  delay(1000/15);
}

void parseSBUS(bool serialPrint)
{
  if (sbus_rx.Read())
  {
    sbusPrevPacketTime = millis();
    if (sbusLost)
    {
      Serial.println("SBUS connection restored");
      sbusLost = false;
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
    if (!sbusLost)
    {
      Serial.println("SBUS connection lost");

      sbusLost = true;
    }
  }

}


