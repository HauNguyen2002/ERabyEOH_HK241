// Functions and macro declarations for 28BYJ-48

#include <AccelStepper.h>
#include <ERa.hpp>

#define HALFSTEP 8

#define motorPin1 38
#define motorPin2 21
#define motorPin3 18
#define motorPin4 17

static AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
static int endPoint=0;
static int stepMultiplier=1;

void rotateClockwise();
void rotateAntiClockwise();
void setZeroPosition();
void stepMultiplierIncrement();
void stepMultiplierDecrement();
void runStepper(void* parameter);

void init_stepper();