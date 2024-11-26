// /*
// Choose multiple: Ctrl+D
// Reindent: Shift+Alt+F
// */
// #define ERA_LOCATION_VN
// #define ERA_AUTH_TOKEN "1b2f6d4f-0330-43bb-9ac8-8b31ce65c61f" // Must be on top
// // #define ERA_AUTH_TOKEN "d0ca164a-fefe-49c8-a17d-93e960134a5f" // Must be on top
// #include "authentication.h"
// #include <ERa.hpp>
// #include <Widgets/ERaWidgets.hpp>

// // GLOBALS
// // const char ssid[] = "Guess who?";
// // const char pass[] = "yourmama";
// const char ssid[] = "THANHHAU";
// const char pass[] = "30110902";

// static enum OpMode mode = VALIDATE;

// static Door door;
// static UserInfo userBuffer;
// static bool cancelTaskFlag = false;

// static ERaString fromStr;
// static ERaWidgetTerminalBox terminal(fromStr, V43, V44);

// static const uint8_t username_queue_len = 5;
// static const uint8_t password_queue_len = 5;
// static const uint8_t terminal_queue_len = 5;

// static QueueHandle_t username_queue;
// static QueueHandle_t password_queue;
// static QueueHandle_t terminal_queue;

// // ERa callbacks

// /*
// Virtual pins list:
// - V41: Blink onboard led
// - V42: RC522 mode, set value of ADD=1, FORGET_PASSWORD=2, SCAN=3
// - V43: ERa Terminal send, set value to 0
// - V44: ERa Terminal receive
// - V45: Username
// - V46: Password
// */
// ERA_WRITE(V41)
// {
//     byte value = param.getInt();
//     digitalWrite(LED_ON_BOARD, value);
// }

// ERA_WRITE(V42)
// {
//     if (!door.isValidated)
//     {
//         xQueueSend(terminal_queue, "Scan your card to validate....", portMAX_DELAY);
//         return;
//     }

//     byte server_mode = param.getInt();

//     switch (server_mode)
//     {
//     case 1:
//         mode = ADD;
//         break;
//     case 2:
//         mode = CHANGE_PASSWORD;
//         break;
//     case 3:
//         cancelTaskFlag = true;
//         break;
//     }
// }
// ERA_WRITE(V45)
// {
//     if (param.getStringLength() > MAX_BLOCK_BUFFER)
//     {
//         xQueueSend(terminal_queue, "Username is too long", portMAX_DELAY);
//     }
//     else
//     {
//         xQueueSend(username_queue, param.getString(), portMAX_DELAY);
//     }
// }
// ERA_WRITE(V46)
// {
//     if (param.getStringLength() > MAX_BLOCK_BUFFER)
//     {
//         xQueueSend(terminal_queue, "Password is too long", portMAX_DELAY);
//     }
//     else
//     {
//         xQueueSend(password_queue, param.getString(), portMAX_DELAY);
//     }
// }
// ERA_WRITE(V47)
// {
//     switch (param.getInt())
//     {
//     case 0:
//         door.isOpened = false;

//         mode = VALIDATE;
//         break;
//     case 1:
//         if (door.isValidated)
//             door.isOpened = true;
//         else
//         {
//             ERa.virtualWrite(V47, 0);
//             xQueueSend(terminal_queue, "Scan your card to validate....", portMAX_DELAY);
//         }
//         break;
//     }
// }

// //-----------------------------------------------------

// // Tasks
// void printTextData()
// {
//     RFID_Device rfidRead;
//     char blockDataStr[MAX_BLOCK_BUFFER + 1];

//     while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//     {
//         vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//         Serial.println("No card");
//     }

//     for (byte sector = 0; sector < MAX_SECTOR;)
//     {
//         rfidRead.firstBlock = sector * BLOCK_PER_SECTOR;

//         if (rfidRead.isNewSector)
//         {
//             rfidRead.trailerBlockAddr = rfidRead.firstBlock + BLOCK_PER_SECTOR - 1;
//             status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidRead.trailerBlockAddr, &key, &mfrc522.uid);
//             if (status != mfrc522.STATUS_OK)
//             {
//                 Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                 Serial.println(sector);
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             status = mfrc522.MIFARE_Read(rfidRead.trailerBlockAddr, rfidRead.trailerBuffer, &rfidRead.byteCount);
//             if (status != mfrc522.STATUS_OK)
//             {
//                 Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                 Serial.println(sector);
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             rfidRead.isNewSector = false;
//         }
//         rfidRead.dataBlockAddr = rfidRead.firstBlock + rfidRead.blockOffset;
//         memset(rfidRead.dataBuffer, 0, rfidRead.byteCount);
//         status = mfrc522.MIFARE_Read(rfidRead.dataBlockAddr, rfidRead.dataBuffer, &rfidRead.byteCount);
//         if (status != mfrc522.STATUS_OK)
//         {
//             Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
//             Serial.println(sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             continue;
//         }
//         if (rfidRead.dataBuffer[0] == 0x00 && rfidRead.blockOffset == 0)
//             return;

