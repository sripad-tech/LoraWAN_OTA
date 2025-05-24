# STM32 LoRaWAN CPV Field Logger Firmware

This project contains the firmware for a LoRaWAN-enabled data logger designed for Concentrated Photovoltaic (CPV) field monitoring. It features Over-The-Air (OTA) updates and secure pairing using LESC.

## Hardware Requirements

*   **Microcontroller:** STM32WL series (e.g., STM32WLE5xx).
    *   **Rationale:** The STM32WL series is chosen for its integrated LoRa transceiver, which simplifies hardware design and reduces BOM cost. It offers an Arm Cortex-M4 core providing sufficient processing power for the application's needs, including sensor data processing, LoRaWAN stack management, and cryptographic operations. It also provides a good range of peripherals (I2C, SPI, UART, ADCs) suitable for interfacing with various sensors and a Secure Element. Its low-power modes are essential for battery-operated deployments.
    *   See: `docs/hardware_selection.md`
*   **Secure Element:** STSAFE-A110 (or equivalent).
    *   **Rationale:** An STSAFE-A110 Secure Element is incorporated to provide robust, hardware-based security for critical operations. This includes secure storage of keys, and offloading cryptographic computations.
    *   See: `docs/hardware_selection.md` and `docs/secure_element_lesc.md`
*   **Sensors:** (To be detailed based on specific CPV monitoring requirements)
    *   Examples: Solar irradiance sensors (pyranometers), temperature sensors (ambient and panel), voltage/current sensors for PV strings, GPS module (optional, for location).
*   **LoRa Antenna**

## Software Architecture

An overview of the software architecture is available in `docs/architecture.md`.

## Key Design Documents

The following documents provide detailed design information for key aspects of the firmware:

*   [Hardware Selection Rationale](docs/hardware_selection.md)
*   [Software Architecture Overview](docs/architecture.md)
*   [Bootloader Design](docs/bootloader_design.md)
*   [LoRaWAN Stack Integration and Configuration](docs/lorawan_integration.md)
*   [Secure Element (STSAFE-A110) Driver and LESC Scaffolding](docs/secure_element_lesc.md)
*   [Application Core Logic - Data Acquisition and Transmission](docs/application_core_logic.md)
*   [OTA Update Application-Side Logic](docs/ota_application_logic.md)
*   [Initial Power Management Strategy](docs/power_management_strategy.md)
*   [Build System and Basic Test Structure](docs/build_and_test_strategy.md)

## Setup and Build

(To be detailed - will include instructions for STM32CubeIDE and Ceedling for unit tests)

## Directory Structure

A textual representation of the proposed root directory structure is available in `PROPOSED_DIRECTORY_STRUCTURE.txt`.
