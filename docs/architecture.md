# Software Architecture

This document outlines the software architecture of the STM32 LoRaWAN CPV Field Logger firmware.

## Key Modules

*   **Bootloader:** Responsible for verifying and applying firmware updates. Ensures secure and reliable firmware updating.
    *   *Details: [Bootloader Design](bootloader_design.md)*
*   **HAL (Hardware Abstraction Layer):** Low-level drivers and MCU peripheral configuration (e.g., STM32Cube HAL/LL drivers for STM32WL).
*   **BSP (Board Support Package):** Board-specific configurations and drivers for components like LEDs, buttons (if any), and specific pin configurations not directly part of a sensor or external component driver.
*   **Sensor Drivers:** Drivers for specific sensors used in the CPV logger (e.g., temperature, irradiance, voltage/current). These will be located in `drivers/components/`.
    *   *Core Logic Interface: [Application Core Logic](application_core_logic.md)*
*   **LoRaWAN Stack Module (`app_lorawan/`):** Manages LoRaWAN communication (joining, uplink, downlink) using the chosen stack (e.g., I-CUBE-LRWAN).
    *   *Details: [LoRaWAN Stack Integration and Configuration](lorawan_integration.md)*
*   **Security Module (`app_security/`):** Handles secure pairing (LESC) and cryptographic operations, interfacing with the Secure Element (STSAFE-A110). Manages secure key storage and access.
    *   *Details: [Secure Element (STSAFE-A110) Driver and LESC Scaffolding](secure_element_lesc.md)*
*   **OTA Update Module (Application Side - `app_ota/`):** Manages the download, initial verification, and preparation of new firmware images before rebooting to the bootloader.
    *   *Details: [OTA Update Application-Side Logic](ota_application_logic.md)*
*   **Application Core Logic (`app_core/`):** Core application coordinating sensor reading, data processing and packaging (e.g. using Cayenne LPP), scheduling LoRaWAN transmissions, and handling application-level commands or events.
    *   *Details: [Application Core Logic - Data Acquisition and Transmission](application_core_logic.md)*
*   **Power Management Module (`power_manager/`):** Implements strategies to minimize power consumption, managing MCU sleep modes and peripheral power states.
    *   *Details: [Initial Power Management Strategy](power_management_strategy.md)*
*   **Unit Testing Framework:** Infrastructure for unit testing modules, likely using Ceedling (Unity/CMock).
    *   *Details: [Build System and Basic Test Structure](build_and_test_strategy.md)*


## Directory Structure

The proposed high-level directory structure is as follows (also detailed in `../../PROPOSED_DIRECTORY_STRUCTURE.txt`):

/
├── src/                     # Source files for the main application
│   ├── main.c
│   ├── stm32wlxx_it.c       # Interrupt handlers
│   ├── app_core/            # Core application logic
│   ├── app_lorawan/         # LoRaWAN application specifics
│   ├── app_ota/             # OTA update application logic
│   └── app_security/        # Security and LESC logic
│   └── power_manager/       # Power management logic
├── drivers/                 # Device drivers
│   ├── bsp/                 # Board Support Package
│   ├── cmsis/               # CMSIS core and device files
│   ├── stm32wlxx_hal_driver/# STM32WL HAL drivers
│   └── components/          # Drivers for external components (sensors, STSAFE)
│       ├── stsafe_a110/
│       └── sensor_xyz/      # Placeholder for specific sensor drivers
├── middlewares/             # Third-party middleware (e.g., LoRaWAN stack)
│   └── st/
│       └── stm32_lorawan/   # Example path for ST's LoRaWAN middleware
├── bootloader/              # Bootloader code (separate project/target ideally)
│   ├── src/
│   └── include/
├── docs/                    # Project documentation (this file, and others)
│   ├── architecture.md
│   ├── hardware_selection.md
│   ├── bootloader_design.md
│   ├── lorawan_integration.md
│   ├── secure_element_lesc.md
│   ├── application_core_logic.md
│   ├── ota_application_logic.md
│   ├── power_management_strategy.md
│   └── build_and_test_strategy.md
├── tests/                   # Test files
│   ├── unit/                # Unit tests (e.g., for Ceedling)
│   └── integration/         # Integration test plans/scripts
├── Makefile                 # Or project files for an IDE (e.g., .cproject, .project for STM32CubeIDE)
└── README.md                # Main project README
