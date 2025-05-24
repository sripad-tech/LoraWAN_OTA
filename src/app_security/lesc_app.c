#include "lesc_app.h"
#include "stsafe_a110_core.h"
#include "lora_app_conf.h" // For LORAWAN_DEVICE_EUI, LORAWAN_JOIN_EUI (needed for real derivation), and APP_LOG
#include <string.h> // For memcpy

// APP_LOG is expected to be defined in lora_app_conf.h
// Ensure lora_app_conf.h is included and LORAWAN_DEBUG_APP is set appropriately.

static lesc_data_t g_lesc_data;
static I2C_HandleTypeDef *g_hi2c_stsafe_lesc = NULL;

int lesc_app_init(I2C_HandleTypeDef *hi2c_stsafe) {
    if (hi2c_stsafe == NULL) {
        APP_LOG(("LESC_APP: ERROR - I2C handle for STSAFE is NULL.\n"));
        return -1;
    }
    g_hi2c_stsafe_lesc = hi2c_stsafe;
    g_lesc_data.is_ephemeral_key_generated = 0;
    g_lesc_data.is_join_server_key_received = 0;
    g_lesc_data.is_shared_secret_computed = 0;
    g_lesc_data.are_lorawan_keys_derived = 0; // Initialize new flag
    APP_LOG(("LESC_APP: Initialized.\n"));
    return 0;
}

int lesc_app_start_ecdh_key_generation(void) {
    if (g_hi2c_stsafe_lesc == NULL) {
        APP_LOG(("LESC_APP: ERROR - STSAFE I2C handle not initialized for LESC.\n"));
        return -1;
    }
    APP_LOG(("LESC_APP: Generating ephemeral ECC key pair in STSAFE slot %d...\n", LESC_EPHEMERAL_KEY_SLOT));
    stsafe_status_t stsafe_stat = stsafe_generate_ecc_p256_key_pair(
        g_hi2c_stsafe_lesc, LESC_EPHEMERAL_KEY_SLOT, g_lesc_data.ephemeral_public_key);
    if (stsafe_stat == STSAFE_OK) {
        g_lesc_data.is_ephemeral_key_generated = 1;
        APP_LOG(("LESC_APP: Ephemeral ECC Key Pair generated successfully.\n"));
        return 0;
    } else {
        g_lesc_data.is_ephemeral_key_generated = 0;
        APP_LOG(("LESC_APP: FAILED to generate ephemeral ECC Key Pair. STSAFE Status: %d\n", stsafe_stat));
        return -1;
    }
}

const uint8_t* lesc_app_get_ephemeral_public_key(void) {
    if (g_lesc_data.is_ephemeral_key_generated) { return g_lesc_data.ephemeral_public_key; }
    APP_LOG(("LESC_APP: WARNING - Ephemeral public key requested but not generated.\n"));
    return NULL;
}

int lesc_app_prepare_commissioning_payload(uint8_t* output_buffer, size_t buffer_len, size_t* payload_len) {
    if (output_buffer == NULL || payload_len == NULL) { return -1; }
    if (!g_lesc_data.is_ephemeral_key_generated) { return -1; }
    if (buffer_len < LESC_COMMISSIONING_PAYLOAD_SIZE) { return -1; }
    uint8_t dev_eui[] = LORAWAN_DEVICE_EUI;
    memcpy(output_buffer, dev_eui, sizeof(dev_eui));
    memcpy(output_buffer + sizeof(dev_eui), g_lesc_data.ephemeral_public_key, STSAFE_P256_PUBLIC_KEY_SIZE);
    *payload_len = LESC_COMMISSIONING_PAYLOAD_SIZE;
    APP_LOG(("LESC_APP: Prepared commissioning payload (%lu bytes).\n", (unsigned long)*payload_len));
    return 0;
}

void lesc_app_process_join_server_public_key(const uint8_t* js_eph_pub_key, size_t key_len) {
    if (js_eph_pub_key == NULL || key_len != STSAFE_P256_PUBLIC_KEY_SIZE) {
        APP_LOG(("LESC_APP: ERROR - Invalid Join Server public key data (len %lu, expected %d).\n", (unsigned long)key_len, STSAFE_P256_PUBLIC_KEY_SIZE));
        return;
    }
    memcpy(g_lesc_data.join_server_ephemeral_public_key, js_eph_pub_key, STSAFE_P256_PUBLIC_KEY_SIZE);
    g_lesc_data.is_join_server_key_received = 1;
    APP_LOG(("LESC_APP: Processed (simulated) Join Server Ephemeral Public Key.\n"));

    if (lesc_app_compute_ecdh_shared_secret() != 0) {
        APP_LOG(("LESC_APP: FAILED to compute ECDH shared secret after receiving JS key.\n"));
    }
}

