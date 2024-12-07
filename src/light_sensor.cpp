#include "light_sensor.h"


void init_light_level(){
    Wire1.begin(SDA_PIN_LIGHT, SCL_PIN_LIGHT);
    lightSensor.begin(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1);
}

void TaskLightLevel(void *pvParameters)
{ // This is a task.

    while (1)
    {
        Serial.print("Task Light Level: ");

        // Read light level from BH1750
        lux = lightSensor.readLightLevel(); // Read light level

        if (lux < 0)
        {
            Serial.println("Failed to read from BH1750!");
        }
        else
        {
            // Serial.print("Light Level: ");
            Serial.print(lux);
            Serial.println(" lx");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 100ms delay
    }
}