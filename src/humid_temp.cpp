#include "humid_temp.h"

void init_humid_temp()
{
    dht.begin();
}

void TaskTemperatureHumidity(void *pvParameters)
{
    while (1)
    {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
        }
        else
        {
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.print(" %    ");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" *C    ");
            // Use ERa defined in authentication.cpp
            ERa.virtualWrite(V0, temperature, V1, humidity);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