int lesc_app_compute_ecdh_shared_secret(void) {
    if (!g_lesc_data.is_ephemeral_key_generated || !g_lesc_data.is_join_server_key_received || g_hi2c_stsafe_lesc == NULL) {
        APP_LOG(("LESC_APP: ECDH - Pre-conditions not met (DevKey: %d, JSKey: %d, I2C: %p)\n",
                 g_lesc_data.is_ephemeral_key_generated, g_lesc_data.is_join_server_key_received, (void*)g_hi2c_stsafe_lesc));
        return -1;
    }
    APP_LOG(("LESC_APP: Computing ECDH Shared Secret...\n"));
    stsafe_status_t stsafe_stat = stsafe_ecdh_compute_shared_secret(
        g_hi2c_stsafe_lesc, LESC_EPHEMERAL_KEY_SLOT,
        g_lesc_data.join_server_ephemeral_public_key, g_lesc_data.shared_secret);

    if (stsafe_stat == STSAFE_OK) {
        g_lesc_data.is_shared_secret_computed = 1;
        APP_LOG(("LESC_APP: ECDH Shared Secret computed successfully (%d bytes).\n", STSAFE_ECDH_SHARED_SECRET_SIZE));
        // Log shared secret (optional, for debug)
        // for (int i = 0; i < STSAFE_ECDH_SHARED_SECRET_SIZE; ++i) { /* ... log ... */ }

        // Now, attempt to derive LoRaWAN keys
        if (lesc_app_derive_lorawan_keys() != 0) {
            APP_LOG(("LESC_APP: FAILED to derive LoRaWAN keys after computing shared secret.\n"));
            // Shared secret computation was OK, but derivation failed. Return success for ECDH part.
        }
        return 0; // ECDH itself was OK
    } else {
        g_lesc_data.is_shared_secret_computed = 0;
        APP_LOG(("LESC_APP: FAILED to compute ECDH Shared Secret. STSAFE Status: %d\n", stsafe_stat));
        return -1;
    }
}

int lesc_app_derive_lorawan_keys(void) {
    if (!g_lesc_data.is_shared_secret_computed) {
        APP_LOG(("LESC_APP: KeyDeriv - Shared secret not computed. Cannot derive LoRaWAN keys.\n"));
        return -1;
    }

    APP_LOG(("LESC_APP: KeyDeriv - CONCEPTUAL derivation of LoRaWAN keys from Shared Secret.\n"));
    APP_LOG(("LESC_APP: KeyDeriv - Shared Secret (Input, %d bytes):\n", STSAFE_ECDH_SHARED_SECRET_SIZE));
    for (int i = 0; i < STSAFE_ECDH_SHARED_SECRET_SIZE; ++i) {
        APP_LOG(("%02X ", g_lesc_data.shared_secret[i]));
        if ((i + 1) % 16 == 0 && i < (STSAFE_ECDH_SHARED_SECRET_SIZE - 1)) APP_LOG(("\n"));
    }
    APP_LOG(("\n"));

    // --- PLACEHOLDER for actual key derivation ---
    // For this placeholder, we'll just copy parts of the shared secret as dummy keys.
    // This is NOT cryptographically sound.
    memcpy(g_lesc_data.derived_app_key, g_lesc_data.shared_secret, LORAWAN_KEY_SIZE);
    // Make NwkKey different from AppKey for the dummy version
    // Ensure shared secret is large enough (32 bytes) for this split
    if (STSAFE_ECDH_SHARED_SECRET_SIZE >= LORAWAN_KEY_SIZE * 2) {
        for (int i = 0; i < LORAWAN_KEY_SIZE; ++i) {
            g_lesc_data.derived_nwk_key[i] = g_lesc_data.shared_secret[LORAWAN_KEY_SIZE + i];
        }
    } else {
        // If shared secret is not 32 bytes, just copy it to NwkKey as well for placeholder
        memcpy(g_lesc_data.derived_nwk_key, g_lesc_data.shared_secret, LORAWAN_KEY_SIZE);
    }


    g_lesc_data.are_lorawan_keys_derived = 1;

    APP_LOG(("LESC_APP: KeyDeriv - DUMMY AppKey (first 16B of SharedSecret):\n"));
    for (int i = 0; i < LORAWAN_KEY_SIZE; ++i) APP_LOG(("%02X ", g_lesc_data.derived_app_key[i]));
    APP_LOG(("\n"));

    APP_LOG(("LESC_APP: KeyDeriv - DUMMY NwkKey (second 16B of SharedSecret or copy):\n"));
    for (int i = 0; i < LORAWAN_KEY_SIZE; ++i) APP_LOG(("%02X ", g_lesc_data.derived_nwk_key[i]));
    APP_LOG(("\n"));

    APP_LOG(("LESC_APP: KeyDeriv - LoRaWAN keys (conceptually) derived.\n"));
    return 0;
}
