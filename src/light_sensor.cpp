// #include "light_sensor.h"

// BH1750 lightSensor;

// void TaskLightLevel(void *pvParameters)
// { // This is a task.

//     while (1)
//     {
//         Serial.print("Task Light Level: ");

//         // Read light level from BH1750
//         lux = lightSensor.readLightLevel(); // Read light level

//         if (lux < 0)
//         {
//             Serial.println("Failed to read from BH1750!");
//         }
//         else
//         {
//             // Serial.print("Light Level: ");
//             Serial.print(lux);
//             Serial.println(" lx");
//         }
//         vTaskDelay(1000 / portTICK_PERIOD_MS); // 100ms delay
//     }
// }