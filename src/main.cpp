#define ERA_LOCATION_VN
// #define ERA_AUTH_TOKEN "1b2f6d4f-0330-43bb-9ac8-8b31ce65c61f" // Must be on top
#define ERA_AUTH_TOKEN "d0ca164a-fefe-49c8-a17d-93e960134a5f" // Must be on top

#include <Arduino.h>
#include <ERa.hpp>
#include "stepperMotor.h"
#include "humid_temp.h"
#include "light_sensor.h"
#include "authentication.h"
#include "distance_detection.h"

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// GLOBALS
const char ssid[] = "Guess who?";
const char pass[] = "yourmama";
// const char ssid[] = "THANHHAU";
// const char pass[] = "30110902";


static ERaFlash flash;
static WiFiClient ERaWiFiClient;
static ERaMqtt<WiFiClient, MQTTClient> mqtt(ERaWiFiClient);
ERaPnP<ERaMqtt<WiFiClient, MQTTClient>> ERa(mqtt, flash);

void setup()
{
    /* Setup debug console */
    Serial.begin(115200);

    init_door_authentication();

    init_humid_temp();

    init_light_level();
    
    init_distance_detection();
    
    init_stepper();

    ERa.begin(ssid, pass);

    pinMode(LED_ON_BOARD, OUTPUT);

    digitalWrite(LED_ON_BOARD, LOW);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // xTaskCreatePinnedToCore(dumpData, "Dump Data", 4096, NULL, 1, NULL, app_cpu);
    xTaskCreatePinnedToCore(doCLI, "Do CLI", 4096, NULL, 1, NULL, app_cpu);
    xTaskCreatePinnedToCore(DoorAuthenticationTaskCenter, "Door Authentication Task Center", 4096, NULL, 1, NULL, app_cpu);
    // xTaskCreatePinnedToCore(TaskTemperatureHumidity, "Task Temperature Humidity", 4096, NULL, 1, NULL, app_cpu);
    // xTaskCreatePinnedToCore(TaskDistance, "Task Distance", 4096, NULL, 1, NULL, app_cpu);
    // xTaskCreatePinnedToCore(TaskLightLevel, "Task Light Level", 2048, NULL, 1, NULL, app_cpu);
    // xTaskCreatePinnedToCore(runStepper, "Run Stepper Motor", 4096, NULL, 1, NULL, app_cpu);

    /* Setup  called function every second */
}
void loop()
{
    ERa.run();
}