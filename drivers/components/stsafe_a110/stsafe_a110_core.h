#ifndef STSAFE_A110_CORE_H
#define STSAFE_A110_CORE_H

#include "stm32wlxx_hal.h" // For I2C_HandleTypeDef

// Basic status codes
typedef enum {
    STSAFE_OK = 0,
    STSAFE_ERROR = 1,
    STSAFE_I2C_ERROR = 2,
    STSAFE_CMD_ERROR = 3,      // Command execution error reported by STSAFE
    STSAFE_BAD_PARAM = 4,
    STSAFE_DEVICE_NOT_FOUND = 5
} stsafe_status_t;

// Define P256 public key size (uncompressed format: 0x04 || X || Y)
#define STSAFE_P256_PUBLIC_KEY_SIZE 65 // 1 byte for format tag + 32 bytes for X + 32 bytes for Y
#define STSAFE_ECDH_SHARED_SECRET_SIZE 32 // Standard size for P256 ECDH shared secret

/**
 * @brief Initializes the STSAFE component and checks for device presence.
 * @param hi2c Pointer to the I2C_HandleTypeDef structure for STSAFE communication.
 * @retval stsafe_status_t Status of the initialization.
 */
stsafe_status_t stsafe_init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Placeholder for reading STSAFE device ID or version (remains placeholder).
 * @param hi2c Pointer to the I2C_HandleTypeDef structure.
 * @param p_device_id Buffer to store the device ID.
 * @param id_len Length of the buffer.
 * @retval stsafe_status_t Status.
 */
stsafe_status_t stsafe_read_id(I2C_HandleTypeDef *hi2c, uint8_t* p_device_id, uint8_t id_len);

/**
 * @brief Generates an ECC P256 key pair in a specified slot within STSAFE
 *        and retrieves the public key.
 * @param hi2c Pointer to the I2C_HandleTypeDef structure.
 * @param key_slot The slot number where the key pair will be generated. (e.g., 0)
 * @param public_key_out Buffer to store the retrieved public key (must be STSAFE_P256_PUBLIC_KEY_SIZE bytes).
 * @retval stsafe_status_t Status of the operation.
 */
stsafe_status_t stsafe_generate_ecc_p256_key_pair(I2C_HandleTypeDef *hi2c, uint8_t key_slot, uint8_t* public_key_out);

/**
 * @brief Computes an ECDH shared secret using a private key stored in STSAFE
 *        and a peer's public key.
 * @param hi2c Pointer to the I2C_HandleTypeDef structure.
 * @param private_key_slot Slot number of the device's private key for ECDH.
 * @param peer_public_key Pointer to the peer's public key (STSAFE_P256_PUBLIC_KEY_SIZE bytes).
 * @param shared_secret_out Buffer to store the computed shared secret (STSAFE_ECDH_SHARED_SECRET_SIZE bytes).
 * @retval stsafe_status_t Status of the operation.
 */
stsafe_status_t stsafe_ecdh_compute_shared_secret(I2C_HandleTypeDef *hi2c,
                                                uint8_t private_key_slot,
                                                const uint8_t* peer_public_key,
                                                uint8_t* shared_secret_out);

#endif // STSAFE_A110_CORE_H
