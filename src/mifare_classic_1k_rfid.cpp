#include "mifare_classic_1k_rfid.h"

/*----------------------------------------------
---------------Class Definitions----------------
------------------------------------------------
*/
void init_mifare_1k_mfrc522()
{
    M5.begin();       // Initialize M5Stack
    M5.Power.begin(); // Initialize power module
    Wire.begin(MFRC522_SDA_PIN, MFRC522_SCL_PIN);
    mfrc522.PCD_Init();

    sector_queue = xQueueCreate(sector_queue_len, sizeof(uint8_t));
    block_queue = xQueueCreate(block_queue_len, sizeof(uint8_t));
    start_byte_queue = xQueueCreate(start_byte_queue_len, sizeof(uint8_t));

    xTaskCreatePinnedToCore(MFRC522OperationCenter, "MFRC522 Operation Center", 4096, NULL, 1, NULL, app_cpu);
}

Mifare_Classic_1K_MFRC522::Mifare_Classic_1K_MFRC522()
{
    for (uint8_t i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    byte_count = sizeof(data_buffer);
    is_new_sector = true;

    memset(block_eraser, 0, MAX_BLOCK_SIZE);
}

bool Mifare_Classic_1K_MFRC522::isCardInserted()
{
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("No card!");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait if no card is present
        return false;
    }
    xQueueSend(terminal_queue_EW, "Scanning... Please stay still!", portMAX_DELAY);
    return true;
}

void Mifare_Classic_1K_MFRC522::resetAttributes()
{
    status = 0;
    first_block = 0;
    data_block_addr = 0;
    trailer_block_addr = 0;
    memset(data_buffer, 0, byte_count);
    memset(trailer_buffer, 0, byte_count);
    is_new_sector = true;
}

bool Mifare_Classic_1K_MFRC522::clearCardData()
{
    uint8_t block_offset = 1;
    for (uint8_t sector = 0; sector < MAX_SECTOR;)
    {
        first_block = sector * BLOCK_PER_SECTOR;
        data_block_addr = first_block + block_offset;
        if (is_new_sector)
        {
            trailer_block_addr = first_block + BLOCK_PER_SECTOR - 1;
            status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailer_block_addr, &key, &mfrc522.uid);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.printf("Authenticate Trailerblock failed at sector: %d\n", sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                return false;
            }
            is_new_sector = false;
        }
        status = mfrc522.MIFARE_Write(data_block_addr, block_eraser, MAX_BLOCK_SIZE);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.printf("MIFARE_Write() blockEraser failed at sector:  %d\n", sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            return false;
        }

        block_offset++;
        if (block_offset >= BLOCK_PER_SECTOR - 1)
        {
            block_offset = 0;
            sector++;
            is_new_sector = true;
        }
    }
    return true;
}

void Mifare_Classic_1K_MFRC522::appendCharData(uint8_t *data_buffer, size_t data_len, char *ret_data, size_t ret_data_size)
{
    size_t current_len = strlen(ret_data); // Get the current length of ret_data
    if (current_len + data_len >= ret_data_size)
    {
        return; // Prevent overflow
    }

    // Append data from data_buffer to ret_data
    uint8_t append_idx = 0;
    for (size_t i = 0; i < data_len; i++)
    {
        if (data_buffer[i] != 0)
        {
            ret_data[current_len + append_idx] = (char)data_buffer[i];
            append_idx++;
        }
    }

    // Null-terminate the final result
    ret_data[current_len + data_len] = '\0';
}