//         if (rfidRead.dataBuffer[0] != 0x00)
//         {
//             for (byte index = 0; index < 16; index++)
//             {
//                 if (rfidRead.dataBuffer[index] < 0x10)
//                     Serial.print(F(" 0"));
//                 else
//                     Serial.print(F(" "));
//                 Serial.print(rfidRead.dataBuffer[index], HEX);
//                 if ((index % 4) == 3)
//                 {
//                     Serial.print(F(" "));
//                 }
//             }
//             Serial.println();
//             memset(blockDataStr, 0, MAX_BLOCK_BUFFER + 1);
//             for (byte index = 0; index < MAX_BLOCK_BUFFER; index++)
//             {
//                 blockDataStr[index] = rfidRead.dataBuffer[index];
//             }
//             blockDataStr[MAX_BLOCK_BUFFER] = '\0';
//             Serial.print("Current blockDataStr: ");
//             Serial.println(blockDataStr);
//             vTaskDelay(500 / portTICK_PERIOD_MS);
//         }

//         if (checkInvertedNibblesMatch(rfidRead.trailerBuffer))
//         {
//             Serial.print(F(" Inverted access bits did not match! "));
//         }

//         rfidRead.blockOffset++;
//         if (rfidRead.blockOffset >= BLOCK_PER_SECTOR - 1)
//         {
//             rfidRead.blockOffset = 0;
//             sector++;
//             rfidRead.isNewSector = true;
//         }
//     }

//     mfrc522.PICC_HaltA();
//     mfrc522.PCD_StopCrypto1();
//     terminal.println("Read successfully!");

//     rfidRead.blockOffset = 1;
// }
// void dumpData(void *parameters)
// {
//     while (1)
//     {
//         if (mode == DUMP)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             mfrc522.PICC_DumpToSerial(&mfrc522.uid);
//             terminal.println("Read successfully!");
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void clearAllDataBlock()
// {
//     RFID_Device rfidClear;
//     byte blockEraser[MAX_BLOCK_BUFFER];

//     memset(blockEraser, 0, MAX_BLOCK_BUFFER);

//     for (byte sector = 0; sector < MAX_SECTOR;)
//     {
//         rfidClear.firstBlock = sector * BLOCK_PER_SECTOR;

//         if (rfidClear.isNewSector)
//         {
//             rfidClear.trailerBlockAddr = rfidClear.firstBlock + BLOCK_PER_SECTOR - 1;
//             status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidClear.trailerBlockAddr, &key, &mfrc522.uid);
//             if (status != mfrc522.STATUS_OK)
//             {
//                 Serial.print(F("Authenticate Trailerblock failed at sector: "));
//                 Serial.println(sector);
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             status = mfrc522.MIFARE_Read(rfidClear.trailerBlockAddr, rfidClear.trailerBuffer, &rfidClear.byteCount);
//             if (status != mfrc522.STATUS_OK)
//             {
//                 Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                 Serial.println(sector);
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             rfidClear.isNewSector = false;
//         }
//         rfidClear.dataBlockAddr = rfidClear.firstBlock + rfidClear.blockOffset;
//         status = mfrc522.MIFARE_Write(rfidClear.dataBlockAddr, blockEraser, MAX_BLOCK_BUFFER);
//         if (status != mfrc522.STATUS_OK)
//         {
//             Serial.print(F("MIFARE_Write() blockEraser failed at sector: "));
//             Serial.println(sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             continue;
//         }
//         if (checkInvertedNibblesMatch(rfidClear.trailerBuffer))
//         {
//             Serial.print(F(" Inverted access bits did not match! "));
//         }

//         rfidClear.blockOffset++;
//         if (rfidClear.blockOffset >= BLOCK_PER_SECTOR - 1)
//         {
//             rfidClear.blockOffset = 0;
//             sector++;
//             rfidClear.isNewSector = true;
//         }
//     }
// }
// void writeUserInfoToRFID(WriteMode write_mode)
// {
//     RFID_Device w_rfid;
//     byte w_sector;

