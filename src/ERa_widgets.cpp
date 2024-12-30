#include "ERa_widgets.h"

const uint8_t terminal_queue_len_EW = 5;

QueueHandle_t terminal_queue_EW = nullptr;

ERaString fromStr_EW;

ERaWidgetTerminalBox terminal_EW(fromStr_EW, V43, V44);

void chatBox_EW(void *parameters)
{
    char terminal_buffer[MAX_CHAT_BOX_MSG_SIZE_EW];
    memset(terminal_buffer, 0, MAX_CHAT_BOX_MSG_SIZE_EW);
    xQueueSend(terminal_queue_EW, "Hello", portMAX_DELAY);
    while (1)
    {
        if (xQueueReceive(terminal_queue_EW, (void *)&terminal_buffer, 0) == pdTRUE)
        {
            terminal_EW.println(terminal_buffer);

            Serial.println(terminal_buffer);
            memset(terminal_buffer, 0, MAX_CHAT_BOX_MSG_SIZE_EW);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void init_terminal_EW()
{
    terminal_EW.begin(terminalCallback_EW);
    terminal_queue_EW = xQueueCreate(terminal_queue_len_EW, MAX_CHAT_BOX_MSG_SIZE_EW);

    xTaskCreatePinnedToCore(chatBox_EW, "Chat box", 4096, NULL, 1, NULL, 1);
}