#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <Arduino.h>
#include <M5Unified.h>
#include "MFRC522_I2C.h"
#include "credential_store.h"
#include <ERa.hpp>
#include <Widgets/ERaWidgets.hpp>

// USER DEFINES
#define MAX_BLOCK_BUFFER 16
#define MAX_MSG_BUFFER 100
#define LED_ON_BOARD 48
#define NUM_INFO_TYPE 3
#define BLOCK_PER_SECTOR 4
#define MAX_SECTOR 16

#define MARK_SECTOR 15
#define STATE_SECTOR 14

#define SDA_PIN 11 // Configurable, see typical pin layout above
#define SCL_PIN 12 // Configurable, see typical pin layout above

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

static const char mark_credential_key[MAX_BLOCK_BUFFER + 1] = "%#!&%NNWEWER";

static const char writing_key='w';
static const char done_writing_key='d';

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

enum WriteState{
    WRITING,
    DONE,
    UNKNOWN
};

enum OpStatus
{
    OP_STATUS_OK,
    OP_STATUS_FAILED,
    OP_STATUS_CANCELLED
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

void init_door_authentication();
bool isOpCancelled();

bool checkInvertedNibblesMatch(byte *trailerBuffer);

void printTextData();

void dumpData(void *parameters);

bool clearAllDataBlock();

OpStatus writeUserInfoToRFID(WriteMode write_mode);

int validateCredentialAccount();

void doCLI(void *parameters);

void DoorAuthenticationTaskCenter(void *parameters);

bool validateUser();

void addNewUser();

void updateUserPassword();
void deleteUser();

#endif // AUTHENTICATION_H