# Jaha Koo - Toad

Code and PCB design for the transmitter and receiver + driver board to control the Toad robot. The wireless control is achieved ising an ExpressLRS wireless link, controlled and read using arduino-compatible microcontrollers. The Toad uses Dynamixel smart servo motors. The host board in the Toad contains a Dynamixel motor driver board, an open-source board designed by [Josue Alejandro Gutierrez](https://github.com/JosueAGtz/dynamixelInterface) and fabbed by me.

Produced for [Campo](https://campo.nu).

## Current files
* Code
  * Toad control from desktop (Chataigne) - [toad_v07_increment_CCs](./chataigne/toad_v07_increment_CCs.noisette)
  * Midi-host transmitter code
    * transmitter #1 (Wemos S2 mini) - [CRServoF_lib_tx_v02_add_midi.cpp](/arduino/platformio/WEMOS_ESP32S2_MIDI_ELRSTX/src/CRServoF_lib_tx_v02_add_midi.cpp)
    * transmitter #2  (Wemos S3 mini) - [CRServoF_lib_tx_v02_add_midi.cpp](./arduino/platformio/wemos_s3_mini/src/CRServoF_lib_tx_v02_add_midi.cpp)
  * Toad receiver code (Xiao esp32-c3): [toad_dynamixel_examples_moveServo_v08_update_to_arduino_core_3.cpp](./arduino/platformio/esp32c3_helloworld/esp32c3_helloworld/src/toad_dynamixel_examples_moveServo_v08_update_to_arduino_core_3.cpp)
* PCB - [toad_mainboard_v03](./kicad/toad%20mainboard%20v01/toad_mainboard_v03/toad_mainboard_v03.kicad_pro)

