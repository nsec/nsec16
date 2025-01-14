#pragma once

#include "cryptoauthlib.h"

// Slot to configure for SHA-HMAC access and Write key to
#define SLOT_HMAC 9

// Slot to configure for AES-HMAC access and Write key to
#define SLOT_AESHMAC 5

// Slot to store PrivWrite key
#define SLOT_PRIVWRITE 2

// Slot to store external private key to be loaded by the user
#define SLOT_EXT_PUBKEY 13


void print_16(uint8_t* data);
void print_16str(uint8_t* data);
void print_bin2hex(uint8_t* data, size_t size);
void encrypt_flag(uint8_t flag[16], uint8_t (&encrypted_flag)[16]);
void decrypt_flag(uint8_t encrypted_flag[16], uint8_t (&flag)[16]);
void print_config(atecc608_config_t * pConfig, uint16_t custom_param);
void print_n_hmac(const char* dataToHash, int slot_id, int n);
