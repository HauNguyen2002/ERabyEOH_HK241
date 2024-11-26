#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <freeRTOS/FreeRTOS.h>
#include <nvs_flash.h>
#include <nvs.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#define MAX_BLOCK_BUFFER 16
#define LED_ON_BOARD 48
#define NUM_INFO_TYPE 2
#define BLOCK_PER_SECTOR 4
#define MAX_SECTOR 16

#endif //GLOBAL_H