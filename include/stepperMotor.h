// Functions and macro declarations for 28BYJ-48

#include <AccelStepper.h>

#define HALFSTEP 8

#define motorPin1 38
#define motorPin2 21
#define motorPin3 18
#define motorPin4 17

extern AccelStepper stepper1;
extern int endPoint;
extern int stepMultiplier;

void rotateClockwise();
void rotateAntiClockwise();
void setZeroPosition();
void stepMultiplierIncrement();
void stepMultiplierDecrement();
void runStepper(void* parameter);