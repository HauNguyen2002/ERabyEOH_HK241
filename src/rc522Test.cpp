// /*
// *******************************************************************************
// * Copyright (c) 2023 by M5Stack
// *                  Equipped with M5Core sample source code
// *                          配套  M5Core 示例源代码
// * Visit for more information: https://docs.m5stack.com/en/core/gray
// * 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/gray
// *
// * Describe: RFID.
// * Date: 2021/8/19
// *******************************************************************************
//   Please connect to Port A (22, 21) and use the RFID Unit to read the Fudan card ID.
// */

// #include <M5Unified.h>
// #include "MFRC522_I2C.h"
// #include "freertos/FreeRTOS.h"

// // Use only core 1 for demo purposes
// #if CONFIG_FREERTOS_UNICORE
// static const BaseType_t app_cpu = 0;
// #else
// static const BaseType_t app_cpu = 1;
// #endif

// // USER DEFINES
// #define MAX_BLOCK_BUFFER 16
// // Globals
// static MFRC522 mfrc522(0x28); // Create MFRC522 instance with I2C address 0x28, comment it out if using SPI/UART 

// static byte blockData[MAX_BLOCK_BUFFER];
// static int blockNum = 2;
// static byte status;
// enum OpMode
// {
//     READ,
//     WRITE,
//     WAIT,
//     ADD_MSG
// };
// static enum OpMode mode = WAIT;

// void readUID(void *parameters)
// {
//     while (1)
//     {
//         if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//         {
//             vTaskDelay(200 / portTICK_PERIOD_MS); // Delay to prevent spamming when no card is present
//             continue;
//         }

//         // Print UID to Serial
//         Serial.print("UID: ");
//         for (byte i = 0; i < mfrc522.uid.size; i++)
//         {
//             Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//             Serial.print(mfrc522.uid.uidByte[i], HEX);
//         }
//         Serial.println();

//         vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for readability before next scan
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
//             mode = WAIT;
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void writeRFID(void *parameters)
// {
//     MFRC522::MIFARE_Key key;
//     for (byte i = 0; i < 6; i++)
//         key.keyByte[i] = 0xFF;

//     while (1)
//     {
//         if (mode == WRITE)
//         {
//             if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
//             {
//                 vTaskDelay(200 / portTICK_PERIOD_MS); // Wait if no card is present
//                 continue;
//             }

//             // Reset the removal flag as we start writing

//             status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

//             if (status != MFRC522::STATUS_OK)
//             {
//                 Serial.print("Authentication failed for Write: ");
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS);
//                 continue;
//             }

//             Serial.println("Authentication success");

//             status = mfrc522.MIFARE_Write(blockNum, blockData, MAX_BLOCK_BUFFER);

//             if (status != MFRC522::STATUS_OK)
//             {
//                 Serial.print("Writing to Block failed: ");
//                 Serial.println(mfrc522.GetStatusCodeName(status));
//                 vTaskDelay(200 / portTICK_PERIOD_MS);
//                 continue;
//             }

//             Serial.println("Data was written into Block successfully");
//             memset(blockData, 0, MAX_BLOCK_BUFFER);
//             mode = WAIT;

//             // Send the card to a halt state and wait for it to be removed
//             mfrc522.PICC_HaltA();
//             mfrc522.PCD_StopCrypto1();

//             // Wait for card removal before next operation

//             // Set the card removal flag so the task knows a new card is required
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void doCLI(void *parameters)
// {
//     char c;
//     while (1)
//     {
//         if (mode == WAIT)
//         {
//             Serial.println("Choose mode:");
//             Serial.println("READ: 1");
//             Serial.println("WRITE: 2");
//             while (Serial.available() > 0)
//                 Serial.read();
//             while (Serial.available() <= 0)
//                 ;
//             c = Serial.read();
//             if (c == '1')
//             {
//                 Serial.println("Please insert the card to dump");
//                 mode = READ;
//             }
//             else if (c == '2')
//             {
//                 Serial.println("Please enter data to write");
//                 mode = ADD_MSG;
//             }
//             Serial.print("Current mode: ");
//             Serial.println(mode);
//         }
//         else
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void processMsg(void *parameters)
// {
//     int idx = 0;
//     char c;
//     memset(blockData, 0, MAX_BLOCK_BUFFER);
//     while (1)
//     {
//         if (mode == ADD_MSG)
//         {
//             if (Serial.available() > 0)
//             {
//                 c = Serial.read();
//                 Serial.print(c);
//                 if (c == '\n' || c == '\r')
//                 {
//                     if (idx > 0)
//                     {
//                         Serial.println("Please insert card to start writing");
//                         mode = WRITE;
//                         idx = 0;
//                     }
//                     else
//                     {
//                         Serial.println("Cannot write empty data!");
//                         while (Serial.available() > 0) Serial.read();
//                     }
//                 }
//                 else if (idx < MAX_BLOCK_BUFFER)
//                     blockData[idx++] = c;
//             }
//         }
//         else
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }

// void setup()
// {
//     M5.begin();           // Initialize M5Stack
//     M5.Power.begin();     // Initialize power module
//     Wire.begin(11, 12);   // Initialize I2C bus


//     Serial.begin(115200); // Initialize Serial for output
    
//     mfrc522.PCD_Init(); // Initialize MFRC522
//     vTaskDelay(6000 / portTICK_PERIOD_MS);
//     Serial.println("MFRC522 Test");

//     xTaskCreatePinnedToCore(doCLI, "Do CLI", 2048, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(dumpData, "Dump Data", 2048, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(writeRFID, "Write RFID", 2048, NULL, 1, NULL, app_cpu);
//     xTaskCreatePinnedToCore(processMsg, "Process Message", 2048, NULL, 1, NULL, app_cpu);
// }

// void loop()
// {
// }
