#include <Arduino.h>
#include <ERa.hpp>

// Define HC-SR04 pins
#define TRIG_PIN 9   // Pin where the TRIG is connected -- D6
#define ECHO_PIN 10   // Pin where the ECHO is connected -- D7

static long duration = 0;
static long dist = 0;

void TaskDistance(void *parameters);
void init_distance_detection();