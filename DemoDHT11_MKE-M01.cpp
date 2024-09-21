
/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
// #define ERA_DEBUG

#define DEFAULT_MQTT_HOST "mqtt1.eoh.io"

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "1b2f6d4f-0330-43bb-9ac8-8b31ce65c61f"

#define LED_BOARD 48
#define LED_GREEN 1

#define DHT11Pin 2
#define DHTType DHT11

#include <Arduino.h>
#include <DHT.h>
#include <ERa.hpp>
#include <ERa/ERaTimer.hpp>

const char ssid[] = "THANHHAU";
const char pass[] = "30110902";

DHT HT(DHT11Pin, DHTType);
float humi;
float tempC;

char msg[50];

ERaTimer timer;

/* This function print uptime every second */
void timerEvent() {
    ERA_LOG("Timer", "Uptime: %d", ERaMillis() / 1000L);
    humi=HT.readHumidity();
    tempC=HT.readTemperature();
    sprintf(msg, "Temp (C): %.2f, Humi: %.2f",tempC, humi);
    ERa.virtualWrite(V1,msg,V2,humi,V3,tempC);
}

ERA_WRITE(V0){
  int value = param.getInt();
  digitalWrite(LED_BOARD,value);
  digitalWrite(LED_GREEN,value);
  
}

void setup() {
    /* Setup debug console */
    Serial.begin(115200);
    HT.begin();
    ERa.begin(ssid, pass);
    pinMode(LED_BOARD,OUTPUT);
    pinMode(LED_GREEN,OUTPUT);

    /* Setup timer called function every second */
    timer.setInterval(1000L, timerEvent);
    
}

void loop() {
    ERa.run();
    timer.run();
}


