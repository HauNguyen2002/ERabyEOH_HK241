
// /*************************************************************
// Define SPI to use library from Miguel Balboa. https://github.com/miguelbalboa/rfid.
// Define I2C to use M5Stack RC522 module and library from M5Stack. https://github.com/m5stack/M5Stack/tree/master/examples/Unit/RFID_RC522
//  *************************************************************/

// #define ERA_LOCATION_VN
// #define ERA_AUTH_TOKEN "1b2f6d4f-0330-43bb-9ac8-8b31ce65c61f" // Must be on top
// // #define ERA_AUTH_TOKEN "d0ca164a-fefe-49c8-a17d-93e960134a5f" // Must be on top

// #include <Arduino.h>
// #include <ERa.hpp>
// #include <Widgets/ERaWidgets.hpp>
// #include <freeRTOS/FreeRTOS.h>

// #if CONFIG_FREERTOS_UNICORE
// static const BaseType_t app_cpu = 0;
// #else
// static const BaseType_t app_cpu = 1;
// #endif

// // USER DEFINES
// #define I2C
// // #define SPI

// #ifdef I2C
// #include <M5Unified.h>
// #include "MFRC522_I2C.h"
// #endif

// #define MAX_BLOCK_BUFFER 16
// #define MAX_MSG_BUFFER 20
// #define LED_ON_BOARD 48
// #define ITEM_ATTRIBUTES 6
// #define BLOCK_PER_SECTOR 4
// #define MAX_SECTOR 16

// #ifdef I2C
// #define SDA_PIN 11 // Configurable, see typical pin layout above
// #define SCL_PIN 12 // Configurable, see typical pin layout above
// #endif

// #ifdef SPI
// #define RST_PIN 9 // Configurable, see typical pin layout above
// #define SS_PIN 10 // Configurable, see typical pin layout above
// #endif

// // GLOBALS
// // const char ssid[] = "Guess who?";
// // const char pass[] = "yourmama";
// const char ssid[] = "THANHHAU";
// const char pass[] = "30110902";

// #ifdef I2C
// // Create MFRC522 instance with I2C address 0x28, comment it out if using SPI/UART
// static MFRC522 mfrc522(0x28);
// #endif

// #ifdef SPI
// /*Create MFRC522 instance with SPI*/
// MFRC522 mfrc522(SS_PIN, RST_PIN);
// #endif

// MFRC522::MIFARE_Key key;
// static byte blockData[MAX_BLOCK_BUFFER];
// static byte itemData[ITEM_ATTRIBUTES][MAX_BLOCK_BUFFER];
// static byte blockEraser[MAX_BLOCK_BUFFER];
// static char blockDataStr[MAX_BLOCK_BUFFER + 1];
// static byte status;

// static char msg[MAX_MSG_BUFFER];
// enum OpMode
// {
//     WAIT,
//     WAIT_WRITE,
//     READ,
//     WRITE,
//     ADD_MSG,
//     CLEAN
// };
// static enum OpMode mode = WAIT;

// enum CLIState
// {
//     m_WAIT,
//     m_READ,
//     m_WRITE,
//     m_EMPTY_WRITE,
//     m_INVALID_WRITE,
//     m_VALID_WRITE,
//     m_INVALID_OP,
//     m_CLEAN,
//     m_CLEAN_SUCCESS
// };
// static enum CLIState cli_state = m_WAIT;

// enum itemAttr
// {
//     MODEL_NAME,
//     COLOR,
//     SIZE,
//     CATEGORY,
//     PRICE,
//     DISCOUNT
// };

// static ERaString fromStr;
// static ERaWidgetTerminalBox terminal(fromStr, V2, V3);
// static ERaWidgetTerminalBox terminal_readData(fromStr, V4, V5);

// // ERa callbacks

// /*
// Virtual pins list:
// - V0: Blink onboard led
// - V1: RC522 mode, set value of READ=1, WRITE=2
// - V2: ERa Terminal send, set value to 0
// - V3: ERa Terminal receive
// */
// ERA_WRITE(V41)
// {
//     int value = param.getInt();
//     digitalWrite(LED_ON_BOARD, value);
// }

