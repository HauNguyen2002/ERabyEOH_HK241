#include <BH1750.h>

#define SDA_PIN_LIGHT 6 // -- D3
#define SCL_PIN_LIGHT 7 // -- D4


static BH1750 lightSensor;

static float lux = 0;

void TaskLightLevel(void *pvParameters);

void init_light_level();