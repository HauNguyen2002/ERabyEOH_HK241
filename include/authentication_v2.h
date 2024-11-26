#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <Arduino.h>
#include <freeRTOS/FreeRTOS.h>
#include <M5Unified.h>
#include "MFRC522_I2C.h"
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

// GLOBALS
// Create MFRC522 instance with I2C address 0x28, comment it out if using SPI/UART
static MFRC522 mfrc522(0x28);

MFRC522::MIFARE_Key key;

static byte userAccount[NUM_INFO_TYPE][MAX_BLOCK_BUFFER];
static char newPassword[MAX_BLOCK_BUFFER];
static byte status;

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

void clearAllDataBlock();

void writeUserInfoToRFID(WriteMode write_mode);

int validateCredentialAccount();

void doCLI(void *parameters);

void taskCenter(void *parameters);

bool validateUser();

void addNewUser();

void updateUserPassword();
void deleteUser();

#endif // AUTHENTICATION_H