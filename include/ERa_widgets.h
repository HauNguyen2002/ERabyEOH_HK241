#include <ERa.hpp>
#include <Widgets/ERaWidgets.hpp>

#define MAX_CHAT_BOX_MSG_SIZE_EW 80

extern const uint8_t terminal_queue_len_EW;
extern QueueHandle_t terminal_queue_EW;
extern ERaString fromStr_EW;
extern ERaWidgetTerminalBox terminal_EW;

void terminalCallback_EW();

void chatBox_EW(void *parameters);

void init_terminal_EW();