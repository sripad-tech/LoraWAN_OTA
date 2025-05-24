#ifndef LESC_APP_H
#define LESC_APP_H

#include "stsafe_a110_core.h"
#include <stddef.h>

#define LESC_EPHEMERAL_KEY_SLOT 1
#define LESC_COMMISSIONING_PAYLOAD_SIZE (8 + STSAFE_P256_PUBLIC_KEY_SIZE)
#define LORAWAN_KEY_SIZE 16 // AppKey and NwkKey are 16 bytes

typedef struct {
    uint8_t ephemeral_public_key[STSAFE_P256_PUBLIC_KEY_SIZE];
    uint8_t is_ephemeral_key_generated;
    uint8_t join_server_ephemeral_public_key[STSAFE_P256_PUBLIC_KEY_SIZE];
    uint8_t is_join_server_key_received;
    uint8_t shared_secret[STSAFE_ECDH_SHARED_SECRET_SIZE];
    uint8_t is_shared_secret_computed;
    uint8_t derived_app_key[LORAWAN_KEY_SIZE]; // New
    uint8_t derived_nwk_key[LORAWAN_KEY_SIZE]; // New
    uint8_t are_lorawan_keys_derived;          // New
} lesc_data_t;

int lesc_app_init(I2C_HandleTypeDef *hi2c_stsafe);
int lesc_app_start_ecdh_key_generation(void);
const uint8_t* lesc_app_get_ephemeral_public_key(void); // Added back prototype
int lesc_app_prepare_commissioning_payload(uint8_t* output_buffer, size_t buffer_len, size_t* payload_len); // Added back prototype
void lesc_app_process_join_server_public_key(const uint8_t* js_eph_pub_key, size_t key_len);
int lesc_app_compute_ecdh_shared_secret(void);

/**
 * @brief Conceptually derives LoRaWAN AppKey and NwkKey from the shared secret.
 *        This is a PLACEHOLDER and does not perform actual cryptographic derivation.
 *        It should be called after the ECDH shared secret is computed.
 * @retval 0 on success, -1 on failure (e.g., shared secret not available).
 */
int lesc_app_derive_lorawan_keys(void);

#endif // LESC_APP_H
