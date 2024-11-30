#include "credential_store.h"

void init_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

byte storageStatus()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
        return STORAGE_OPEN_FAIL;

    char key[KEY_SIZE];
    size_t required_size = 0;
    byte count = 0;
    for (int user_id = 0; user_id < MAX_USERS; user_id++)
    {
        memset(key, 0, KEY_SIZE);
        sprintf(key, "user_%d", user_id);
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);

        if (err == ESP_OK)
            count++;
        continue;

        if (err != ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(nvs_handle);
            return STORAGE_CHECK_ERROR;
        }
    }
    if (count >= MAX_USERS)
        return STORAGE_FULL;
    if (count == 0)
        return STORAGE_EMPTY;
    return STORAGE_AVAILABLE;
}

esp_err_t dump_user_credentials()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
        return err;

    char key[KEY_SIZE];
    char storedUsername[MAX_DATA_SIZE] = {0};
    char storedPassword[MAX_DATA_SIZE] = {0};
    size_t username_len = MAX_DATA_SIZE;
    size_t password_len = MAX_DATA_SIZE;
    // Retrieve username
    for (int user_id = 0; user_id < MAX_USERS; user_id++)
    {
        memset(storedUsername, 0, MAX_DATA_SIZE);
        memset(storedPassword, 0, MAX_DATA_SIZE);
        memset(key, 0, KEY_SIZE);
        sprintf(key, "user_%d", user_id);
        username_len = MAX_DATA_SIZE;
        err = nvs_get_str(nvs_handle, key, storedUsername, &username_len);
        if (err == ESP_ERR_NVS_NOT_FOUND)
            continue; // Skip if no username is found
        if (err != ESP_OK)
        {
            nvs_close(nvs_handle);
            return err;
        }

        // Retrieve password
        memset(key, 0, KEY_SIZE);
        sprintf(key, "pass_%d", user_id);
        password_len = MAX_DATA_SIZE;
        err = nvs_get_str(nvs_handle, key, storedPassword, &password_len);

        Serial.printf("Storage %d:\n", user_id);
        Serial.printf("Username: %s\n", storedUsername);
        Serial.printf("Password: %s\n", storedPassword);
    }

    nvs_close(nvs_handle);
    Serial.println("Dump successfull!");
    return err;
}

esp_err_t clear_user_credentials()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return err;

    char key[KEY_SIZE];

    // Loop through all possible user IDs and delete their data
    for (int user_id = 0; user_id < MAX_USERS; user_id++)
    {
        // Delete the username
        memset(key, 0, KEY_SIZE);
        sprintf(key, "user_%d", user_id);
        err = nvs_erase_key(nvs_handle, key);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(nvs_handle);
            return err;
        }

        // Delete the password
        memset(key, 0, KEY_SIZE);
        sprintf(key, "pass_%d", user_id);
        err = nvs_erase_key(nvs_handle, key);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(nvs_handle);
            return err;
        }
    }

    // Commit changes to make deletions permanent
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return err;
}

byte save_user_credentials(const char *username, const char *password)
{
    // Serial.println("In save_user_credentials:");
    // Serial.printf("username: %s size: %d\n", username, strlen(username));
    // Serial.printf("password: %s size: %d\n", password, strlen(password));

    char key[KEY_SIZE];
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return -1; // Return false for failed NVS open

    size_t required_size = 0;
    for (byte user_id = 0; user_id < MAX_USERS; user_id++)
    {
        memset(key, 0, KEY_SIZE);
        sprintf(key, "user_%d", user_id);
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);

        if (err == ESP_OK)
            continue;

        if (err != ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(nvs_handle);
            return -1;
        }

        err = nvs_set_str(nvs_handle, key, username);
        if (err != ESP_OK)
        {
            nvs_close(nvs_handle);
            return -1;
        }
        memset(key, 0, KEY_SIZE);
        sprintf(key, "pass_%d", user_id);
        err = nvs_set_str(nvs_handle, key, password);
        if (err != ESP_OK)
        {
            nvs_close(nvs_handle);
            return -1;
        }

        err = nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        return user_id;
    }

    nvs_close(nvs_handle);
    return -1; // No available user slots
}

