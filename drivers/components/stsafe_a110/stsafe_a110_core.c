#include "stsafe_a110_core.h"
#include <string.h> // For memcpy

// For APP_LOG - ensure this is consistent with how it's defined elsewhere (e.g. lora_app_conf.h or a dedicated debug header)
// Assuming lora_app_conf.h is included where STSAFE_APP_LOG is used, or APP_LOG is globally available.
// Using a local definition for STSAFE driver's own logging.
#if 1 // Enable logging for STSAFE driver debugging
#include <stdio.h> // For basic printf, map to SWO or UART in real app
#define STSAFE_APP_LOG(PRINTF_ARGS) do { printf PRINTF_ARGS; } while(0)
#else
#define STSAFE_APP_LOG(PRINTF_ARGS)
#endif


// STSAFE I2C Address
#define STSAFE_A110_I2C_ADDRESS_WRITE (0x20 << 1 | 0x00) 
#define STSAFE_A110_I2C_ADDRESS_READ  (0x20 << 1 | 0x01) 
#define STSAFE_A110_I2C_ADDRESS_7BIT  (0x20)            

// Simplified conceptual command codes (NOT actual STSAFE commands)
#define CMD_STSAFE_WAKE              0x50 
#define CMD_STSAFE_GENERATE_ECC_KEY  0x46 
#define CMD_STSAFE_EXPORT_PUBLIC_KEY 0x48 
#define CMD_STSAFE_ECDH_COMPUTE_SECRET 0x52 // Conceptual: ECDH Compute Secret

#define STSAFE_I2C_TIMEOUT           100  // Timeout in ms for I2C operations

stsafe_status_t stsafe_init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == NULL) {
        return STSAFE_BAD_PARAM;
    }
    STSAFE_APP_LOG(("STSAFE: Initializing...\n"));

    if (HAL_I2C_IsDeviceReady(hi2c, STSAFE_A110_I2C_ADDRESS_WRITE, 3, STSAFE_I2C_TIMEOUT) != HAL_OK) {
        STSAFE_APP_LOG(("STSAFE: Device not ready on I2C bus (Addr: 0x%02X).\n", STSAFE_A110_I2C_ADDRESS_7BIT));
        return STSAFE_DEVICE_NOT_FOUND;
    }
    STSAFE_APP_LOG(("STSAFE: Device found on I2C bus.\n"));
    return STSAFE_OK;
}

stsafe_status_t stsafe_read_id(I2C_HandleTypeDef *hi2c, uint8_t* p_device_id, uint8_t id_len) {
    if (hi2c == NULL || p_device_id == NULL || id_len == 0) {
        return STSAFE_BAD_PARAM;
    }
    for(uint8_t i=0; i<id_len; ++i) { p_device_id[i] = 0xAA; } // Dummy data
    return STSAFE_OK;
}

stsafe_status_t stsafe_generate_ecc_p256_key_pair(I2C_HandleTypeDef *hi2c, uint8_t key_slot, uint8_t* public_key_out) {
    if (hi2c == NULL || public_key_out == NULL) {
        return STSAFE_BAD_PARAM;
    }
    if (key_slot > 7) { 
        STSAFE_APP_LOG(("STSAFE: Invalid key slot %d\n", key_slot));
        return STSAFE_BAD_PARAM;
    }

    HAL_StatusTypeDef hal_status;
    uint8_t command_buffer[2]; 
    uint8_t response_buffer[STSAFE_P256_PUBLIC_KEY_SIZE + 2]; // PubKey + header/status (conceptual)

    STSAFE_APP_LOG(("STSAFE: Generating ECC P256 key pair in slot %d (simulated)...\n", key_slot));
    HAL_Delay(100); // Simulate processing time for key generation

    STSAFE_APP_LOG(("STSAFE: Exporting public key from slot %d...\n", key_slot));
    command_buffer[0] = CMD_STSAFE_EXPORT_PUBLIC_KEY; 
    command_buffer[1] = key_slot;                    

    hal_status = HAL_I2C_Master_Transmit(hi2c, STSAFE_A110_I2C_ADDRESS_WRITE, command_buffer, 2, STSAFE_I2C_TIMEOUT);
    if (hal_status != HAL_OK) {
        STSAFE_APP_LOG(("STSAFE: Export PubKey - I2C Transmit failed (status: %d)\n", hal_status));
        return STSAFE_I2C_ERROR;
    }

    uint16_t expected_response_len = STSAFE_P256_PUBLIC_KEY_SIZE + 2; 
    hal_status = HAL_I2C_Master_Receive(hi2c, STSAFE_A110_I2C_ADDRESS_READ, response_buffer, expected_response_len, STSAFE_I2C_TIMEOUT + 50); 
    
    if (hal_status != HAL_OK) {
        STSAFE_APP_LOG(("STSAFE: Export PubKey - I2C Receive failed (status: %d)\n", hal_status));
        return STSAFE_I2C_ERROR;
    }

    if (response_buffer[0] != 0x90) { 
        STSAFE_APP_LOG(("STSAFE: Export PubKey - Command failed, STSAFE device status: 0x%02X\n", response_buffer[0]));
        return STSAFE_CMD_ERROR;
    }
    if (response_buffer[1] != STSAFE_P256_PUBLIC_KEY_SIZE) {
        STSAFE_APP_LOG(("STSAFE: Export PubKey - Incorrect public key length received: %d, expected %d\n", response_buffer[1], STSAFE_P256_PUBLIC_KEY_SIZE));
        memset(public_key_out, 0xEE, STSAFE_P256_PUBLIC_KEY_SIZE); 
        return STSAFE_ERROR; 
    }

    memcpy(public_key_out, &response_buffer[2], STSAFE_P256_PUBLIC_KEY_SIZE);
    STSAFE_APP_LOG(("STSAFE: Public key exported successfully from slot %d.\n", key_slot));

    return STSAFE_OK;
}