//     while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//     {
//         vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
//     }
//     xQueueSend(terminal_queue, "Scanning... Please stay still!", portMAX_DELAY);
//     switch (write_mode)
//     {
//     case ALL:
//         clearAllDataBlock();
//         w_sector = 0;
//         // Loop through attributes and write to blocks
//         for (byte infoIdx = USERNAME; infoIdx <= ID; infoIdx++)
//         {
//             w_rfid.firstBlock = w_sector * BLOCK_PER_SECTOR;
//             w_rfid.dataBlockAddr = w_rfid.firstBlock + w_rfid.blockOffset;

//             // Authenticate with the sector's trailer block
//             if (w_rfid.isNewSector)
//             {
//                 w_rfid.trailerBlockAddr = w_rfid.firstBlock + BLOCK_PER_SECTOR - 1;
//                 status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, w_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
//                 if (status != MFRC522::STATUS_OK)
//                 {
//                     Serial.print("Authentication failed for sector ");
//                     Serial.println(w_sector);
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     vTaskDelay(200 / portTICK_PERIOD_MS);
//                     break;
//                 }
//                 w_rfid.isNewSector = false;
//             }

//             memset(w_rfid.dataBuffer, 0, w_rfid.byteCount);
//             switch (infoIdx)
//             {
//             case USERNAME:
//                 memcpy(w_rfid.dataBuffer, userBuffer.username, MAX_BLOCK_BUFFER);
//                 status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
//                 break;
//             case PASSWORD:
//                 memcpy(w_rfid.dataBuffer, userBuffer.password, MAX_BLOCK_BUFFER);
//                 status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
//                 break;
//             case ID:
//                 w_rfid.dataBuffer[0] = userBuffer.id;
//                 status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
//                 break;
//             }
//             if (status != MFRC522::STATUS_OK)
//             {
//                 Serial.print("Writing to block failed: ");
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 break;
//             }

//             // Advance to next block
//             w_rfid.blockOffset++;
//             if (w_rfid.blockOffset >= BLOCK_PER_SECTOR - 1)
//             {
//                 w_rfid.blockOffset = 0; // Reset block offset
//                 w_sector++;             // Move to the next sector
//                 w_rfid.isNewSector = true;
//             }
//         }
//         break;
//     case ONLY_PASSWORD:
//         // Loop through attributes and write to blocks
//         w_sector = 0;
//         w_rfid.blockOffset = 2;
//         w_rfid.firstBlock = w_sector * BLOCK_PER_SECTOR;
//         w_rfid.dataBlockAddr = w_rfid.firstBlock + w_rfid.blockOffset;
//         w_rfid.trailerBlockAddr = w_rfid.firstBlock + BLOCK_PER_SECTOR - 1;
//         // Authenticate with the sector's trailer block
//         status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, w_rfid.trailerBlockAddr, &key, &(mfrc522.uid));
//         if (status != MFRC522::STATUS_OK)
//         {
//             Serial.printf("Authentication failed for sector %d\n", w_sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS);
//             break;
//         }

//         memset(w_rfid.dataBuffer, 0, w_rfid.byteCount);
//         memcpy(w_rfid.dataBuffer, userBuffer.password, MAX_BLOCK_BUFFER);
//         status = mfrc522.MIFARE_Write(w_rfid.dataBlockAddr, w_rfid.dataBuffer, MAX_BLOCK_BUFFER);
//         if (status != MFRC522::STATUS_OK)
//         {
//             Serial.print("Writing to block failed: ");
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             break;
//         }
//         break;
//     }

//     // Halt and stop the reader after completing the writing process
//     mfrc522.PICC_HaltA();
//     mfrc522.PCD_StopCrypto1();
// }
// int validateCredentialAccount()
// {
//     RFID_Device rfidRead;
//     byte sector, blockOffset;
//     bool isUsername = true;
//     bool isPassword = false;

//     userBuffer.location[USERNAME][0] = 0;
//     userBuffer.location[USERNAME][1] = 1;
//     userBuffer.location[PASSWORD][0] = 0;
//     userBuffer.location[PASSWORD][1] = 2;
//     userBuffer.location[ID][0] = 1;
//     userBuffer.location[ID][1] = 0;

