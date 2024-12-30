#include "fingerprint.h"

M5_FPC1020A finger;

Fingerprint_FPC1020A fingerprint;

bool Fingerprint_FPC1020A::addUserProcess(uint8_t id, uint8_t permission)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        while (!finger.addFinger(id, permission, i))
        {
            xQueueSend(terminal_queue_EW, "Please put your finger on the sensor", 0);
        }
    }
    return true;
}
void Fingerprint_FPC1020A::init_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
esp_err_t Fingerprint_FPC1020A::add_to_smallest_id(uint8_t *assigned_id)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    char key[8];
    uint8_t id_status;

    // Find the smallest unused ID
    for (uint8_t id = 1; id <= MAX_ID; id++)
    {
        snprintf(key, sizeof(key), "ID_%03d", id);
        ret = nvs_get_u8(handle, key, &id_status);
        if (ret == ESP_ERR_NVS_NOT_FOUND)
        {
            // ID is unused, save it
            ret = nvs_set_u8(handle, key, 1); // Use '1' to mark the ID as used
            if (ret == ESP_OK)
            {
                ret = nvs_commit(handle);
                if (ret == ESP_OK)
                {
                    *assigned_id = id; // Return the assigned ID
                }
            }
            nvs_close(handle);
            return ret;
        }
    }

    // If no unused ID is found
    nvs_close(handle);
    return ESP_ERR_NO_MEM; // Indicate no available ID
}
esp_err_t Fingerprint_FPC1020A::delete_fingerprint_id(uint8_t id)
{
    if (id > MAX_ID)
        return ESP_ERR_INVALID_ARG;

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    char key[8];
    snprintf(key, sizeof(key), "ID_%03d", id);
    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK)
        ret = nvs_commit(handle);

    nvs_close(handle);
    return ret;
}
esp_err_t Fingerprint_FPC1020A::delete_all_ids()
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK)
        return ret;

    char key[8];

    // Iterate through all possible IDs
    for (uint8_t id = 0; id <= MAX_ID; id++)
    {
        snprintf(key, sizeof(key), "ID_%03d", id);
        ret = nvs_erase_key(handle, key); // Attempt to delete the key
        if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(handle); // Close NVS handle if error occurs
            return ret;
        }
    }

    // Commit the changes to storage
    ret = nvs_commit(handle);
    nvs_close(handle);
    return ret;
}
void Fingerprint_FPC1020A::reset_storage()
{
    delete_all_ids();
}
bool Fingerprint_FPC1020A::verifyFingerprint(uint8_t &id, uint8_t &permission)
{
    // TODO
    if (finger.available() == ACK_SUCCESS)
    {
        id = finger.getFingerID();
        permission = finger.getFingerPermission();
        Serial.printf("User ID: %d, User Permission: %d\n", id, permission);
        return true;
    }
    return false;
}
bool Fingerprint_FPC1020A::clear()
{
    // TODO
    for (int i = 0; i < 3; i++)
    {
        if (finger.delAllFinger())
        {
            Serial.println("All fingerprints deleted successfully");
            return true;
        }
    }

    Serial.println("Deleted failed");
    return false;
}
bool Fingerprint_FPC1020A::deleteAt(uint8_t &id)
{
    // TODO
    for (int i = 0; i < 3; i++)
    {
        if (finger.delFinger(id))
        {
            Serial.printf("Fingerprint at ID: %d deleted successfully\n", id);
            return true;
        }
    }

    Serial.println("Deleted failed");
    return false;
}
bool Fingerprint_FPC1020A::deleteFinger()
{
    if (!finger.available() == ACK_SUCCESS)
    {
        return false;
    }
    uint8_t id = finger.getFingerID();
    while (finger.available() == ACK_SUCCESS)
        Serial.println("Release");
    if (finger.delFinger(id))
    {
        delete_fingerprint_id(id);
        return true;
    }
    return false;
}
bool Fingerprint_FPC1020A::replace(uint8_t id, uint8_t permission)
{
    // TODO
    if(!deleteAt(id)) return false;
    if (addUserProcess(id, permission))
    {
        Serial.println("Fingerprint replaced successfully!");
        return true;
    }
    return false;
}
bool Fingerprint_FPC1020A::setPermission(uint8_t permission)
{
    // TODO
    for (int i = 0; i < 3; i++)
    {
        if (finger.available() == ACK_SUCCESS)
            break;
        if (i == 2)
            return false;
    }

    uint8_t id = finger.getFingerID();
    while (finger.available() == ACK_SUCCESS)
        Serial.println("Release");
    if (!deleteAt(id))
        return false;
    if (addUserProcess(id, permission))
    {
        Serial.println("Permission level changed successfully!");
        Serial.printf("Current permission level: %d\n", finger.getFingerPermission());
        return true;
    }

    return false;
}
void Fingerprint_FPC1020A::processAdd()
{
    xQueueSend(terminal_queue_EW, "Current fingerprint mode: Add", 0);
    xQueueSend(terminal_queue_EW, "Please enter the permission level", 0);
    xQueueReceive(permission_queue, &write_permission, portMAX_DELAY);
    char add_msg[50];
    uint8_t id;
    esp_err_t status = add_to_smallest_id(&id);
    sprintf(add_msg, "Adding fingerprint to ID: %d", id);
    xQueueSend(terminal_queue_EW, add_msg, 0);
    memset(add_msg, 0, 50);
    if (status != ESP_OK)
    {
        xQueueSend(terminal_queue_EW, "Failed to find unused ID", 0);
        return;
    }

    xQueueSend(terminal_queue_EW, "Insert your finger....", 0);

    if (addUserProcess(id, write_permission))
    {
        sprintf(add_msg, "Finger added at ID: %d successfully", id);
    }
    else
    {
        sprintf(add_msg, "Finger added at ID: %d failed", id);
    }
    xQueueSend(terminal_queue_EW, add_msg, 0);
}
void Fingerprint_FPC1020A::processVerify()
{
    xQueueSend(terminal_queue_EW, "Current fingerprint mode: Verify", 0);
    xQueueSend(terminal_queue_EW, "Insert your finger and press Start", 0);
    while (!is_start_op)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    is_start_op = false;
    bool verify_status = false;
    xQueueSend(terminal_queue_EW, "Please wait...", 0);

    for (uint8_t i = 0; i < 3; i++)
    {
        verify_status = verifyFingerprint(write_id, write_permission);
        if (verify_status)
            break;
    }
    if (verify_status)
    {
        char verify_msg[50];
        sprintf(verify_msg, "Fingerprint has ID: %d, Permission level: %d", write_id, write_permission);
        xQueueSend(terminal_queue_EW, verify_msg, 0);
    }
    else
        xQueueSend(terminal_queue_EW, "Fingerprint is not recognized", 0);
}
void Fingerprint_FPC1020A::processDelete()
{
    xQueueSend(terminal_queue_EW, "Current fingerprint mode: Delete", 0);
    xQueueSend(terminal_queue_EW, "Choose delete mode:", 0);
    xQueueSend(terminal_queue_EW, "1: Delete fingerprint at ID", 0);
    xQueueSend(terminal_queue_EW, "2: Delete all fingerprint", 0);
    delete_mode = 0;
    xQueueReceive(delete_mode_queue, &delete_mode, portMAX_DELAY);
    char delete_msg[50];
    switch (delete_mode)
    {
    case 1:
        xQueueSend(terminal_queue_EW, "Insert your finger and press Start", 0);
        while (!is_start_op)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        is_start_op = false;
        write_id = deleteFinger();
        if (write_id > 0)
            sprintf(delete_msg, "Fingerprint at ID: %d is deleted", write_id);
        else
            sprintf(delete_msg, "Input Fingerprint deleted failed");
        xQueueSend(terminal_queue_EW, delete_msg, 0);

        break;
    case 2:
        xQueueSend(terminal_queue_EW, "Enter ID:", 0);
        xQueueReceive(id_queue, &write_id, portMAX_DELAY);
        xQueueSend(terminal_queue_EW, "Please wait...", 0);

        if (deleteAt(write_id))
        {
            delete_fingerprint_id(write_id);
            sprintf(delete_msg, "Fingerprint at ID: %d is deleted", write_id);
        }
        else
            sprintf(delete_msg, "Delete Fingerprint at ID: %d failed", write_id);
        xQueueSend(terminal_queue_EW, delete_msg, 0);
        break;
    case 3:
        xQueueSend(terminal_queue_EW, "Please wait...", 0);

        if (clear())
        {
            xQueueSend(terminal_queue_EW, "All fingerprints are deleted successfully", 0);
            delete_all_ids();
        }
        else
            xQueueSend(terminal_queue_EW, "All fingerprints are deleted failed", 0);
        break;
    }
}
void Fingerprint_FPC1020A::processReplace()
{
    xQueueSend(terminal_queue_EW, "Current fingerprint mode: Replace", 0);
    xQueueSend(terminal_queue_EW, "Enter ID:", 0);
    xQueueReceive(id_queue, &write_id, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Enter Permission Level:", 0);
    xQueueReceive(permission_queue, &write_permission, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Insert your finger and press Start", 0);
    while (!is_start_op)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    is_start_op = false;
    xQueueSend(terminal_queue_EW, "Please wait...", 0);

    if (replace(write_id, write_permission))
        xQueueSend(terminal_queue_EW, "fingerprint is replaced successfully", 0);
    else
        xQueueSend(terminal_queue_EW, "fingerprint is replaced failed", 0);
}
void Fingerprint_FPC1020A::processChangePermission()
{
    xQueueSend(terminal_queue_EW, "Current fingerprint mode: Change Permission Level", 0);
    xQueueSend(terminal_queue_EW, "Enter Permission Level:", 0);
    xQueueReceive(permission_queue, &write_permission, portMAX_DELAY);
    xQueueSend(terminal_queue_EW, "Insert your finger and press Start", 0);
    while (!is_start_op)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    is_start_op = false;
    xQueueSend(terminal_queue_EW, "Please wait...", 0);

    if (setPermission(write_permission))
        xQueueSend(terminal_queue_EW, "Permission level changed successfully", 0);
    else
        xQueueSend(terminal_queue_EW, "Permission level changed failed", 0);
}

void FingerprintOperationCenter(void *parameters)
{
    while (1)
    {
        while (uxQueueMessagesWaiting(permission_queue) > 0)
        {
            void *dummy_item;
            xQueueReceive(permission_queue, &dummy_item, 0);
        }
        while (uxQueueMessagesWaiting(id_queue) > 0)
        {
            void *dummy_item;
            xQueueReceive(id_queue, &dummy_item, 0);
        }
        switch (f_op_mode)
        {
        case F_WAIT:
            break;
        case VERIFY:
            fingerprint.processVerify();
            f_op_mode = F_WAIT;
            break;
        case ADD:
            fingerprint.processAdd();
            f_op_mode = F_WAIT;
            break;
        case F_DELETE:
            fingerprint.processDelete();
            f_op_mode = F_WAIT;
            break;
        case REPLACE:
            fingerprint.processReplace();
            f_op_mode = F_WAIT;
            break;
        case CHANGE_PERMISSION:
            fingerprint.processChangePermission();
            f_op_mode = F_WAIT;
            break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void init_FPC1020A()
{
    while (!finger.begin(&Serial1, FPC1020A_TX_PIN, FPC1020A_RX_PIN, 19200))
    {
        Serial.println("FPC1020A not found");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    id_queue = xQueueCreate(id_queue_len, sizeof(uint8_t));
    permission_queue = xQueueCreate(permission_queue_len, sizeof(uint8_t));
    delete_mode_queue = xQueueCreate(delete_mode_queue_len, sizeof(uint8_t));

    finger.setFingerMode(FINGER_NOT_REPEAT);
    finger.delAllFinger();

    fingerprint.reset_storage();

    xTaskCreatePinnedToCore(FingerprintOperationCenter, "MFRC522 Operation Center", 4096, NULL, 1, NULL, 1);
}

ERA_WRITE(V48)
{
    uint8_t mode = param.getInt();
    switch (mode)
    {
    case 0:
        f_op_mode = VERIFY;
        break;
    case 1:
        f_op_mode = ADD;
        break;
    case 2:
        f_op_mode = F_DELETE;
        break;
    case 3:
        f_op_mode = REPLACE;
        break;
    case 4:
        f_op_mode = CHANGE_PERMISSION;
        break;
    }
}
ERA_WRITE(V49)
{
    uint8_t id_value = param.getInt();
    xQueueSend(id_queue, &id_value, portMAX_DELAY);
}
ERA_WRITE(V50)
{
    uint8_t permission_value = param.getInt();
    xQueueSend(permission_queue, &permission_value, portMAX_DELAY);
}
ERA_WRITE(V53)
{
    uint8_t start = param.getInt();
    if (start == 1)
        is_start_op = true;
}
ERA_WRITE(V54)
{
    uint8_t delete_value = param.getInt();
    xQueueSend(delete_mode_queue, &delete_value, portMAX_DELAY);
}