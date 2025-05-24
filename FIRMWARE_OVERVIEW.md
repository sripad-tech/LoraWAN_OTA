# Firmware Overview: STM32 LoRaWAN CPV Logger

This document provides a brief overview of the STM32 firmware for a Concentrated Photovoltaic (CPV) field logger. The focus is on simplicity and core functionality.

## 1. Core Objective

To develop a LoRaWAN-enabled data logger for CPV systems with secure commissioning (LESC) and basic Over-The-Air (OTA) update capabilities. The firmware should be kept as simple as possible while meeting these core objectives.

## 2. Hardware Platform

*   **Microcontroller:** STM32WL series (specifically NUCLEO-WL55JC1 board using STM32WL55JCIx).
    *   Integrated LoRa radio.
    *   Sufficient processing power and peripherals.
*   **Secure Element:** STSAFE-A110.
    *   Used for secure key storage and cryptographic operations essential for LESC and secure OTA. Connected via I2C.

## 3. Key Firmware Functionalities & Modules

The firmware development will prioritize these features:

*   **LoRaWAN Communication (`src/app_lorawan/`):**
    *   Join LoRaWAN network using OTAA.
    *   Send uplink messages (e.g., sensor data, status).
    *   Receive downlink messages (e.g., ACKs, commands for OTA).
    *   Utilizes STM32CubeWL LoRaWAN middleware (I-CUBE-LRWAN).
*   **Secure Commissioning - LESC (`src/app_security/` and `drivers/components/stsafe_a110/`):**
    *   Leverages the STSAFE-A110 to perform key aspects of LESC.
    *   **Device Ephemeral Key Generation:** An ECC P256 key pair is generated within the STSAFE, and its public key is retrieved by the application (`lesc_app.c` using `stsafe_a110_core.c`).
    *   **Commissioning Payload Preparation:** The device prepares a payload containing its `DevEUI` and the generated ephemeral public key (`lesc_app.c`).
    *   **Conceptual Commissioning Exchange:**
        *   The commissioning payload is sent via a LoRaWAN message (`lorawan_app.c`).
        *   Reception of the Join Server's ephemeral public key is *simulated* within `lorawan_app.c` upon receiving any message on the LESC port after the request. This simulated key is passed to `lesc_app.c`.
    *   **ECDH Shared Secret Computation:** Using the device's ephemeral private key (held in STSAFE) and the (simulated) Join Server's public key, an ECDH shared secret is computed by the STSAFE (`stsafe_a110_core.c` called by `lesc_app.c`). The result is logged.
    *   **LoRaWAN Root Key Derivation (Placeholder):** A function (`lesc_app_derive_lorawan_keys`) exists as a placeholder to show where `AppKey` and `NwkKey` would be derived from the shared secret. Currently, it populates dummy keys and logs them. The actual cryptographic derivation is not yet implemented.
    *   The goal is to eventually use these derived keys for the actual LoRaWAN join.
*   **Over-The-Air (OTA) Updates (`src/app_ota/`, `bootloader/`, and `Core/Inc/ota_app_flags.h`):**
        *   **Goal:** Basic mechanism to update the main application firmware. Current implementation simulates Flash operations.
        *   **Flash Layout & Metadata (Conceptual - defined in `ota_app_flags.h`):**
            *   Bootloader Area (e.g., `0x08000000`, 32KB)
            *   Main Application Area (e.g., `0x08008000`, 96KB)
            *   Firmware Staging Area (e.g., `0x08020000`, 96KB) for downloaded firmware.
            *   `ota_firmware_metadata_t` (size, checksums, version) stored at the start of the staging area.
        *   **Application-Side (`src/app_ota/ota_app.c`):**
            *   `ota_app_simulate_download_and_stage_firmware()`:
                *   Uses a hardcoded dummy firmware image for simulation.
                *   Calculates metadata (size, version, checksum of image, checksum of metadata).
                *   **Simulates (via logging)** erasing the staging area, writing metadata, and writing the dummy firmware image to their defined Flash locations. No actual Flash hardware operations are performed.
            *   `main.c` calls this simulation. If successful, it calls `Trigger_OTA_Update()`.
            *   `Trigger_OTA_Update()` (in `main.c`): Writes `OTA_UPDATE_MAGIC_VALUE` to `OTA_UPDATE_MAGIC_ADDRESS` in RAM and triggers a system reset.
        *   **Bootloader-Side (`bootloader/src/main_bl.c`):**
            *   **Entry Point & Init:** `main_bl()` performs basic platform initialization (simplified, uses `HAL_Init()` for now).
            *   **OTA Flag Check:** On startup, checks `OTA_UPDATE_MAGIC_ADDRESS` in RAM.
            *   If OTA flag is set:
                *   Clears the flag in RAM.
                *   Calls `perform_ota_update()`.
                    *   **Simulates (via logging)** reading `ota_firmware_metadata_t` from the staging area.
                    *   **Simulates (via logging)** verifying the firmware image in staging against the metadata (checksums).
                    *   **Simulates (via logging)** copying the firmware from staging to the main application Flash area if verification passes.
                *   If simulated update is successful, `NVIC_SystemReset()` is called to boot the "new" application.
                *   If simulated update fails, it attempts to boot the existing application.
            *   **Application Boot:** If no OTA flag, or if OTA fails and fallback is chosen:
                *   `is_valid_application()`: Performs a basic heuristic check on the main application's vector table (MSP in RAM, Reset Handler in Flash & Thumb). This is a placeholder for robust checksum/signature verification.
                *   `jump_to_application()`: De-initializes bootloader peripherals (simplified), sets the application's stack pointer, and jumps to its reset handler.
*   **CPV Data Logging (Future - `src/app_core/`):**
    *   Interface with sensors (Voltage, Current, Temperature, Irradiance).
    *   Package and transmit data over LoRaWAN. This will be added after core LoRaWAN/OTA/LESC are functional.
*   **Main Application & System (`Core/`):**
    *   STM32CubeIDE generated structure.
    *   HAL drivers for MCU peripherals.
    *   Basic system initialization, LED indicators.
*   **Utility Modules (`src/app_utils/`):**
    *   Small helper functions (e.g., `byte_tools.c`). Unit tested using Ceedling.

## 4. Project Structure Highlights

*   **`Core/`**: STM32CubeIDE generated files (main.c, HAL config, system init).
*   **`Drivers/`**: STM32 HAL drivers, CMSIS.
*   **`drivers/components/`**: Drivers for external components like STSAFE-A110.
*   **`Middlewares/ST/STM32_LoRaWAN/`**: LoRaWAN stack.
*   **`src/app_lorawan/`**: LoRaWAN application layer code.
*   **`src/app_security/`**: LESC and STSAFE interaction logic.
*   **`src/app_ota/`**: Application-side OTA handling.
*   **`src/app_core/`**: Main application logic, sensor data handling (future).
*   **`src/app_utils/`**: Utility functions.
*   **`tests/`**: Ceedling unit tests.
*   **`bootloader/`**: (Future) Separate directory for the bootloader code.

## 5. Development Approach

*   Prioritize working code for core features.
*   Keep code simple and understandable.
*   Iterative development: Get basic LoRaWAN up, then STSAFE/LESC basics, then OTA signaling.
*   Unit tests for utility modules.

This document will be updated briefly as major components are implemented.
