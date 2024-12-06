#ifndef CREDENTIAL_STORE_H
#define CREDENTIAL_STORE_H

#include <Arduino.h>
#include <nvs_flash.h>
#include <nvs.h>

#define MAX_USERS       10
#define MAX_DATA_SIZE   17
#define KEY_SIZE        20
void init_nvs();


enum StorageStatus{
    STORAGE_OPEN_FAIL,
    STORAGE_CHECK_ERROR,
    STORAGE_FULL,
    STORAGE_EMPTY,
    STORAGE_AVAILABLE
};


esp_err_t dump_user_credentials();

esp_err_t clear_user_credentials();

byte save_user_credentials(const char* username, const char* password);

esp_err_t get_user_credentials(char* username, size_t username_len, char* password, size_t password_len, size_t user_id);

esp_err_t change_user_password(const char* new_password,size_t user_id);

esp_err_t delete_user_credentials(size_t user_id);

int isStored(const char *username, const char *password);

byte storageStatus();
int getNewID();














#endif //CREDENTIAL_STORE_H