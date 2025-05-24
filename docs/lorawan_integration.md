# LoRaWAN Stack Integration and Configuration

This document outlines the integration and configuration of the LoRaWAN stack for the STM32 CPV Field Logger.

## 1. LoRaWAN Stack Selection

*   **Stack:** STMicroelectronics I-CUBE-LRWAN (LoRaWAN Middleware).
*   **Rationale:** This stack is provided and supported by ST, ensuring good compatibility with STM32WL microcontrollers. It includes example applications and a framework for building LoRaWAN end-devices. Alternatives like LoRaMac-node could be considered if specific features not present in I-CUBE-LRWAN are required, but I-CUBE-LRWAN will be the baseline for this project.
*   **Version:** The latest stable version available at the project start should be used.

## 2. Core Configuration Parameters

The LoRaWAN stack will be configured with the following key parameters, typically managed in a configuration header file (e.g., `lora_app_conf.h` or similar within the middleware structure):

*   **Regional Parameters:**
    *   This will be configurable at build time (e.g., via defines) to support various regions like `LORAMAC_REGION_EU868`, `LORAMAC_REGION_US915`, `LORAMAC_REGION_AS923`, etc. The initial target region should be specified (e.g., EU868).
    *   Adaptive Data Rate (ADR) will be **enabled** (`LORAWAN_ADR_STATE` set to `LORAWAN_ADR_ON`) to allow the network server to optimize data rate and power consumption.
*   **Activation Method:**
    *   **Over-The-Air Activation (OTAA)** will be the primary method. This is more secure than ABP as session keys are negotiated with the network server on each join.
    *   Activation By Personalization (ABP) might be supported for specific testing or fallback scenarios, but OTAA is preferred for production.
*   **Device Class:**
    *   The device will primarily operate as a **Class A** device. This is the most power-efficient class, where the end-device initiates communication and opens receive windows only after an uplink transmission.
    *   Support for Class C could be considered for the firmware update process if faster download times are critical and power constraints allow, but Class A will be the default operational mode.
*   **Uplink/Downlink Capabilities:**
    *   The application will support sending **confirmed and unconfirmed uplinks**. Confirmed uplinks will be used for critical data or commands, while unconfirmed uplinks will be used for routine sensor data to save power and bandwidth.
    *   The application will be capable of receiving downlinks during the two receive windows (RX1, RX2) after an uplink.

## 3. LoRaWAN Keys Management

*   **Secure Storage:** All LoRaWAN keys must be stored securely. As per the hardware design, the following keys will be provisioned into and managed by the **STSAFE-A110 Secure Element**:
    *   `DevEUI` (End-Device Identifier)
    *   `AppEUI` (Application Identifier, also known as `JoinEUI` in LoRaWAN 1.1+)
    *   `AppKey` (Application Root Key, used for OTAA to derive session keys)
    *   `NwkKey` (Network Root Key, used for OTAA in LoRaWAN 1.1+ to derive session keys; in LoRaWAN 1.0.x, AppKey is often used for both functions).
*   **Provisioning:** The method for provisioning these keys onto the STSAFE (e.g., during manufacturing or commissioning) needs to be defined. Secure pairing using LESC (LoRaWAN End-Device Secure Commissioning) will be the target mechanism for initial key establishment or re-keying in the field.
*   **Access:** The LoRaWAN stack will need to interface with the Security Module (which in turn interfaces with STSAFE) to retrieve these keys when needed (e.g., during the join procedure). The actual keys should not be directly accessible or stored in plain text in the main MCU's flash or RAM.

## 4. Basic LoRaWAN Functionalities to Implement

The initial implementation within `app_lorawan/` will focus on:

*   **Initialization:**
    *   Initialize the LoRaWAN middleware (`LORA_Init()`).
    *   Set necessary callbacks for events like `TxDone`, `RxData`, `Joined`, etc.
*   **Joining the Network:**
    *   Implement the OTAA join procedure (`LORA_Join()`).
    *   Handle join success and failure, including retries with appropriate backoff strategies.
*   **Sending Uplink Messages:**
    *   Provide a function to send data payloads (e.g., `lorawan_send_data(payload, length, confirmed)`).
    *   Manage duty cycle restrictions imposed by the LoRaWAN specification and regional regulations. The stack usually handles this, but the application must be aware.
*   **Receiving Downlink Messages:**
    *   Implement a handler for received downlink data. This will be crucial for commands, acknowledgments, and potentially for initiating OTA updates.
*   **Low-Power Considerations:**
    *   Ensure the LoRaWAN radio and MCU enter low-power modes between transmissions and during idle periods. The I-CUBE-LRWAN stack and STM32WL HAL provide mechanisms for this.

## 5. Integration with Application Core

*   The `app_core` module will invoke `app_lorawan` functions to send data.
*   `app_lorawan` will use a message queue or callback system to inform `app_core` or other relevant modules about network status, received data, and transmission confirmations.

This outline provides the basis for integrating the LoRaWAN stack. The actual implementation will involve writing C code within the `app_lorawan/` directory, configuring the I-CUBE-LRWAN middleware, and interfacing with the security module for key handling.
```
