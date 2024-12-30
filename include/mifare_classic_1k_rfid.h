#ifndef MIFARE_CLASSIC_1K_RFID_H
#define MIFARE_CLASSIC_1K_RFID_H
#include <M5Unified.h>
#include "MFRC522_I2C.h"
#include <ERa.hpp>
#include <Widgets/ERaWidgets.hpp>
#include "ERa_widgets.h"
#include "chrono"
#include "ctime"

// USER DEFINES

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#define MAX_SECTOR 16
#define MAX_BLOCK_SIZE 16
#define BLOCK_PER_SECTOR 4
#define MFRC522_SDA_PIN 11
#define MFRC522_SCL_PIN 12
#define MFRC522_I2C_ADDRESS 0x28
#define MAX_INPUT_CONSECUTIVE_DATA_SIZE 752
#define MAX_CHAT_BOX_MSG_SIZE 100

enum MFRC522_MODE
{
    WAIT = -1,
    WRITE = 0,
    WRITE_AT = 1,
    DELETE = 2,
    READ = 3
};

static MFRC522 mfrc522(MFRC522_I2C_ADDRESS);

static MFRC522_MODE op_mode = WAIT;

static char consecutive_data[MAX_INPUT_CONSECUTIVE_DATA_SIZE];
static char block_data[MAX_BLOCK_SIZE];
static int8_t write_sector = 0;
static int8_t write_block_offset = 1;
static uint8_t write_start_byte = 0;

static const uint8_t sector_queue_len = 5;
static const uint8_t block_queue_len = 5;
static const uint8_t start_byte_queue_len = 5;

static QueueHandle_t sector_queue;
static QueueHandle_t block_queue;
static QueueHandle_t start_byte_queue;

static bool is_data_enter = false;

static bool is_cancel_operation = false;

static ERaString fromStr;


class Mifare_Classic_1K_MFRC522
{
public:
    MFRC522::MIFARE_Key key;
    uint8_t status;

    uint8_t first_block;
    uint8_t data_buffer[MAX_BLOCK_SIZE + 2];
    uint8_t byte_count;
    uint8_t trailer_buffer[MAX_BLOCK_SIZE + 2];
    uint8_t data_block_addr;
    uint8_t trailer_block_addr;
    bool is_new_sector;

    uint8_t block_eraser[MAX_BLOCK_SIZE];

    Mifare_Classic_1K_MFRC522();

private:
    bool isCardInserted();

    void resetAttributes();

    bool clearCardData();

    void appendCharData(uint8_t *data_buffer, size_t data_len, char *ret_data, size_t ret_data_size);

public:
    void dumpData();
    bool deleteCard();
    bool deleleBlock(uint8_t sector, uint8_t block_offset);
    bool writeBlock(uint8_t sector, uint8_t block_offset);
    bool writeConsecutive(const uint8_t start_sector, const uint8_t start_block_offet, uint8_t start_byte);
    bool readAllData();

    void processWriteConsecutive();
    void processWriteBlock();
    void processDelete();
    void processRead();
};

static Mifare_Classic_1K_MFRC522 card;

void MFRC522OperationCenter(void *parameters);

void init_mifare_1k_mfrc522();

#endif // MIFARE_CLASSIC_1K_RFID_H