//     while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//     {
//         vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
//     }
//     for (auto loc : userBuffer.location)
//     {
//         sector = loc[0];
//         blockOffset = loc[1];
//         rfidRead.firstBlock = sector * BLOCK_PER_SECTOR;
//         rfidRead.trailerBlockAddr = rfidRead.firstBlock + BLOCK_PER_SECTOR - 1;
//         status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, rfidRead.trailerBlockAddr, &key, &mfrc522.uid);
//         if (status != mfrc522.STATUS_OK)
//         {
//             Serial.print(F("PCD_Authenticate() Trailerblock failed at sector: "));
//             Serial.println(sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             return -1;
//         }
//         status = mfrc522.MIFARE_Read(rfidRead.trailerBlockAddr, rfidRead.trailerBuffer, &rfidRead.byteCount);
//         if (status != mfrc522.STATUS_OK)
//         {
//             Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//             Serial.println(sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             return -1;
//         }
//         rfidRead.dataBlockAddr = rfidRead.firstBlock + blockOffset;
//         memset(rfidRead.dataBuffer, 0, rfidRead.byteCount);
//         status = mfrc522.MIFARE_Read(rfidRead.dataBlockAddr, rfidRead.dataBuffer, &rfidRead.byteCount);
//         if (status != mfrc522.STATUS_OK)
//         {
//             Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
//             Serial.println(sector);
//             Serial.println(mfrc522.GetStatusCodeName(status));
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             return -1;
//         }
//         if (isUsername)
//         {
//             strncpy(userBuffer.username, (char *)rfidRead.dataBuffer, MAX_BLOCK_BUFFER);
//             isUsername = false;
//             isPassword = true;
//         }
//         else if (isPassword)
//         {
//             strncpy(userBuffer.password, (char *)rfidRead.dataBuffer, MAX_BLOCK_BUFFER);
//             isPassword = false;
//         }
//         else
//         {
//             userBuffer.id = rfidRead.dataBuffer[0];
//         }
//         if (checkInvertedNibblesMatch(rfidRead.trailerBuffer))
//         {
//             Serial.print(F(" Inverted access bits did not match! "));
//             return -1;
//         }
//     }

//     mfrc522.PICC_HaltA();
//     mfrc522.PCD_StopCrypto1();
//     return isStored(userBuffer.username, userBuffer.password);
// }

// void doCLI(void *parameters)
// {
//     char terminal_buffer[MAX_MSG_BUFFER];
//     memset(terminal_buffer, 0, MAX_MSG_BUFFER);

//     xQueueSend(terminal_queue, "Hello", portMAX_DELAY);
//     while (1)
//     {
//         if (xQueueReceive(terminal_queue, (void *)&terminal_buffer, 0) == pdTRUE)
//         {
//             terminal.println(terminal_buffer);
//             memset(terminal_buffer, 0, MAX_MSG_BUFFER);
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void taskCenter(void *parameters)
// {
//     while (1)
//     {
//         switch (mode)
//         {
//         case VALIDATE:
//             if (validateUser())
//                 mode = WAIT;
//             break;
//         case ADD:
//             addNewUser();
//             mode = VALIDATE;
//             break;
//         case CHANGE_PASSWORD:
//             updateUserPassword();
//             mode = VALIDATE;
//             break;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void cancelTask(void *parameters)
// {
//     while (1)
//     {
//         if (cancelTaskFlag)
//         {
//             cancelTaskFlag = false;
//             mode = VALIDATE;
//             xQueueSend(terminal_queue, "Operation is cancelled", portMAX_DELAY);
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void setup()
// {
//     M5.begin();                   // Initialize M5Stack
//     M5.Power.begin();             // Initialize power module
//     Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C bus, customized to SDA = GPIO11, SCL = GPIO12

//     /* Setup debug console */
//     Serial.begin(115200);

//     init_nvs();

//     ERa.begin(ssid, pass);
//     mfrc522.PCD_Init(); // Initialize MFRC522
//     pinMode(LED_ON_BOARD, OUTPUT);
//     digitalWrite(LED_ON_BOARD, LOW);
//     vTaskDelay(3000 / portTICK_PERIOD_MS);

//     for (byte i = 0; i < 6; i++)
//         key.keyByte[i] = 0xFF;

//     terminal_queue = xQueueCreate(terminal_queue_len, MAX_MSG_BUFFER);
//     username_queue = xQueueCreate(username_queue_len, MAX_BLOCK_BUFFER);
//     password_queue = xQueueCreate(password_queue_len, MAX_BLOCK_BUFFER);
//     // mode = DUMP;
//     // xTaskCreatePinnedToCore(dumpData, "Dump Data", 4096, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(doCLI, "Do CLI", 4096, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(taskCenter, "Task Center", 4096, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(cancelTask, "Cancel Task", 1024, NULL, 2, NULL, app_cpu);
//     dump_user_credentials();

