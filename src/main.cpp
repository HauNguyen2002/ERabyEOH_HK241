


// /* Select ERa host location (VN: Viet Nam, SG: Singapore) */
// #define ERA_LOCATION_VN

// // You should get Auth Token in the ERa App or ERa Dashboard
// #define ERA_AUTH_TOKEN "1b2f6d4f-0330-43bb-9ac8-8b31ce65c61f"


// #include <Arduino.h>
// #include <ERa.hpp>

// const char ssid[] = "THANHHAU";
// const char pass[] = "30110902";


// #define LED_ON_BOARD 48
// /* This function will run every time ERa is connected */

// ERA_WRITE(V41)
// {
//     int value = param.getInt();
//     digitalWrite(LED_ON_BOARD, value);
// }


// void setup() {

//     /* Initializing the ERa library. */
//     ERa.begin(ssid, pass);
//     pinMode(LED_ON_BOARD, OUTPUT);

//     /* Setup timer called function every second */
// }

// void loop() {
//     ERa.run();
// }