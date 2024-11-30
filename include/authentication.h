#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <Arduino.h>
#include <freeRTOS/FreeRTOS.h>
#include <M5Unified.h>
#include "MFRC522_I2C.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "credential_store.h"

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// USER DEFINES
#define MAX_BLOCK_BUFFER 16
#define MAX_MSG_BUFFER 100
#define LED_ON_BOARD 48
#define NUM_INFO_TYPE 3
#define BLOCK_PER_SECTOR 4
#define MAX_SECTOR 16
#define SDA_PIN 11 // Configurable, see typical pin layout above
#define SCL_PIN 12 // Configurable, see typical pin layout above

// Define the DHT pins and type
#define DHTPIN 5     // Pin where the DHT11 is connected
#define DHTTYPE DHT11  // Define the type of sensor


// Define HC-SR04 pins
#define TRIG_PIN 6   // Pin where the TRIG is connected
#define ECHO_PIN 7   // Pin where the ECHO is connected

// GLOBALS
static MFRC522 mfrc522(0x28);

static MFRC522::MIFARE_Key key;

static byte status;

static const uint8_t username_queue_len = 5;
static const uint8_t password_queue_len = 5;
static const uint8_t terminal_queue_len = 5;

static QueueHandle_t username_queue;
static QueueHandle_t password_queue;
static QueueHandle_t terminal_queue;

static TaskHandle_t addHandle;
static TaskHandle_t updatePassHandle;


enum OpMode
{
    WAIT,
    DUMP,
    VALIDATE,
    ADD,
    CHANGE_PASSWORD,
    DELETE
};

enum InfoType
{
    USERNAME,
    PASSWORD,
    ID
};

enum WriteMode
{
    ALL,
    ONLY_PASSWORD
};

// CLASSES

typedef struct RFID_Device
{
    byte firstBlock;
    byte dataBuffer[MAX_BLOCK_BUFFER + 2];
    byte byteCount = sizeof(dataBuffer);
    byte trailerBuffer[MAX_BLOCK_BUFFER + 2];
    byte dataBlockAddr;
    byte trailerBlockAddr;
    byte blockOffset = 1;
    bool isNewSector = true;
} RFID_Device;

typedef struct UserInfo
{
    byte id;
    char username[MAX_BLOCK_BUFFER + 1] = "";
    char password[MAX_BLOCK_BUFFER + 1] = "";
    byte location[NUM_INFO_TYPE][2];

} UserInfo;

typedef struct Door
{
    bool isOpened = false;
    bool isValidated = false;
} Door;

// Forward Declarations
bool checkInvertedNibblesMatch(byte *trailerBuffer);

void printTextData();

void dumpData(void *parameters);

bool clearAllDataBlock();

bool writeUserInfoToRFID(WriteMode write_mode);

int validateCredentialAccount();

void doCLI(void *parameters);

void DoorAuthenticationTaskCenter(void *parameters);

bool validateUser();

void addNewUser();

void updateUserPassword();
void deleteUser();

//------------------------------------------------------------------------

void TaskTemperatureHumidity(void *pvParameters);

#endif // AUTHENTICATION_H