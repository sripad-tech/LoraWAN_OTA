# Secure Element (STSAFE-A110) Driver and LESC Scaffolding

This document details the plan for integrating the STSAFE-A110 Secure Element, developing its driver, and scaffolding the LoRaWAN End-device Secure Commissioning (LESC) process.

## 1. STSAFE-A110 Driver

*   **Source:** STMicroelectronics provides middleware and drivers for the STSAFE-A100 series, typically as part of their STM32Cube expansion packages (e.g., X-CUBE-SBSFU or standalone STSAFE libraries). The project will leverage these official drivers and port them if necessary to the STM32WL platform.
*   **Communication Interface:** The STSAFE-A110 will be connected to the STM32WL via an I2C interface. The driver will manage I2C communication, including command/response handling as per the STSAFE-A110 datasheet.
*   **Driver Location:** The driver code will reside in `drivers/components/stsafe_a110/`.

## 2. Core STSAFE-A110 Functionalities to Expose

The driver, along with a higher-level security module (`app_security/`), will expose functions to:

*   **Initialization and Health Check:** Initialize the STSAFE, check its status, and verify communication.
*   **Secure Key Generation:**
    *   Generate elliptic curve key pairs (e.g., P256) within the STSAFE. The private key will never leave the STSAFE.
    *   Extract the public key.
*   **Secure Key Storage & Management:**
    *   Write pre-provisioned keys (e.g., a batch key for initial LESC, or LoRaWAN root keys if not using LESC for initial provisioning) to secure slots.
    *   Read public keys or certificates from specific slots.
    *   Update keys securely when required (e.g., after re-keying).
    *   Prevent unauthorized read access to private/secret keys.
*   **Cryptographic Operations:**
    *   **ECDH (Elliptic Curve Diffie-Hellman):** Perform ECDH key agreement using a private key stored in STSAFE and a public key provided by the peer (e.g., Join Server for LESC).
    *   **ECDSA (Elliptic Curve Digital Signature Algorithm):**
        *   Sign data/hashes using a private key stored in STSAFE (e.g., for challenge-response mechanisms or application-level signatures).
        *   Verify signatures using a public key (though signature verification for OTA is primarily a bootloader task, STSAFE could assist if needed).
    *   **AES (Advanced Encryption Standard):**
        *   Encrypt/decrypt data using keys stored in STSAFE (e.g., for securing sensitive application data before storage or transmission if end-to-end encryption beyond LoRaWAN's is needed).
        *   Derive keys using CMAC-AES.
*   **Secure Read/Write:** Access monotonic counters, secure data slots for application parameters.
*   **Nonce Generation/Random Number Generation:** Utilize STSAFE's hardware random number generator for cryptographic purposes.

## 3. LoRaWAN End-device Secure Commissioning (LESC)

LESC provides a standardized and secure way to provision LoRaWAN devices with their root keys (AppKey, NwkKey) using a Join Server. The STSAFE-A110 is crucial for implementing LESC securely.

**LESC Process Outline (Simplified):**

1.  **Pre-requisites:**
    *   The device (via STSAFE) needs a unique identifier (e.g., `DevEUI`) and a pre-shared secret or a trusted public key (`Owner_Key` or `Batch_Key`) that is also known to the Join Server. This might be provisioned during manufacturing into STSAFE.
2.  **Key Exchange (ECDH):**
    *   The device generates an ephemeral ECDH key pair within STSAFE (`Device_Ephemeral_Private_Key`, `Device_Ephemeral_Public_Key`).
    *   The device sends its `DevEUI` and `Device_Ephemeral_Public_Key` to the Join Server as part of an initial (unsecured or minimally secured) message.
    *   The Join Server, possessing its own key pair and the device's pre-shared secret/public key, performs an ECDH operation to derive a shared secret.
    *   The device, using its `Device_Ephemeral_Private_Key` and the Join Server's public key (which might be sent to the device or pre-configured), also computes the same shared secret via ECDH within STSAFE.
3.  **Key Derivation:**
    *   Both the device (within STSAFE) and the Join Server use this shared secret, along with other parameters (like `DevNonce`, `JoinEUI`), to derive the LoRaWAN session keys (`AppSKey`, `NwkSKey`) and ultimately the root keys (`AppKey`, `NwkKey`). The exact derivation steps are defined in the LoRaWAN specifications (e.g., LoRaWAN Backend Interfaces spec).
4.  **Root Key Storage:**
    *   The derived `AppKey` and `NwkKey` are then securely stored in dedicated slots within the STSAFE-A110.
5.  **Confirmation:** The device can then perform a standard LoRaWAN Join procedure (OTAA) using the newly provisioned root keys.

**Role of `app_security/` module:**

*   Orchestrate the LESC process.
*   Request STSAFE to perform necessary operations (key generation, ECDH).
*   Format messages for communication with the Join Server during LESC.
*   Handle responses and store derived keys into STSAFE.

## 4. LoRaWAN Key Management after LESC

*   Once `AppKey` and `NwkKey` (and `DevEUI`, `AppEUI`/`JoinEUI`) are provisioned in STSAFE:
    *   The LoRaWAN stack will request these keys from the `app_security` module when needed (e.g., for the OTAA Join procedure).
    *   The `app_security` module will retrieve them from STSAFE. **The keys themselves are not read out from STSAFE; rather, STSAFE is commanded to use these keys for specific LoRaWAN operations if its command set supports it, or the keys are used by the MCU after being securely retrieved in a trusted execution context.** For LoRaWAN join, the MCU typically needs the root keys to pass to the LoRaWAN stack, which then derives session keys. STSAFE's primary role here is the *secure storage* of these root keys.

## 5. Scaffolding and Initial Implementation

*   **STSAFE Driver Integration:** Add ST's STSAFE driver to the project, configure I2C.
*   **Basic STSAFE API:** Create wrapper functions in `app_security/` for common STSAFE operations (init, read ID, generate key pair, basic ECDH).
*   **LESC State Machine Placeholder:** Outline the states of the LESC process within `app_security/lesc_handler.c`.
*   **Key Slot Definition:** Define which STSAFE slots will be used for `DevEUI`, `Owner_Key`/`Batch_Key`, ephemeral keys, and the final LoRaWAN root keys.

This approach ensures that sensitive cryptographic operations and key storage are handled by the hardware Secure Element, significantly enhancing the security of the device and the LESC process.
```
