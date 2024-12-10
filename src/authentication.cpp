/*
Choose multiple: Ctrl+D
Reindent: Shift+Alt+F
*/

#include "authentication.h"

static enum OpMode mode = VALIDATE;

static Door door;
static UserInfo userBuffer;

static ERaString fromStr;
static ERaWidgetTerminalBox terminal(fromStr, V43, V44);

static bool cancel_flag = false;

// ERa callbacks

/*
Virtual pins list:
- V41: Blink onboard led
- V42: RC522 mode, set value of ADD=1, FORGET_PASSWORD=2, SCAN=3
- V43: ERa Terminal send, set value to 0
- V44: ERa Terminal receive
- V45: Username
- V46: Password
*/

ERA_WRITE(V41)
{
    byte value = param.getInt();
    digitalWrite(LED_ON_BOARD, value);
}

ERA_WRITE(V42)
{
    if (!door.isValidated)
    {
        xQueueSend(terminal_queue, "Scan your card to validate....", portMAX_DELAY);
        return;
    }

    byte server_mode = param.getInt();

    switch (server_mode)
    {
    case 1:
        mode = ADD;
        break;
    case 2:
        mode = CHANGE_PASSWORD;
        break;
    case 3:
        mode = DELETE;
        break;
    case 4:
        cancel_flag = true;
    }
}
ERA_WRITE(V45)
{
    if (param.getStringLength() > MAX_BLOCK_BUFFER)
    {
        xQueueSend(terminal_queue, "Username is too long", portMAX_DELAY);
    }
    else
    {
        xQueueSend(username_queue, param.getString(), portMAX_DELAY);
    }
}
ERA_WRITE(V46)
{
    if (param.getStringLength() > MAX_BLOCK_BUFFER)
    {
        xQueueSend(terminal_queue, "Password is too long", portMAX_DELAY);
    }
    else
    {
        xQueueSend(password_queue, param.getString(), portMAX_DELAY);
    }
}
ERA_WRITE(V47)
{
    switch (param.getInt())
    {
    case 0:
        door.isOpened = false;

        mode = VALIDATE;
        break;
    case 1:
        if (door.isValidated)
            door.isOpened = true;
        else
        {
            ERa.virtualWrite(V47, 0);
            xQueueSend(terminal_queue, "Scan your card to validate....", portMAX_DELAY);
        }
        break;
    }
}

//-----------------------------------------------------