// ERA_WRITE(V1)
// {
//     int server_mode = param.getInt();
//     switch (server_mode)
//     {
//     case 1:
//         cli_state = m_READ;
//         break;
//     case 2:
//         cli_state = m_WRITE;
//         break;
//     case 3:
//         cli_state = m_CLEAN;
//         break;
//     }
// }
// ERA_WRITE(V6)
// {
//     memset(itemData[MODEL_NAME], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[MODEL_NAME], param.getString(), param.getStringLength());
// }
// ERA_WRITE(V7)
// {
//     memset(itemData[COLOR], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[COLOR], param.getString(), param.getStringLength());
// }
// ERA_WRITE(V8)
// {
//     memset(itemData[SIZE], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[SIZE], param.getString(), param.getStringLength());
// }
// ERA_WRITE(V9)
// {
//     memset(itemData[CATEGORY], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[CATEGORY], param.getString(), param.getStringLength());
// }
// ERA_WRITE(V10)
// {
//     memset(itemData[PRICE], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[PRICE], param.getString(), param.getStringLength());
// }
// ERA_WRITE(V11)
// {
//     memset(itemData[DISCOUNT], 0, MAX_BLOCK_BUFFER);
//     memcpy(itemData[DISCOUNT], param.getString(), param.getStringLength());
// }

// void terminalCallback()
// {
//     if (mode == WAIT_WRITE)
//     {
//         if (fromStr == "")
//         {
//             cli_state = m_EMPTY_WRITE;
//         }
//         else if (fromStr.length() > 16)
//         {
//             cli_state = m_INVALID_WRITE;
//         }
//         else
//         {
//             memset(blockData, 0, MAX_BLOCK_BUFFER);
//             memcpy(blockData, fromStr, fromStr.length());
//             cli_state = m_VALID_WRITE;
//         }
//     }
//     else
//     {
//         cli_state = m_INVALID_OP;
//     }
// }
// //-----------------------------------------------------
// // Forward Declarations
// bool checkInvertedNibblesMatch(byte *trailerBuffer);

// // Tasks
// void printTextData(void *parameters)
// {
//     byte firstBlock;
//     byte dataBuffer[18];
//     byte byteCount = sizeof(dataBuffer);
//     byte trailerBuffer[18];
//     byte dataBlockAddr;
//     byte trailerBlockAddr;
//     byte r_blockOffset = 1;
//     bool isNewSector = true;
//     while (1)
//     {
//         if (mode == READ)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }

//             for (byte r_sector = 0; r_sector < MAX_SECTOR;)
//             {
//                 firstBlock = r_sector * BLOCK_PER_SECTOR;

//                 if (isNewSector)
//                 {
//                     trailerBlockAddr = firstBlock + BLOCK_PER_SECTOR - 1;
//                     status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailerBlockAddr, &key, &mfrc522.uid);
//                     if (status != mfrc522.STATUS_OK)
//                     {
//                         Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                         Serial.println(r_sector);
//                         Serial.println(mfrc522.GetStatusCodeName(status));
//                         vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                         continue;
//                     }
//                     status = mfrc522.MIFARE_Read(trailerBlockAddr, trailerBuffer, &byteCount);
//                     if (status != mfrc522.STATUS_OK)
//                     {
//                         Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                         Serial.println(r_sector);
//                         Serial.println(mfrc522.GetStatusCodeName(status));
//                         vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                         continue;
//                     }
//                     isNewSector = false;
//                 }
//                 dataBlockAddr = firstBlock + r_blockOffset;
//                 memset(dataBuffer, 0, byteCount);
//                 status = mfrc522.MIFARE_Read(dataBlockAddr, dataBuffer, &byteCount);
//                 if (status != mfrc522.STATUS_OK)
//                 {
//                     Serial.print(F("MIFARE_Read() Datablock failed at sector: "));
//                     Serial.println(r_sector);
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                     continue;
//                 }

//                 if (dataBuffer[0] != 0x00)
//                 {
//                     for (byte index = 0; index < 16; index++)
//                     {
//                         if (dataBuffer[index] < 0x10)
//                             Serial.print(F(" 0"));
//                         else
//                             Serial.print(F(" "));
//                         Serial.print(dataBuffer[index], HEX);
//                         if ((index % 4) == 3)
//                         {
//                             Serial.print(F(" "));
//                         }
//                     }
//                     Serial.println();
//                     memset(blockDataStr, 0, MAX_BLOCK_BUFFER + 1);
//                     for (byte index = 0; index < MAX_BLOCK_BUFFER; index++)
//                     {
//                         blockDataStr[index] = dataBuffer[index];
//                     }
//                     blockDataStr[16] = '\0';
//                     Serial.print("Current blockDataStr: ");
//                     Serial.println(blockDataStr);
//                     terminal_readData.println(blockDataStr);
//                     vTaskDelay(500 / portTICK_PERIOD_MS);
//                 }

//                 if (checkInvertedNibblesMatch(trailerBuffer))
//                 {
//                     Serial.print(F(" Inverted access bits did not match! "));
//                 }

//                 r_blockOffset++;
//                 if (r_blockOffset >= BLOCK_PER_SECTOR - 1)
//                 {
//                     r_blockOffset = 0;
//                     r_sector++;
//                     isNewSector = true;
//                 }
//             }