esp_err_t get_user_credentials(char *username, size_t username_len, char *password, size_t password_len, size_t user_id)
{
    char key[KEY_SIZE];
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
        return err;

    // Retrieve username
    memset(key, 0, KEY_SIZE);
    sprintf(key, "user_%d", user_id);
    err = nvs_get_str(nvs_handle, key, username, &username_len);
    if (err != ESP_OK)
        return err;

    // Retrieve password
    memset(key, 0, KEY_SIZE);
    sprintf(key, "pass_%d", user_id);
    err = nvs_get_str(nvs_handle, key, password, &password_len);

    nvs_close(nvs_handle);
    return err;
}

esp_err_t change_user_password(const char *new_password, size_t user_id)
{
    char key[KEY_SIZE];
    memset(key, 0, KEY_SIZE);
    sprintf(key, "pass_%d", user_id); // Unique key for password of the specified user

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return err;

    // Store new password
    err = nvs_set_str(nvs_handle, key, new_password);
    if (err != ESP_OK)
        return err;

    // Commit changes
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return err;
}

esp_err_t delete_user_credentials(size_t user_id)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return err;

    char key[KEY_SIZE];

    memset(key, 0, KEY_SIZE);
    sprintf(key, "user_%d", user_id);
    err = nvs_erase_key(nvs_handle, key);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(nvs_handle);
        return err;
    }

    // Delete the password
    memset(key, 0, KEY_SIZE);
    sprintf(key, "pass_%d", user_id);
    err = nvs_erase_key(nvs_handle, key);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        nvs_close(nvs_handle);
        return err;
    }

    // Commit changes to make deletions permanent
    err = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    return err;
}

int isStored(const char *username, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        Serial.println("Failed to open NVS storage");
        return -1;
    }

    char storedUsername[MAX_DATA_SIZE] = {0};
    char storedPassword[MAX_DATA_SIZE] = {0};
    size_t username_len = MAX_DATA_SIZE;
    size_t password_len = MAX_DATA_SIZE;
    char key[KEY_SIZE];

    // Iterate through all possible users
    for (byte user_id = 0; user_id < MAX_USERS; user_id++)
    {
        memset(key, 0, KEY_SIZE);
        sprintf(key, "user_%d", user_id);
        username_len = MAX_DATA_SIZE;
        err = nvs_get_str(nvs_handle, key, storedUsername, &username_len);

        if (err != ESP_OK)
            continue; // Skip if retrieval fails
        memset(key, 0, KEY_SIZE);
        sprintf(key, "pass_%d", user_id);
        password_len = MAX_DATA_SIZE;
        err = nvs_get_str(nvs_handle, key, storedPassword, &password_len);

        if (err != ESP_OK)
            continue; // Skip if retrieval fails

        // Compare username and password
        if (strncmp(username, storedUsername, MAX_DATA_SIZE) == 0 &&
            strncmp(password, storedPassword, MAX_DATA_SIZE) == 0)
        {
            nvs_close(nvs_handle);
            return user_id;
        }
    }

    nvs_close(nvs_handle);
    return -1; // No matching credentials found
}

int getNewID()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        Serial.println("Failed to open NVS storage");
        return -1;
    }
    char key[KEY_SIZE];
    size_t required_size = 0;
    for (byte user_id = 0; user_id < MAX_USERS; user_id++)
    {
        snprintf(key, sizeof(key), "user_%d", user_id);
        required_size = 0;
        err = nvs_get_str(nvs_handle, key, NULL, &required_size);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            nvs_close(nvs_handle);
            return user_id;
        }
        else if (err != ESP_OK)
        {
            Serial.printf("Error checking key for ID %d\n", user_id);
            nvs_close(nvs_handle);
            return -1;
        }
    }
    nvs_close(nvs_handle);
    Serial.println("No available IDs found");
    return -1;
}