// Tasks
/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
--------------------------------Door authentication----------------------------
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void printTextData()
{
    RFID_Device rfidRead;
    char blockDataStr[MAX_BLOCK_BUFFER + 1];

    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
        Serial.println("No card");
    }

    for (byte sector = 0; sector < MAX_SECTOR;)
    {
        rfidRead.firstBlock = sector * BLOCK_PER_SECTOR;

        if (rfidRead.isNewSector)
        {
            rfidRead.trailerBlockAddr = rfidRead.firstBlock + BLOCK_PER_SECTOR - 1;
            status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidRead.trailerBlockAddr, &key, &mfrc522.uid);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
                Serial.println(sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
                continue;
            }
            status = mfrc522.MIFARE_Read(rfidRead.trailerBlockAddr, rfidRead.trailerBuffer, &rfidRead.byteCount);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
                Serial.println(sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
                continue;
            }
            rfidRead.isNewSector = false;
        }
        rfidRead.dataBlockAddr = rfidRead.firstBlock + rfidRead.blockOffset;
        memset(rfidRead.dataBuffer, 0, rfidRead.byteCount);
        status = mfrc522.MIFARE_Read(rfidRead.dataBlockAddr, rfidRead.dataBuffer, &rfidRead.byteCount);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
            Serial.println(sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
            continue;
        }
        if (rfidRead.dataBuffer[0] == 0x00 && rfidRead.blockOffset == 0)
            return;

        if (rfidRead.dataBuffer[0] != 0x00)
        {
            for (byte index = 0; index < 16; index++)
            {
                if (rfidRead.dataBuffer[index] < 0x10)
                    Serial.print(F(" 0"));
                else
                    Serial.print(F(" "));
                Serial.print(rfidRead.dataBuffer[index], HEX);
                if ((index % 4) == 3)
                {
                    Serial.print(F(" "));
                }
            }
            Serial.println();
            memset(blockDataStr, 0, MAX_BLOCK_BUFFER + 1);
            for (byte index = 0; index < MAX_BLOCK_BUFFER; index++)
            {
                blockDataStr[index] = rfidRead.dataBuffer[index];
            }
            blockDataStr[MAX_BLOCK_BUFFER] = '\0';
            Serial.print("Current blockDataStr: ");
            Serial.println(blockDataStr);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        if (checkInvertedNibblesMatch(rfidRead.trailerBuffer))
        {
            Serial.print(F(" Inverted access bits did not match! "));
        }

        rfidRead.blockOffset++;
        if (rfidRead.blockOffset >= BLOCK_PER_SECTOR - 1)
        {
            rfidRead.blockOffset = 0;
            sector++;
            rfidRead.isNewSector = true;
        }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    terminal.println("Read successfully!");

    rfidRead.blockOffset = 1;
}
void dumpData(void *parameters)
{
    while (1)
    {
        if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
        {
            Serial.println("No card");
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
            continue;
        }
        mfrc522.PICC_DumpToSerial(&mfrc522.uid);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
bool write_credential_mark()
{
    RFID_Device mark_rfid;
    byte mark_sector = MARK_SECTOR;

    mark_rfid.blockOffset = 2;
    mark_rfid.firstBlock = mark_sector * BLOCK_PER_SECTOR;
    mark_rfid.dataBlockAddr = mark_rfid.firstBlock + mark_rfid.blockOffset;
    mark_rfid.trailerBlockAddr = mark_rfid.firstBlock + BLOCK_PER_SECTOR - 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, mark_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.printf("Authentication failed for sector %d\n", mark_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    memset(mark_rfid.dataBuffer, 0, mark_rfid.byteCount);
    memcpy(mark_rfid.dataBuffer, mark_credential_key, strlen(mark_credential_key));
    status = mfrc522.MIFARE_Write(mark_rfid.dataBlockAddr, mark_rfid.dataBuffer, MAX_BLOCK_BUFFER);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Writing to block failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    return true;
}

bool set_write_state(WriteState state)
{
    RFID_Device state_rfid;
    byte mark_sector = STATE_SECTOR;

    state_rfid.blockOffset = 2;
    state_rfid.firstBlock = mark_sector * BLOCK_PER_SECTOR;
    state_rfid.dataBlockAddr = state_rfid.firstBlock + state_rfid.blockOffset;
    state_rfid.trailerBlockAddr = state_rfid.firstBlock + BLOCK_PER_SECTOR - 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, state_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.printf("Authentication failed for sector %d\n", mark_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    memset(state_rfid.dataBuffer, 0, state_rfid.byteCount);
    if (state == WriteState::WRITING)
        state_rfid.dataBuffer[0] = writing_key;
    else if (state == WriteState::DONE)
        state_rfid.dataBuffer[0] = done_writing_key;

    status = mfrc522.MIFARE_Write(state_rfid.dataBlockAddr, state_rfid.dataBuffer, MAX_BLOCK_BUFFER);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Writing to block failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    return true;
}

bool check_credential_mark()
{
    RFID_Device read_mark_rfid;
    byte read_mark_sector = MARK_SECTOR;
    read_mark_rfid.blockOffset = 2;
    read_mark_rfid.firstBlock = read_mark_sector * BLOCK_PER_SECTOR;
    read_mark_rfid.dataBlockAddr = read_mark_rfid.firstBlock + read_mark_rfid.blockOffset;
    read_mark_rfid.trailerBlockAddr = read_mark_rfid.firstBlock + BLOCK_PER_SECTOR - 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, read_mark_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.printf("Authentication failed for sector %d\n", read_mark_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }
    memset(read_mark_rfid.dataBuffer, 0, read_mark_rfid.byteCount);
    status = mfrc522.MIFARE_Read(read_mark_rfid.dataBlockAddr, read_mark_rfid.dataBuffer, &read_mark_rfid.byteCount);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
        Serial.println(read_mark_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    if (strncmp((char *)read_mark_rfid.dataBuffer, mark_credential_key, strlen(mark_credential_key)) != 0)
    {
        return false;
    }

    return true;
}

WriteState check_write_state()
{
    RFID_Device read_state_rfid;
    byte read_state_sector = STATE_SECTOR;
    read_state_rfid.blockOffset = 2;
    read_state_rfid.firstBlock = read_state_sector * BLOCK_PER_SECTOR;
    read_state_rfid.dataBlockAddr = read_state_rfid.firstBlock + read_state_rfid.blockOffset;
    read_state_rfid.trailerBlockAddr = read_state_rfid.firstBlock + BLOCK_PER_SECTOR - 1;

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, read_state_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.printf("Authentication failed for sector %d\n", read_state_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return UNKNOWN;
    }
    memset(read_state_rfid.dataBuffer, 0, read_state_rfid.byteCount);
    status = mfrc522.MIFARE_Read(read_state_rfid.dataBlockAddr, read_state_rfid.dataBuffer, &read_state_rfid.byteCount);
    if (status != mfrc522.STATUS_OK)
    {
        Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
        Serial.println(read_state_sector);
        Serial.println(mfrc522.GetStatusCodeName(status));
        return UNKNOWN;
    }

    if (read_state_rfid.dataBuffer[0] == writing_key)
    {
        return WRITING;
    }
    if (read_state_rfid.dataBuffer[0] == done_writing_key)
    {
        return DONE;
    }
    return UNKNOWN;
}

bool clearAllDataBlock()
{
    RFID_Device rfidClear;
    byte blockEraser[MAX_BLOCK_BUFFER];

    memset(blockEraser, 0, MAX_BLOCK_BUFFER);

    for (byte sector = 0; sector < MAX_SECTOR;)
    {
        rfidClear.firstBlock = sector * BLOCK_PER_SECTOR;

        if (rfidClear.isNewSector)
        {
            rfidClear.trailerBlockAddr = rfidClear.firstBlock + BLOCK_PER_SECTOR - 1;
            status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidClear.trailerBlockAddr, &key, &mfrc522.uid);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.print(F("Authenticate Trailerblock failed at sector: "));
                Serial.println(sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                return false;
            }
            status = mfrc522.MIFARE_Read(rfidClear.trailerBlockAddr, rfidClear.trailerBuffer, &rfidClear.byteCount);
            if (status != mfrc522.STATUS_OK)
            {
                Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
                Serial.println(sector);
                Serial.println(mfrc522.GetStatusCodeName(status));
                return false;
            }
            rfidClear.isNewSector = false;
        }
        rfidClear.dataBlockAddr = rfidClear.firstBlock + rfidClear.blockOffset;
        status = mfrc522.MIFARE_Write(rfidClear.dataBlockAddr, blockEraser, MAX_BLOCK_BUFFER);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.print(F("MIFARE_Write() blockEraser failed at sector: "));
            Serial.println(sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            return false;
        }
        if (checkInvertedNibblesMatch(rfidClear.trailerBuffer))
        {
            Serial.print(F(" Inverted access bits did not match! "));
        }

        rfidClear.blockOffset++;
        if (rfidClear.blockOffset >= BLOCK_PER_SECTOR - 1)
        {
            rfidClear.blockOffset = 0;
            sector++;
            rfidClear.isNewSector = true;
        }
    }
    return true;
}

bool isCardInserted()
{
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
        return false;
    }
    xQueueSend(terminal_queue, "Scanning... Please stay still!", portMAX_DELAY);
    return true;
}
OpStatus writeUserInfoToRFID(WriteMode write_mode)
{
    RFID_Device w_rfid;
    byte w_sector;

    while (!isCardInserted())
    {
        if (isOpCancelled())
            return OpStatus::OP_STATUS_CANCELLED;
    }
    switch (write_mode)
    {
    case ALL:
        if (!clearAllDataBlock())
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        if (!set_write_state(WriteState::WRITING))
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        
        w_sector = 0;
        // Loop through attributes and write to blocks
        for (byte infoIdx = USERNAME; infoIdx <= ID; infoIdx++)
        {
            w_rfid.firstBlock = w_sector * BLOCK_PER_SECTOR;
            w_rfid.dataBlockAddr = w_rfid.firstBlock + w_rfid.blockOffset;

            // Authenticate with the sector's trailer block
            if (w_rfid.isNewSector)
            {
                w_rfid.trailerBlockAddr = w_rfid.firstBlock + BLOCK_PER_SECTOR - 1;
                status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, w_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
                if (status != MFRC522::STATUS_OK)
                {
                    Serial.print("Authentication failed for sector ");
                    Serial.println(w_sector);
                    Serial.println(mfrc522.GetStatusCodeName(status));
                    mfrc522.PICC_HaltA();
                    mfrc522.PCD_StopCrypto1();
                    return OpStatus::OP_STATUS_FAILED;
                }
                w_rfid.isNewSector = false;
            }

            memset(w_rfid.dataBuffer, 0, w_rfid.byteCount);
            switch (infoIdx)
            {
            case USERNAME:
                memcpy(w_rfid.dataBuffer, userBuffer.username, strlen(userBuffer.username));
                status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
                break;
            case PASSWORD:
                memcpy(w_rfid.dataBuffer, userBuffer.password, strlen(userBuffer.password));
                status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
                break;
            case ID:
                w_rfid.dataBuffer[0] = userBuffer.id;
                status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
                break;
            }
            if (status != MFRC522::STATUS_OK)
            {
                Serial.print("Writing to block failed: ");
                Serial.println(mfrc522.GetStatusCodeName(status));
                mfrc522.PICC_HaltA();
                mfrc522.PCD_StopCrypto1();
                return OpStatus::OP_STATUS_FAILED;
            }

            // Advance to next block
            w_rfid.blockOffset++;
            if (w_rfid.blockOffset >= BLOCK_PER_SECTOR - 1)
            {
                w_rfid.blockOffset = 0; // Reset block offset
                w_sector++;             // Move to the next sector
                w_rfid.isNewSector = true;
            }
        }
        if (!set_write_state(WriteState::DONE))
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        if (!write_credential_mark())
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        break;
    case ONLY_PASSWORD:
        if (!set_write_state(WriteState::WRITING))
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        // Loop through attributes and write to blocks
        w_sector = 0;
        w_rfid.blockOffset = 2;
        w_rfid.firstBlock = w_sector * BLOCK_PER_SECTOR;
        w_rfid.dataBlockAddr = w_rfid.firstBlock + w_rfid.blockOffset;
        w_rfid.trailerBlockAddr = w_rfid.firstBlock + BLOCK_PER_SECTOR - 1;
        // Authenticate with the sector's trailer block
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, w_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK)
        {
            Serial.printf("Authentication failed for sector %d\n", w_sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }

        memset(w_rfid.dataBuffer, 0, w_rfid.byteCount);
        memcpy(w_rfid.dataBuffer, userBuffer.password, strlen(userBuffer.password));
        status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print("Writing to block failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        if (!set_write_state(WriteState::DONE))
        {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            return OpStatus::OP_STATUS_FAILED;
        }
        break;
    }

    // Halt and stop the reader after completing the writing process
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return OpStatus::OP_STATUS_OK;
}
int validateCredentialAccount()
{
    RFID_Device rfidRead;
    byte sector, blockOffset;
    bool isUsername = true;
    bool isPassword = false;

    userBuffer.location[USERNAME][0] = 0;
    userBuffer.location[USERNAME][1] = 1;
    userBuffer.location[PASSWORD][0] = 0;
    userBuffer.location[PASSWORD][1] = 2;
    userBuffer.location[ID][0] = 1;
    userBuffer.location[ID][1] = 0;

    while (!isCardInserted())
    {
        vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
    }

    if (!check_credential_mark())
    {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return -2;
    }

    for (auto loc : userBuffer.location)
    {
        sector = loc[0];
        blockOffset = loc[1];
        rfidRead.firstBlock = sector * BLOCK_PER_SECTOR;
        rfidRead.trailerBlockAddr = rfidRead.firstBlock + BLOCK_PER_SECTOR - 1;
        status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidRead.trailerBlockAddr, &key, &mfrc522.uid);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.print(F("PCD_Authenticate() Trailerblock failed at sector: "));
            Serial.println(sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
            return -1;
        }
        status = mfrc522.MIFARE_Read(rfidRead.trailerBlockAddr, rfidRead.trailerBuffer, &rfidRead.byteCount);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
            Serial.println(sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
            return -1;
        }
        rfidRead.dataBlockAddr = rfidRead.firstBlock + blockOffset;
        memset(rfidRead.dataBuffer, 0, rfidRead.byteCount);
        status = mfrc522.MIFARE_Read(rfidRead.dataBlockAddr, rfidRead.dataBuffer, &rfidRead.byteCount);
        if (status != mfrc522.STATUS_OK)
        {
            Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
            Serial.println(sector);
            Serial.println(mfrc522.GetStatusCodeName(status));
            vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
            return -1;
        }
        if (isUsername)
        {
            strncpy(userBuffer.username, (char *)rfidRead.dataBuffer, MAX_BLOCK_BUFFER);
            isUsername = false;
            isPassword = true;
        }
        else if (isPassword)
        {
            strncpy(userBuffer.password, (char *)rfidRead.dataBuffer, MAX_BLOCK_BUFFER);
            isPassword = false;
        }
        else
        {
            userBuffer.id = rfidRead.dataBuffer[0];
        }
        if (checkInvertedNibblesMatch(rfidRead.trailerBuffer))
        {
            Serial.print(F(" Inverted access bits did not match! "));
            return -1;
        }
    }

    status = check_write_state();
    if (status == WriteState::WRITING)
    {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return -3;
    }
    else if (status == WriteState::UNKNOWN)
    {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return -4;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return isStored(userBuffer.username, userBuffer.password, userBuffer.id);
}

void doCLI(void *parameters)
{
    char terminal_buffer[MAX_MSG_BUFFER];
    memset(terminal_buffer, 0, MAX_MSG_BUFFER);

    xQueueSend(terminal_queue, "Hello", portMAX_DELAY);
    while (1)
    {
        if (xQueueReceive(terminal_queue, (void *)&terminal_buffer, 0) == pdTRUE)
        {
            terminal.println(terminal_buffer);
            memset(terminal_buffer, 0, MAX_MSG_BUFFER);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void DoorAuthenticationTaskCenter(void *parameters)
{
    while (1)
    {
        switch (mode)
        {
        case VALIDATE:
            if (validateUser())
                mode = WAIT;
            break;
        case ADD:
            addNewUser();
            mode = VALIDATE;
            break;
        case CHANGE_PASSWORD:
            updateUserPassword();
            mode = VALIDATE;
            break;
        case DELETE:
            deleteUser();
            mode = VALIDATE;
            break;
        }
        if (cancel_flag)
            cancel_flag = false;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// SUPPORT FUNCTIONS
bool checkInvertedNibblesMatch(byte *trailerBuffer)
{
    byte c1, c2, c3;    // Nibbles
    byte c1_, c2_, c3_; // Inverted nibbles
    c1 = trailerBuffer[7] >> 4;
    c2 = trailerBuffer[8] & 0xF;
    c3 = trailerBuffer[8] >> 4;
    c1_ = trailerBuffer[6] & 0xF;
    c2_ = trailerBuffer[6] >> 4;
    c3_ = trailerBuffer[7] & 0xF;
    return (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
}

bool isOpCancelled()
{
    if (cancel_flag)
    {
        xQueueSend(terminal_queue, "Operation cancelled", portMAX_DELAY);
        return true;
    }
    return false;
}

bool validateUser()
{
    if (storageStatus() == STORAGE_EMPTY)
    {
        xQueueSend(terminal_queue, "The Credential list is empty, please add new credential!", portMAX_DELAY);
        door.isValidated = true;
        return true;
    }

    door.isValidated = false;
    xQueueSend(terminal_queue, "Scan your card to validate....", 0);

    int recover_status = validateCredentialAccount();
    if (recover_status >= 0)
    {
        door.isValidated = true;
        xQueueSend(terminal_queue, "Access Granted!", 0);

        char welcomeMsg[MAX_MSG_BUFFER];
        sprintf(welcomeMsg, "Welcome %s!", userBuffer.username);
        xQueueSend(terminal_queue, welcomeMsg, 0); // Removed unnecessary cast
        return true;
    }

    switch (recover_status)
    {
    case -2:
        xQueueSend(terminal_queue, "Card does not belong to credential!", 0);
        break;
    case -3:
    {
        xQueueSend(terminal_queue, "Card data is corrupted!", 0);
        xQueueSend(terminal_queue, "Recovering...", 0);
        xQueueSend(terminal_queue, "Please do not take away your card...", 0);

        get_user_credentials(userBuffer.username, MAX_BLOCK_BUFFER + 1, userBuffer.password, MAX_BLOCK_BUFFER + 1, userBuffer.id);

        enum OpStatus recover_w_status = writeUserInfoToRFID(ALL); // Scoped variable to avoid jump issues
        if (recover_w_status == OP_STATUS_OK)
        {
            xQueueSend(terminal_queue, "Card is recovered!", 0);
        }
        else
        {
            xQueueSend(terminal_queue, "Recover card failed!", 0);
        }
        break;
    }
    case -4:
        xQueueSend(terminal_queue, "Card is invalid!", 0);
        break;
    default:
        xQueueSend(terminal_queue, "Access Denied!", 0);
        break;
    }

    return false;
}

void addNewUser()
{
    if (storageStatus() == STORAGE_FULL)
    {
        xQueueSend(terminal_queue, "The Credential list is full!", portMAX_DELAY);
        return;
    }

    xQueueSend(terminal_queue, "Start adding new user...", portMAX_DELAY);
    xQueueSend(terminal_queue, "Enter username:", portMAX_DELAY);

    memset(userBuffer.username, 0, MAX_BLOCK_BUFFER + 1);
    memset(userBuffer.password, 0, MAX_BLOCK_BUFFER + 1);
    while (xQueueReceive(username_queue, (void *)&userBuffer.username, 0) != pdTRUE)
    {
        if (isOpCancelled())
            return;
    }
    xQueueSend(terminal_queue, "Enter password:", portMAX_DELAY);
    while (xQueueReceive(password_queue, (void *)&userBuffer.password, 0) != pdTRUE)
    {
        if (isOpCancelled())
            return;
    }
    userBuffer.username[MAX_BLOCK_BUFFER] = '\0';
    userBuffer.password[MAX_BLOCK_BUFFER] = '\0';

    userBuffer.id = getNewID();
    if (userBuffer.id < 0)
    {
        xQueueSend(terminal_queue, "Add new user failed", portMAX_DELAY);
        return;
    }

    xQueueSend(terminal_queue, "Insert your card to set card data...", portMAX_DELAY);
    OpStatus status = writeUserInfoToRFID(ALL);
    if (status == OP_STATUS_OK)
    {
        userBuffer.id = save_user_credentials(userBuffer.username, userBuffer.password);

        xQueueSend(terminal_queue, "New user is added", portMAX_DELAY);
        xQueueSend(terminal_queue, "This card is now available", portMAX_DELAY);
    }
    else if (status == OP_STATUS_FAILED)
        xQueueSend(terminal_queue, "Add new user failed", portMAX_DELAY);
}
void updateUserPassword()
{
    if (storageStatus() == STORAGE_EMPTY)
    {
        xQueueSend(terminal_queue, "The Credential list is empty!", portMAX_DELAY);
        return;
    }
    xQueueSend(terminal_queue, "Start changing password...", portMAX_DELAY);
    xQueueSend(terminal_queue, "Enter new password: ", portMAX_DELAY);
    memset(userBuffer.password, 0, MAX_BLOCK_BUFFER + 1);
    while (xQueueReceive(password_queue, (void *)&userBuffer.password, 0) != pdTRUE)
    {
        if (isOpCancelled())
            return;
    }
    userBuffer.password[MAX_BLOCK_BUFFER] = '\0';
    xQueueSend(terminal_queue, "Insert your card to set card data...", portMAX_DELAY);
    enum OpStatus status = writeUserInfoToRFID(ONLY_PASSWORD);
    if (status == OP_STATUS_OK)
    {
        xQueueSend(terminal_queue, "Password changed successfully!", portMAX_DELAY);
        change_user_password(userBuffer.password, userBuffer.id);
        xQueueSend(terminal_queue, "New password added!", portMAX_DELAY);
    }
    else if (status == OP_STATUS_FAILED)
        xQueueSend(terminal_queue, "Change user password failed", portMAX_DELAY);
}
OpStatus deleteOnCard()
{
    while (!isCardInserted())
    {
        if (isOpCancelled())
            return OP_STATUS_CANCELLED;
    }
    bool status = clearAllDataBlock();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    if (status)
        return OP_STATUS_OK;
    return OP_STATUS_FAILED;
}
void deleteUser()
{
    xQueueSend(terminal_queue, "Start deleting user...", portMAX_DELAY);
    xQueueSend(terminal_queue, "Insert your card to delete card data...", portMAX_DELAY);
    OpStatus status = deleteOnCard();
    if (status == OpStatus::OP_STATUS_OK)
    {
        xQueueSend(terminal_queue, "Your card is cleared", portMAX_DELAY);
        Serial.println(delete_user_credentials(userBuffer.id));
        xQueueSend(terminal_queue, "User is deleted!", portMAX_DELAY);
    }
    else if (status == OpStatus::OP_STATUS_FAILED)
    {
        xQueueSend(terminal_queue, "Delete user failed", portMAX_DELAY);
    }
}

void init_door_authentication()
{
    M5.begin();       // Initialize M5Stack
    M5.Power.begin(); // Initialize power module

    init_nvs();

    Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C bus, customized to SDA = GPIO11, SCL = GPIO12
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    mfrc522.PCD_Init(); // Initialize MFRC522

    terminal_queue = xQueueCreate(terminal_queue_len, MAX_MSG_BUFFER);
    username_queue = xQueueCreate(username_queue_len, MAX_BLOCK_BUFFER);
    password_queue = xQueueCreate(password_queue_len, MAX_BLOCK_BUFFER);

    // clear_user_credentials();
    // dump_user_credentials();
}
