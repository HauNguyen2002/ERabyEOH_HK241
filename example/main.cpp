#define ERA_LOCATION_VN
#define ERA_AUTH_TOKEN "d0ca164a-fefe-49c8-a17d-93e960134a5f" // Must be on top

#include <Arduino.h>
#include <ERa.hpp>
#include "fingerprint.h"
#include "mifare_classic_1k_rfid.h"
#include "ERa_widgets.h"
// GLOBALS
const char ssid[] = "Guess who?";
const char pass[] = "yourmama";


/*
Add these in "ERaSimpleEsp32.hpp"

#define NO_GLOBAL_INSTANCES
#define NO_GLOBAL_ERA
*/
static ERaFlash flash;
static WiFiClient ERaWiFiClient;
static ERaMqtt<WiFiClient, MQTTClient> mqtt(ERaWiFiClient);
ERaPnP<ERaMqtt<WiFiClient, MQTTClient>> ERa(mqtt, flash);

void init_common()
{
    Serial.begin(115200);
    ERa.begin(ssid, pass);
}

void setup()
{
    init_common();
    init_terminal_EW();

    init_mifare_1k_mfrc522();

    init_FPC1020A();
}
void loop()
{
    ERa.run();
}