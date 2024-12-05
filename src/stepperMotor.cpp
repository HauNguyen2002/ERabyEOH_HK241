#include <stepperMotor.h>

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

int endPoint = 0;
int stepMultiplier = 1;

void rotateClockwise() {
	endPoint += stepMultiplier;
	stepper1.moveTo(endPoint);
}

void rotateAntiClockwise() {
	endPoint -= stepMultiplier;
	stepper1.moveTo(endPoint);
}

void setZeroPosition() {
	endPoint = 0;
	stepper1.setCurrentPosition(endPoint);
}

void stepMultiplierIncrement() {
	if (stepMultiplier >= 4096) {
		return;
	}

	stepMultiplier *= 2;
}

void stepMultiplierDecrement() {
	if (stepMultiplier <= 1) {
		return;
	}

	stepMultiplier /= 2;
}

void runStepper(void* parameter) {
	while (1) {
		stepper1.run();
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}