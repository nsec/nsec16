#include "esp_log.h"
#include "cryptoauthlib.h"
#include "crypto_operations.h"

static const char *TAG = "crypto_atecc_init";

void crypto_atecc_init() {
    int ret = 0;
    bool lock = false;
    ESP_LOGI(TAG, "Initializing ATECC...");
    // CUSTOM: 0xC0 TNGTLS: 0x6A
    cfg_ateccx08a_i2c_default.atcai2c.address = 0xC0;
    cfg_ateccx08a_i2c_default.rx_retries = 1;
    ret = atcab_init(&cfg_ateccx08a_i2c_default);
    // if (ret != ATCA_SUCCESS) {
    //     ESP_LOGI(TAG, "ATECC CUSTOM not detected on 0xC0, trying TNGTLS on 0x6A...");
    //     cfg_ateccx08a_i2c_default.atcai2c.address = 0x6A;
    //     ret = atcab_init(&cfg_ateccx08a_i2c_default);
    // }
    if (ret != ATCA_SUCCESS) {
        ESP_LOGE(TAG, "Fail! atcab_init returned %02x", ret);
    } else {
        ret = atcab_is_locked(LOCK_ZONE_DATA, &lock);
        if (ret != ATCA_SUCCESS) {
            ESP_LOGE(TAG, "Fail! atcab_is_locked returned %02x", ret);
        } else {
            if (!lock) {
                ESP_LOGI(TAG, "LOCK_ZONE_DATA Unlocked!");
                //ESP_LOGE(TAG, "LOCK_ZONE_DATA Unlocked, please lock(configure) the ATECC608 chip with help of esp_cryptoauth_utility and try again");
                //return;
            } else {
                ESP_LOGI(TAG, "LOCK_ZONE_DATA Locked.");
            }
            atcab_is_config_locked(&lock);
            if (!lock) {
                ESP_LOGI(TAG, "LOCK_ZONE_CONFIG Unlocked!");
                //ESP_LOGE(TAG, "LOCK_ZONE_CONFIG Unlocked, please lock(configure) the ATECC608 chip with help of esp_cryptoauth_utility and try again");
                //return;
            } else {
                ESP_LOGI(TAG, "LOCK_ZONE_CONFIG Locked.");
            }
            uint8_t revision[4];
            ret = atcab_info(revision);
            if (ret != ATCA_SUCCESS) {
                ESP_LOGE(TAG, "Fail! atcab_info returned %02x", ret);
            } else {
                if (atcab_get_device_type() != ATECC608B) {
                    ESP_LOGI(TAG, "Wrong chip detected :(");
                } else {
                    ESP_LOGI(TAG, "ATECC608B detected; Device revision:  %02X%02X", revision[2], revision[3]);
                    uint8_t serial_number[ATCA_SERIAL_NUM_SIZE];
                    if (atcab_read_serial_number(serial_number) != ATCA_SUCCESS) {
                        ESP_LOGI(TAG, "Can't read serial number :()");
                    } else {
                        ESP_LOGI(TAG, "Serial Number: 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X", serial_number[0], serial_number[1], serial_number[2], serial_number[3], serial_number[4], serial_number[5], serial_number[6], serial_number[7], serial_number[8]);
                    }
                }
                }
            
        }
    }
}
