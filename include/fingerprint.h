#ifndef FINGERPRINT_H
#define FINGERPRINT_H
#include "M5Unified.h"
#include "M5_FPC1020A.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <ERa.hpp>
#include <Widgets/ERaWidgets.hpp>
#include "ERa_widgets.h"

#define FPC1020A_TX_PIN 8
#define FPC1020A_RX_PIN 9

#define MAX_ID 150
#define STORAGE_NAMESPACE "fingerprint"

#define MAX_CHAT_BOX_MSG_SIZE 100

enum FINGERPRINT_MODE
{
    F_WAIT = -1,
    VERIFY = 0,
    ADD = 1,
    F_DELETE = 2,
    REPLACE = 3,
    CHANGE_PERMISSION = 4
};

static FINGERPRINT_MODE f_op_mode = F_WAIT;


static uint8_t write_permission = 1;
static uint8_t write_id = 1;
static uint8_t delete_mode;

static bool is_start_op = false;

static const uint8_t permission_queue_len = 5;
static const uint8_t id_queue_len = 5;
static const uint8_t delete_mode_queue_len = 5;

static QueueHandle_t permission_queue;
static QueueHandle_t id_queue;
static QueueHandle_t delete_mode_queue;

class Fingerprint_FPC1020A
{
private:
    bool addUserProcess(uint8_t id, uint8_t permission);

    void init_nvs();

    esp_err_t add_to_smallest_id(uint8_t *assigned_id);
    esp_err_t delete_fingerprint_id(uint8_t id);

    esp_err_t delete_all_ids();

public:
    void reset_storage();
    bool verifyFingerprint(uint8_t &id, uint8_t &permission);

    bool clear();
    bool deleteAt(uint8_t &id);

    bool deleteFinger();
    bool replace(uint8_t id, uint8_t permission);
    bool setPermission(uint8_t permission);

    void processAdd();
    void processVerify();

    void processDelete();
    void processReplace();
    void processChangePermission();
};


void FingerprintOperationCenter(void *parameters);

void init_FPC1020A();


#endif // FINGERPRINT_H