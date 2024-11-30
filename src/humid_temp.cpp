// // #include "humid_temp.h"

// // /*--------------------------------------------------*/
// // /*---------------------- Tasks ---------------------*/
// // /*--------------------------------------------------*/

// // void TaskTemperatureHumidity(void *pvParameters) {  // This is a task.

// //   while(1) {                          
// //     Serial.println("Task Temperature and Humidity");

// //     float temperature = dht.readTemperature();
// //     float humidity = dht.readHumidity();

// //     // Serial.println(humidity);
// //     // Serial.println(temperature);

// //     // Check if any reads failed and exit early
// //     if (isnan(temperature) || isnan(humidity)) {
// //       Serial.println("Failed to read from DHT sensor!");
// //     } else {
// //       Serial.print("Humidity: ");
// //       Serial.print(humidity);
// //       Serial.print(" %    ");
// //       Serial.print("Temperature: ");
// //       Serial.print(temperature);
// //       Serial.println(" *C    ");
// //     }
// //     vTaskDelay(5000/portTICK_PERIOD_MS);
// //   }
// // }

// // The setup function runs once when you press reset or power on the board.
// void setup() {

//   // Initialize serial communication at 115200 bits per second:
//   Serial.begin(115200); // Start serial communication
//   dht.begin();          // Initialize the DHT11 sensor

//   // pinMode(TRIG_PIN, OUTPUT); // Set TRIG_PIN as output
//   // pinMode(ECHO_PIN, INPUT);  // Set ECHO_PIN as input

//   Wire.begin(17, 18);   // Initialize I2C with custom SDA and SCL pins


//   // This variant of task creation can also specify on which core it will be run (only relevant for multi-core ESPs)
//   xTaskCreatePinnedToCore(TaskTemperatureHumidity, "Task Temperature Humidity", 2048,NULL,1,NULL, ARDUINO_RUNNING_CORE);
// }