//     /* Setup  called function every second */
// }

// void loop()
// {
//     ERa.run();
// }

// // SUPPORT FUNCTIONS
// bool checkInvertedNibblesMatch(byte *trailerBuffer)
// {
//     byte c1, c2, c3;    // Nibbles
//     byte c1_, c2_, c3_; // Inverted nibbles
//     c1 = trailerBuffer[7] >> 4;
//     c2 = trailerBuffer[8] & 0xF;
//     c3 = trailerBuffer[8] >> 4;
//     c1_ = trailerBuffer[6] & 0xF;
//     c2_ = trailerBuffer[6] >> 4;
//     c3_ = trailerBuffer[7] & 0xF;
//     return (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
// }

// bool validateUser()
// {
//     if (storageStatus() == STORAGE_EMPTY)
//     {
//         xQueueSend(terminal_queue, "The Credential list is empty, please add new credential!", portMAX_DELAY);
//         return true;
//     }
//     char validate_buffer[MAX_MSG_BUFFER];
//     door.isValidated = false;
//     xQueueSend(terminal_queue, "Scan your card to validate....", portMAX_DELAY);
//     Serial.println("Gohere");
//     if (validateCredentialAccount() >= 0 || storageStatus() == STORAGE_EMPTY)
//     {
//         door.isValidated = true;
//         xQueueSend(terminal_queue, "Access Granted!", portMAX_DELAY);
//         char welcomeMsg[MAX_MSG_BUFFER];
//         sprintf(welcomeMsg, "Welcome %s!", userBuffer.username);
//         xQueueSend(terminal_queue, (void *)&welcomeMsg, portMAX_DELAY);
//         return true;
//     }
//     else
//     {
//         door.isValidated = false;
//         xQueueSend(terminal_queue, "Access Denied!", portMAX_DELAY);
//         return false;
//     }
// }

// void addNewUser()
// {
//     if (storageStatus() == STORAGE_FULL)
//     {
//         xQueueSend(terminal_queue, "The Credential list is full!", portMAX_DELAY);
//         return;
//     }

//     xQueueSend(terminal_queue, "Start adding new user...", portMAX_DELAY);
//     xQueueSend(terminal_queue, "Enter username:", portMAX_DELAY);

//     memset(userBuffer.username, 0, MAX_BLOCK_BUFFER + 1);
//     memset(userBuffer.password, 0, MAX_BLOCK_BUFFER + 1);
//     xQueueReceive(username_queue, (void *)&userBuffer.username, portMAX_DELAY);
//     xQueueSend(terminal_queue, "Enter password:", portMAX_DELAY);
//     xQueueReceive(password_queue, (void *)&userBuffer.password, portMAX_DELAY);
//     userBuffer.username[MAX_BLOCK_BUFFER] = '\0';
//     userBuffer.password[MAX_BLOCK_BUFFER] = '\0';
//     userBuffer.id = save_user_credentials(userBuffer.username, userBuffer.password);

//     xQueueSend(terminal_queue, "New user is added", portMAX_DELAY);
//     xQueueSend(terminal_queue, "Insert your card to set card data...", portMAX_DELAY);

//     writeUserInfoToRFID(ALL);
//     xQueueSend(terminal_queue, "This card is now available", portMAX_DELAY);
// }
// void updateUserPassword()
// {
//     if (storageStatus() == STORAGE_FULL)
//     {
//         xQueueSend(terminal_queue, "The Credential list is empty!", portMAX_DELAY);
//         return;
//     }
//     xQueueSend(terminal_queue, "Start changing password...", portMAX_DELAY);
//     xQueueSend(terminal_queue, "Enter new password: ", portMAX_DELAY);
//     memset(userBuffer.password, 0, MAX_BLOCK_BUFFER + 1);
//     xQueueReceive(password_queue, (void *)&userBuffer.password, portMAX_DELAY);
//     userBuffer.password[MAX_BLOCK_BUFFER] = '\0';
//     change_user_password(userBuffer.password, userBuffer.id);
//     xQueueSend(terminal_queue, "New password added!", portMAX_DELAY);
//     xQueueSend(terminal_queue, "Insert your card to set card data...", portMAX_DELAY);
//     writeUserInfoToRFID(ONLY_PASSWORD);
//     xQueueSend(terminal_queue, "Password changed successfully!", portMAX_DELAY);
// }
// void deleteUserCard()
// {
// }