void Mifare_Classic_1K_MFRC522::dumpData()
{
    resetAttributes();
    while (!isCardInserted())
        ;
    mfrc522.PICC_DumpToSerial(&mfrc522.uid);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

bool Mifare_Classic_1K_MFRC522::deleteCard()
{
    resetAttributes();
    while (!isCardInserted())
        ;
    bool clear_card_status = clearCardData();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return clear_card_status;
}

bool Mifare_Classic_1K_MFRC522::deleleBlock(uint8_t sector, uint8_t block_offset)
{
    resetAttributes();
    while (!isCardInserted())
        ;
    first_block = sector * BLOCK_PER_SECTOR;
    data_block_addr = first_block + block_offset;
    trailer_block_addr = first_block + BLOCK_PER_SECTOR - 1;
    status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailer_block_addr, &key, &mfrc522.uid);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.printf("Authenticate Trailerblock failed at sector: %d\n", sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }
    status = mfrc522.MIFARE_Write(data_block_addr, block_eraser, MAX_BLOCK_SIZE);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.printf("MIFARE_Write() blockEraser failed at sector:  %d\n", sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}

bool Mifare_Classic_1K_MFRC522::writeBlock(uint8_t sector, uint8_t block_offset)
{
    resetAttributes();
    while (!isCardInserted())
        ;
    first_block = sector * BLOCK_PER_SECTOR;
    data_block_addr = first_block + block_offset;
    trailer_block_addr = first_block + BLOCK_PER_SECTOR - 1;
    status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailer_block_addr, &key, &mfrc522.uid);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.printf("Authenticate Trailerblock failed at sector: %d\n", sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }
    memcpy(data_buffer, block_data, strlen(block_data));
    status = mfrc522.MIFARE_Write(data_block_addr, data_buffer, MAX_BLOCK_SIZE);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.printf("MIFARE_Write() data_buffer failed at sector:  %d\n", sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}

bool Mifare_Classic_1K_MFRC522::writeConsecutive(const uint8_t start_sector, const uint8_t start_block_offet, uint8_t start_byte)
{
    resetAttributes();

    while (!isCardInserted())

        ;
    if (!clearCardData())
    {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    uint16_t data_size = strlen(consecutive_data);
    uint8_t sector = start_sector;
    uint8_t block_offset = start_block_offet;
    uint8_t current_data[MAX_BLOCK_SIZE] = {0};
    uint16_t chunk_size = 0;

    first_block = sector * BLOCK_PER_SECTOR;
    for (uint16_t idx = 0; idx < data_size;)
    {
        if (is_new_sector)
        {
            first_block = sector * BLOCK_PER_SECTOR;
            trailer_block_addr = first_block + BLOCK_PER_SECTOR - 1;
            status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailer_block_addr, &key, &mfrc522.uid);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.printf("Authenticate Trailerblock failed at sector: %d\n", sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                mfrc522.PICC_HaltA();
                mfrc522.PCD_StopCrypto1();
                return false;
            }
            is_new_sector = false;
        }
        data_block_addr = first_block + block_offset;

        memset(current_data, 0, MAX_BLOCK_SIZE);
        memset(data_buffer, 0, byte_count);
        chunk_size = min(MAX_BLOCK_SIZE - start_byte, data_size - idx);
        memcpy(current_data, &consecutive_data[idx], chunk_size);
        memcpy(&data_buffer[start_byte], current_data, chunk_size);
        status = mfrc522.MIFARE_Write(data_block_addr, data_buffer, MAX_BLOCK_SIZE);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.printf("MIFARE_Write() data_buffer failed at sector:  %d\n", sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return false;
        }
        idx += MAX_BLOCK_SIZE - start_byte;
        start_byte = 0;
        block_offset++;
        if (block_offset >= BLOCK_PER_SECTOR - 1)
        {
            block_offset = 0;
            sector++;
            is_new_sector = true;
        }
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}
bool Mifare_Classic_1K_MFRC522::readAllData()
{
    resetAttributes();
    while (!isCardInserted())
        ;

    memset(consecutive_data, 0, MAX_INPUT_CONSECUTIVE_DATA_SIZE);
    uint8_t block_offset = 1;
    for (uint8_t sector = 0; sector < MAX_SECTOR;)
    {
        if (is_new_sector)
        {
            first_block = sector * BLOCK_PER_SECTOR;
            trailer_block_addr = first_block + BLOCK_PER_SECTOR - 1;
            status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailer_block_addr, &key, &mfrc522.uid);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.printf("Authenticate Trailerblock failed at sector: %d\n", sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                mfrc522.PICC_HaltA();
                mfrc522.PCD_StopCrypto1();
                return false;
            }
            is_new_sector = false;
        }
        data_block_addr = first_block + block_offset;
        memset(data_buffer, 0, byte_count);
        status = mfrc522.MIFARE_Read(data_block_addr, data_buffer, &byte_count);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.printf("MIFARE_Read() data_buffer failed at sector: %d\n", sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return false;
        }
        appendCharData(data_buffer, MAX_BLOCK_SIZE, consecutive_data, MAX_INPUT_CONSECUTIVE_DATA_SIZE);
        block_offset++;
        if (block_offset >= BLOCK_PER_SECTOR - 1)
        {
            block_offset = 0;
            sector++;
            is_new_sector = true;
        }
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}
void Mifare_Classic_1K_MFRC522::processWriteConsecutive()
{
    xQueueSend(terminal_queue_EW, "Current MFRC522 mode: Write consecutive data", portMAX_DELAY);

    write_sector = 0;
    write_block_offset = 0;
    write_start_byte = 0;
    memset(consecutive_data, 0, MAX_INPUT_CONSECUTIVE_DATA_SIZE);

    xQueueSend(terminal_queue_EW, "Enter start sector:", 0);
    xQueueReceive(sector_queue, &write_sector, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Enter start block:", 0);
    xQueueReceive(block_queue, &write_block_offset, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Enter start byte:", 0);
    xQueueReceive(start_byte_queue, &write_start_byte, portMAX_DELAY);
    if (write_sector < 0 || write_block_offset < 0)
    {
        xQueueSend(terminal_queue_EW, "Invalid sector of block offset", 0);
        return;
    }

    if (write_sector == 0 && write_block_offset == 0)
        write_block_offset = 1;
    uint16_t max_data_size = (MAX_SECTOR - write_sector) * MAX_BLOCK_SIZE * (BLOCK_PER_SECTOR - 1) - 16 - write_start_byte - MAX_BLOCK_SIZE * (write_block_offset - 1);

    char data_msg[50];
    sprintf(data_msg, "Enter data (maximum %d characters):", max_data_size);

    xQueueSend(terminal_queue_EW, data_msg, 0);
    while (!is_data_enter)
        vTaskDelay(100 / portTICK_PERIOD_MS);
    is_data_enter = false;
    if (strlen(consecutive_data) > max_data_size)
    {
        memset(data_msg, 0, 50);
        sprintf(data_msg, "The input is too long (%d characters)", strlen(consecutive_data));
        xQueueSend(terminal_queue_EW, data_msg, 0);
        return;
    }
    xQueueSend(terminal_queue_EW, "Insert your card to start writing...", 0);

    if (writeConsecutive(write_sector, write_block_offset, write_start_byte))
        xQueueSend(terminal_queue_EW, "Card written successfully", 0);
    else
        xQueueSend(terminal_queue_EW, "Card written failed", 0);
}
void Mifare_Classic_1K_MFRC522::processWriteBlock()
{
    xQueueSend(terminal_queue_EW, "Current MFRC522 mode: Write block data", 0);
    write_sector = 0;
    write_block_offset = 0;
    memset(block_data, 0, MAX_BLOCK_SIZE);
    xQueueSend(terminal_queue_EW, "Enter sector:", 0);
    xQueueReceive(sector_queue, &write_sector, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Enter block:", 0);
    xQueueReceive(block_queue, &write_block_offset, portMAX_DELAY);
    if (write_sector < 0 || write_block_offset < 0)
    {
        xQueueSend(terminal_queue_EW, "Invalid sector of block offset", 0);
        return;
    }
    if (write_sector == 0 && write_block_offset == 0)
        write_block_offset = 1;
    char sector_block_msg[70];
    sprintf(sector_block_msg, "You are writing to sector %d, block %d\nEnter data:", write_sector, write_block_offset);
    xQueueSend(terminal_queue_EW, sector_block_msg, 0);
    while (!is_data_enter)
        vTaskDelay(100 / portTICK_PERIOD_MS);
    is_data_enter = false;

    uint8_t input_block_data_length = strlen(block_data);
    uint8_t limit = 16;
    if (input_block_data_length > limit)
    {
        Serial.println("------------------------------------------------------------------------");
        memset(sector_block_msg, 0, 70);
        sprintf(sector_block_msg, "The input is too long (%d characters)", strlen(block_data));
        xQueueSend(terminal_queue_EW, sector_block_msg, 0);
        return;
    }
    xQueueSend(terminal_queue_EW, "Insert your card to start writing...", 0);
    if (writeBlock(write_sector, write_block_offset))
        xQueueSend(terminal_queue_EW, "Card written successfully", 0);
    else
        xQueueSend(terminal_queue_EW, "Card written failed", 0);
}
void Mifare_Classic_1K_MFRC522::processDelete()
{
    xQueueSend(terminal_queue_EW, "Current MFRC522 mode: Delete data", 0);
    write_sector = -1;
    write_block_offset = -1;
    xQueueSend(terminal_queue_EW, "Enter sector:", 0);
    xQueueReceive(sector_queue, &write_sector, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Enter block:", 0);
    xQueueReceive(block_queue, &write_block_offset, portMAX_DELAY);

    if (write_sector == -1 && write_block_offset == -1)
    {
        xQueueSend(terminal_queue_EW, "Insert your card to start deleting...", 0);

        if (deleteCard())
        {
            xQueueSend(terminal_queue_EW, "All data deleted successfully", 0);
        }
    }
    else if (write_sector >= 0 && write_block_offset >= 0)
    {
        xQueueSend(terminal_queue_EW, "Insert your card to start deleting...", 0);

        if (deleleBlock(write_sector, write_block_offset))
        {
            char delete_msg[50];
            sprintf(delete_msg, "Data at sector: %d, block: %d deleted successfully", write_sector, write_block_offset);
            xQueueSend(terminal_queue_EW, delete_msg, 0);
        }
    }
    else
        xQueueSend(terminal_queue_EW, "All data deleted failed", 0);
}
void Mifare_Classic_1K_MFRC522::processRead()
{
    xQueueSend(terminal_queue_EW, "Current MFRC522 mode: Read data", 0);
    xQueueSend(terminal_queue_EW, "Insert your card to start reading...", 0);
    if (readAllData())
    {
        char read_msg[MAX_CHAT_BOX_MSG_SIZE];
        uint16_t data_size = strlen(consecutive_data);
        for (uint16_t idx = 0; idx < data_size; idx += MAX_CHAT_BOX_MSG_SIZE)
        {
            memset(read_msg, 0, MAX_CHAT_BOX_MSG_SIZE);
            size_t chunk_size = (idx + MAX_CHAT_BOX_MSG_SIZE <= data_size) ? MAX_CHAT_BOX_MSG_SIZE : data_size - idx;
            memcpy(read_msg, &consecutive_data[idx], chunk_size);
            xQueueSend(terminal_queue_EW, read_msg, 0);
        }
        memset(read_msg, 0, MAX_CHAT_BOX_MSG_SIZE);
        sprintf(read_msg, "%d bytes of data is read", data_size);
        xQueueSend(terminal_queue_EW, read_msg, 0);
    }
    else
        xQueueSend(terminal_queue_EW, "Card read failed", 0);
}

/*----------------------------------------------
---------------------RTOS-----------------------
------------------------------------------------
*/

void MFRC522OperationCenter(void *parameters)
{
    while (1)
    {

        while (uxQueueMessagesWaiting(sector_queue) > 0)
        {
            void *dummy_item;
            xQueueReceive(sector_queue, &dummy_item, 0);
        }
        while (uxQueueMessagesWaiting(block_queue) > 0)
        {
            void *dummy_item;
            xQueueReceive(block_queue, &dummy_item, 0);
        }
        while (uxQueueMessagesWaiting(start_byte_queue) > 0)
        {
            void *dummy_item;
            xQueueReceive(start_byte_queue, &dummy_item, 0);
        }
        switch (op_mode)
        {
        case WAIT:
            break;
        case WRITE:
            card.processWriteConsecutive();
            op_mode = WAIT;
            break;
        case WRITE_AT:
            card.processWriteBlock();
            op_mode = WAIT;
            break;
        case DELETE:
            card.processDelete();
            op_mode = WAIT;
            break;
        case READ:
            card.processRead();
            op_mode = WAIT;
            break;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*----------------------------------------------
-----------------ERa Callbacks------------------
------------------------------------------------
*/

ERA_WRITE(V42)
{
    auto now = std::chrono::system_clock::now();
    // Convert to time_t for seconds-based time
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    Serial.println(std::ctime(&time_t_now));
    uint8_t mode = param.getInt();
    switch (mode)
    {
    case 0:
        op_mode = WRITE;
        break;
    case 1:
        op_mode = WRITE_AT;
        break;
    case 2:
        op_mode = DELETE;
        break;
    case 3:
        op_mode = READ;
        break;
    default:
        op_mode = WAIT;
        break;
    }
}

ERA_WRITE(V45)
{
    uint8_t input_sector = param.getInt();
    xQueueSend(sector_queue, &input_sector, portMAX_DELAY);
}

ERA_WRITE(V46)
{
    uint8_t input_block = param.getInt();
    xQueueSend(block_queue, &input_block, portMAX_DELAY);
}

ERA_WRITE(V47)
{
    uint8_t input_start_byte = param.getInt();
    xQueueSend(start_byte_queue, &input_start_byte, portMAX_DELAY);
}

void terminalCallback_EW()
{
    switch (op_mode)
    {
    case WRITE:
        memcpy(consecutive_data, fromStr_EW, strlen(fromStr_EW));
        break;
    case WRITE_AT:
        memcpy(block_data, fromStr_EW, strlen(fromStr_EW));
        break;
    }
    is_data_enter = true;
}
