#include "distance_detection.h"

void init_distance_detection()
{
    pinMode(TRIG_PIN, OUTPUT); // Set TRIG_PIN as output
    pinMode(ECHO_PIN, INPUT);  // Set ECHO_PIN as input
}

void TaskDistance(void *parameters)
{ // This is a task.

    while (1)
    {
        Serial.println("Task Distance");

        // HC-SR04 distance measurement

        // Clear the TRIG_PIN
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);

        // Set the TRIG_PIN HIGH for 10 microseconds
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        // Read the ECHO_PIN
        duration = pulseIn(ECHO_PIN, HIGH);

        // Calculate the distance (duration/2) * speed of sound (34300 cm/s)
        dist = (duration / 2) * 0.0343;

        // Print the distance in centimeters
        // Serial.print("Distance: ");
        Serial.print(dist);
        Serial.println(" cm    ");
        ERa.virtualWrite(V3, dist);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // 100ms delay
    }
}