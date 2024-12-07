#include <stepperMotor.h>

ERA_WRITE(V24) // Rotate Camera anti-Clockwise
{
    int value = param.getInt();

    if (value == 24)
    {
        rotateAntiClockwise();

        Serial.print("\n The stepper's current position: ");
        Serial.print(stepper1.currentPosition());
        Serial.print("\n");
    }
}

ERA_WRITE(V25) // Rotate Camera Clockwise
{
    int value = param.getInt();

    if (value == 25)
    {
        rotateClockwise();

        Serial.print("\n The stepper's current position: ");
        Serial.print(stepper1.currentPosition());
        Serial.print("\n");
    }
}

ERA_WRITE(V26) // Set the initial zero position of the camera
{
    int value = param.getInt();

    if (value == 26)
    {
        setZeroPosition();

        Serial.print("\n The initial zero position have been set!");
        Serial.print("\n The stepper's current position: ");
        Serial.print(stepper1.currentPosition());
        Serial.print("\n");
    }
}

ERA_WRITE(V27)
{
    int value = param.getInt();

    if (value == 27)
    {
        stepMultiplierIncrement();

        Serial.print("\n The step multiplier has been incremented");
        Serial.print("\n Current step multiplier: ");
        Serial.print(stepMultiplier);
        Serial.print("\n");
    }
}

ERA_WRITE(V28)
{
    int value = param.getInt();

    if (value == 28)
    {
        stepMultiplierDecrement();

        Serial.print("\n The step multiplier has been decremented");
        Serial.print("\n Current step multiplier: ");
        Serial.print(stepMultiplier);
        Serial.print("\n");
    }
}

void init_stepper()
{
    stepper1.setMaxSpeed(1000.0);
    stepper1.setAcceleration(100.0);
    stepper1.setSpeed(200);
}

void rotateClockwise()
{
    endPoint += stepMultiplier;
    stepper1.moveTo(endPoint);
}

void rotateAntiClockwise()
{
    endPoint -= stepMultiplier;
    stepper1.moveTo(endPoint);
}

void setZeroPosition()
{
    endPoint = 0;
    stepper1.setCurrentPosition(endPoint);
}

void stepMultiplierIncrement()
{
    if (stepMultiplier >= 4096)
    {
        return;
    }

    stepMultiplier *= 2;
}

void stepMultiplierDecrement()
{
    if (stepMultiplier <= 1)
    {
        return;
    }

    stepMultiplier /= 2;
}

void runStepper(void *parameter)
{
    while (1)
    {
        stepper1.run();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}