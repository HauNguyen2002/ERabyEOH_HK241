# RFID and Biometric Fingerprint Authentication System with Era MQTT Server Integration

## Overview

This project focuses on implementing MIFARE 1k RFID operations, including card write, delete, and read functionalities. Additionally, it supports add, delete, and verify operations for the biometric fingerprint sensor. They are connected to the MQTT server provided by the Era platform to create remote interaction from users.

## Features

- **RFID Authentication:** Perform read, write, and delete operations on Mifare 1k RFID card contents.

- **Biometric Authentication:** Perform verify, add, delete, and update fingerprint operations with specified ID and permission level.

- **Era MQTT Integration:** Real-time data transmission and control via the Era IoT MQTT server.

- **Terminal Feedback:** Provides instructions and status updates during the authentication process.

## Hardware Requirements

1. RFID Module M5Stack MFRC522

2. Biometric Fingerprint Sensor M5Stack Fingerprint unit

3. ESP32-S3 (or compatible microcontroller with MQTT support)

4. Era IoT Server

## Software Requirements

1. Arduino IDE (or PlatformIO)

2. Era IoT MQTT Server credentials

## Acknowledgments

- [MFRC522_I2C Library by semaf](https://github.com/semaf/MFRC522_I2C_Library) for RFID module operations.

- [Adafruit Fingerprint Sensor Library by Gitshaoxiang](https://github.com/m5stack/M5-FPC1020A) for biometric fingerprint sensor.

- [Era by EOH](https://e-ra-iot-wiki.gitbook.io/documentation) for the IoT MQTT Server.
- [FreeRTOS](https://www.freertos.org/) for RTOS implementation.

