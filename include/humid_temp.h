#include <ERa.hpp>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 5     // Pin where the DHT11 is connected -- D2
#define DHTTYPE DHT11  // Define the type of sensor


static DHT dht(DHTPIN, DHTTYPE); // Create a DHT object

void TaskTemperatureHumidity(void *pvParameters);

void init_humid_temp();