//             mfrc522.PICC_HaltA();
//             mfrc522.PCD_StopCrypto1();

//             terminal.println("Read successfully!");

//             r_blockOffset = 1;

//             cli_state = m_READ;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void dumpData(void *parameters)
// {
//     while (1)
//     {
//         if (mode == READ)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }
//             mfrc522.PICC_DumpToSerial(&mfrc522.uid);
//             cli_state = m_READ;
//             terminal.println("Read successfully!");
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void clearAllDataBlock(void *parameters)
// {
//     byte firstBlock;
//     byte trailerBuffer[18];
//     byte byteCount = sizeof(trailerBuffer);
//     byte dataBlockAddr;
//     byte trailerBlockAddr;
//     byte c_blockOffset = 1;
//     bool isNewSector = true;
//     memset(blockEraser, 0, MAX_BLOCK_BUFFER);
//     while (1)
//     {
//         if (mode == CLEAN)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                 continue;
//             }

//             for (byte c_sector = 0; c_sector < MAX_SECTOR;)
//             {
//                 firstBlock = c_sector * BLOCK_PER_SECTOR;

//                 if (isNewSector)
//                 {
//                     trailerBlockAddr = firstBlock + BLOCK_PER_SECTOR - 1;
//                     status = mfrc522.PCD_Authenticate(mfrc522.PICC_CMD_MF_AUTH_KEY_A, trailerBlockAddr, &key, &mfrc522.uid);
//                     if (status != mfrc522.STATUS_OK)
//                     {
//                         Serial.print(F("Authenticate Trailerblock failed at sector: "));
//                         Serial.println(c_sector);
//                         Serial.println(mfrc522.GetStatusCodeName(status));
//                         vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                         continue;
//                     }
//                     status = mfrc522.MIFARE_Read(trailerBlockAddr, trailerBuffer, &byteCount);
//                     if (status != mfrc522.STATUS_OK)
//                     {
//                         Serial.print(F("MIFARE_Read() Trailerblock failed at sector: "));
//                         Serial.println(c_sector);
//                         Serial.println(mfrc522.GetStatusCodeName(status));
//                         vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                         continue;
//                     }
//                     isNewSector = false;
//                 }
//                 dataBlockAddr = firstBlock + c_blockOffset;
//                 status = mfrc522.MIFARE_Write(dataBlockAddr, blockEraser, MAX_BLOCK_BUFFER);
//                 if (status != mfrc522.STATUS_OK)
//                 {
//                     Serial.print(F("MIFARE_Write() blockEraser failed at sector: "));
//                     Serial.println(c_sector);
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//                     continue;
//                 }
//                 if (checkInvertedNibblesMatch(trailerBuffer))
//                 {
//                     Serial.print(F(" Inverted access bits did not match! "));
//                 }

//                 c_blockOffset++;
//                 if (c_blockOffset >= BLOCK_PER_SECTOR - 1)
//                 {
//                     c_blockOffset = 0;
//                     c_sector++;
//                     isNewSector = true;
//                 }
//             }
//             mfrc522.PICC_HaltA();
//             mfrc522.PCD_StopCrypto1();

//             terminal.println("Clear successfully!");

//             c_blockOffset = 1;
//             cli_state = m_CLEAN_SUCCESS;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void writeRFIDfromForm(void *parameters)
// {
//     // Initialize key to default value
//     for (byte i = 0; i < 6; i++)
//         key.keyByte[i] = 0xFF;

//     memset(blockEraser, 0, MAX_BLOCK_BUFFER);
//     for (byte *attrBuffer : itemData)
//     {
//         memset(attrBuffer, 0, MAX_BLOCK_BUFFER);
//     }

//     byte w_firstBlock;
//     byte w_sector = 0;
//     byte w_block_offset = 1;
//     byte w_blockNum;

//     while (1)
//     {
//         if (mode == WRITE)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
//                 continue;
//             }

//             // Loop through attributes and write to blocks
//             for (int attr = 0; attr < ITEM_ATTRIBUTES; attr++)
//             {
//                 w_firstBlock = w_sector * BLOCK_PER_SECTOR;
//                 w_blockNum = w_firstBlock + w_block_offset;

//                 // Authenticate with the sector's trailer block
//                 status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, w_firstBlock, &key, &(mfrc522.uid));
//                 if (status != MFRC522::STATUS_OK)
//                 {
//                     terminal.print("Authentication failed for sector ");
//                     Serial.println(w_sector);
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     vTaskDelay(200 / portTICK_PERIOD_MS);
//                     break;
//                 }

