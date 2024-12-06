// #include <Arduino.h>
// #include <Adafruit_Sensor.h>
// #include <BH1750.h>

// long lux = 0;
// BH1750 lightSensor;
// void setup()
// {
//     Serial.begin(115200);
//     // Wire.begin(11, 12);
//     Wire1.begin(47, 48);

//     lightSensor.begin(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1);
// }
// void loop()
// {
//     // Read light level from BH1750
//     lux = lightSensor.readLightLevel(); // Read light level

//     if (lux < 0)
//     {
//         Serial.println("Failed to read from BH1750!");
//     }
//     else
//     {
//         // Serial.print("Light Level: ");
//         Serial.print(lux);
//         Serial.println(" lx");
//     }
//     delay(1000);
// }