stsafe_status_t stsafe_ecdh_compute_shared_secret(I2C_HandleTypeDef *hi2c,
                                                uint8_t private_key_slot,
                                                const uint8_t* peer_public_key,
                                                uint8_t* shared_secret_out) {
    if (hi2c == NULL || peer_public_key == NULL || shared_secret_out == NULL) {
        return STSAFE_BAD_PARAM;
    }
    if (private_key_slot > 7) { // Assuming 8 key slots
        STSAFE_APP_LOG(("STSAFE: ECDH - Invalid private key slot %d\n", private_key_slot));
        return STSAFE_BAD_PARAM;
    }

    HAL_StatusTypeDef hal_status;
    // Command buffer for this simulation: CMD_CODE + PrivKeySlot + PubKey
    uint8_t command_buffer[2 + STSAFE_P256_PUBLIC_KEY_SIZE]; 
    uint8_t response_buffer[STSAFE_ECDH_SHARED_SECRET_SIZE + 2]; // Status, Length, Secret

    STSAFE_APP_LOG(("STSAFE: Computing ECDH Shared Secret using private key slot %d.\n", private_key_slot));

    command_buffer[0] = CMD_STSAFE_ECDH_COMPUTE_SECRET; // Conceptual command
    command_buffer[1] = private_key_slot;
    memcpy(&command_buffer[2], peer_public_key, STSAFE_P256_PUBLIC_KEY_SIZE);

    hal_status = HAL_I2C_Master_Transmit(hi2c, STSAFE_A110_I2C_ADDRESS_WRITE, command_buffer, sizeof(command_buffer), STSAFE_I2C_TIMEOUT + 50);
    if (hal_status != HAL_OK) {
        STSAFE_APP_LOG(("STSAFE: ECDH - I2C Transmit failed (status: %d)\n", hal_status));
        return STSAFE_I2C_ERROR;
    }

    HAL_Delay(150); // Simulate ECDH computation time

    hal_status = HAL_I2C_Master_Receive(hi2c, STSAFE_A110_I2C_ADDRESS_READ, response_buffer, STSAFE_ECDH_SHARED_SECRET_SIZE + 2, STSAFE_I2C_TIMEOUT + 50);
    if (hal_status != HAL_OK) {
        STSAFE_APP_LOG(("STSAFE: ECDH - I2C Receive failed (status: %d)\n", hal_status));
        return STSAFE_I2C_ERROR;
    }

    if (response_buffer[0] != 0x90) { // Conceptual success code
        STSAFE_APP_LOG(("STSAFE: ECDH - Command failed, STSAFE status: 0x%02X\n", response_buffer[0]));
        return STSAFE_CMD_ERROR;
    }
    if (response_buffer[1] != STSAFE_ECDH_SHARED_SECRET_SIZE) {
        STSAFE_APP_LOG(("STSAFE: ECDH - Incorrect shared secret length received: %d (expected %d)\n",
                         response_buffer[1], STSAFE_ECDH_SHARED_SECRET_SIZE));
        return STSAFE_ERROR;
    }

    memcpy(shared_secret_out, &response_buffer[2], STSAFE_ECDH_SHARED_SECRET_SIZE);
    STSAFE_APP_LOG(("STSAFE: ECDH Shared Secret computed successfully.\n"));

    return STSAFE_OK;
}