//                 // Erase the block first, then write data
//                 status = mfrc522.MIFARE_Write(w_blockNum, blockEraser, MAX_BLOCK_BUFFER);
//                 if (status != MFRC522::STATUS_OK)
//                 {
//                     terminal.print("Erase block failed: ");
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     break;
//                 }

//                 status = mfrc522.MIFARE_Write(w_blockNum, itemData[attr], MAX_BLOCK_BUFFER);
//                 if (status != MFRC522::STATUS_OK)
//                 {
//                     terminal.print("Writing to block failed: ");
//                     Serial.println(mfrc522.GetStatusCodeName(status));
//                     break;
//                 }

//                 // Advance to next block
//                 w_block_offset++;
//                 if (w_block_offset >= BLOCK_PER_SECTOR - 1)
//                 {
//                     w_block_offset = 0; // Reset block offset
//                     w_sector++;         // Move to the next sector
//                 }
//             }

//             terminal.println("Data written successfully");
//             // Halt and stop the reader after completing the writing process
//             mfrc522.PICC_HaltA();
//             mfrc522.PCD_StopCrypto1();

//             // Clear the item data and reset sector and offset for the next card
//             for (byte *attrBuffer : itemData)
//             {
//                 memset(attrBuffer, 0, MAX_BLOCK_BUFFER);
//             }
//             w_sector = 0;
//             w_block_offset = 1;
//             cli_state = m_WRITE;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void doCLI(void *parameters)
// {
//     while (1)
//     {
//         switch (cli_state)
//         {
//         case m_WAIT:
//             break;
//         case m_READ:
//             terminal.println("Current mode: READ");
//             terminal.println("Insert your card to start reading...");
//             terminal.flush();
//             cli_state = m_WAIT;
//             mode = READ;
//             break;
//         case m_WRITE:
//             terminal.println("Current mode: WRITE");
//             terminal.println("Enter your data...");
//             terminal.flush();
//             cli_state = m_WAIT;
//             mode = WAIT_WRITE;
//             break;
//         case m_EMPTY_WRITE:
//             terminal.println("Cannot write empty data");
//             terminal.flush();
//             cli_state = m_WAIT;
//             break;
//         case m_INVALID_WRITE:
//             terminal.println("Input is too long");
//             terminal.flush();
//             cli_state = m_WAIT;
//             break;
//         case m_VALID_WRITE:
//             terminal.println("Insert your card to start writing...");
//             terminal.flush();
//             cli_state = m_WAIT;
//             mode = WRITE;
//             break;
//         case m_INVALID_OP:
//             terminal.println("Invalid operation!");
//             terminal.flush();
//             cli_state = m_WAIT;
//             break;
//         case m_CLEAN:
//             terminal.println("Insert your card to start erasing all data...");
//             cli_state = m_WAIT;
//             mode = CLEAN;
//             break;
//         case m_CLEAN_SUCCESS:
//             terminal.println("All data is erased!");
//             cli_state = m_WAIT;
//             break;
//         }

//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void setup()
// {
// #ifdef I2C
//     M5.begin();                   // Initialize M5Stack
//     M5.Power.begin();             // Initialize power module
//     Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C bus, customized to SDA = GPIO11, SCL = GPIO12
// #endif
// #ifdef SPI
//     /*Add SPI initialization*/
//     SPI.begin();
// #endif

//     /* Setup debug console */
//     Serial.begin(115200);
//     ERa.begin(ssid, pass);
//     mfrc522.PCD_Init(); // Initialize MFRC522
//     pinMode(LED_ON_BOARD, OUTPUT);
//     digitalWrite(LED_ON_BOARD, LOW);
//     terminal.begin(terminalCallback);
//     terminal.println("MFRC522 Start");
//     terminal.print("Please choose mode READ or WRITE");
//     terminal.flush();

//     for (byte i = 0; i < 6; i++)
//         key.keyByte[i] = 0xFF;
//     memset(blockEraser, 0, MAX_BLOCK_BUFFER);

//     // xTaskCreatePinnedToCore(dumpData, "Dump Data", 4096, NULL, 1, NULL, app_cpu);
//     // xTaskCreatePinnedToCore(printTextData, "Print Text Data", 4096, NULL, 1, NULL, app_cpu);
//     // xTaskCreatePinnedToCore(clearAllDataBlock, "Clear All Data From Block", 4096, NULL, 1, NULL, app_cpu);
//     // xTaskCreatePinnedToCore(writeRFIDfromForm, "Write RFID", 4096, NULL, 1, NULL, app_cpu);
//     // xTaskCreatePinnedToCore(doCLI, "Do CLI", 4096, NULL, 1, NULL, app_cpu);

//     /* Setup timer called function every second */
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
