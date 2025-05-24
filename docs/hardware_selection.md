# Hardware Selection Rationale

This document provides a more detailed rationale for the key hardware components selected for the STM32 LoRaWAN CPV Field Logger.

## Microcontroller: STM32WL Series (e.g., STM32WLE5xx)

*   **Integrated LoRa Transceiver:** The primary advantage is the single-chip solution for both the main application processor and the LoRa radio. This reduces board complexity, size, and bill of materials (BOM) cost compared to using a separate MCU and LoRa module.
*   **Arm Cortex-M4 Core:** Provides a good balance of processing power and energy efficiency. It's capable of handling:
    *   Real-time sensor data acquisition and processing.
    *   Running the LoRaWAN stack.
    *   Implementing security protocols (though sensitive operations are offloaded to the Secure Element).
    *   Managing OTA firmware updates.
*   **Rich Peripheral Set:** Includes:
    *   Multiple I2C, SPI, and UART interfaces for connecting various sensors, the Secure Element, and potentially other peripherals like external memory or GPS modules.
    *   ADCs for analog sensor readings.
    *   Timers for precise event scheduling and pulse width modulation (PWM) if needed.
*   **Low-Power Modes:** Essential for battery-powered field deployments. The STM32WL offers various low-power modes (Sleep, Stop, Standby, Shutdown) that allow the firmware to significantly reduce energy consumption when the device is idle.
*   **Memory:** Sufficient Flash and RAM for the firmware, LoRaWAN stack, and data buffering. The specific variant (e.g., STM32WLE5JCIx) will be chosen based on the final memory footprint.
*   **Ecosystem and Tools:** STMicroelectronics provides a comprehensive development ecosystem, including STM32CubeIDE, STM32CubeMX for configuration, and extensive libraries (HAL, LoRaWAN middleware).

## Secure Element: STSAFE-A110 (or equivalent)

*   **Hardware-Based Security:** Offers a much higher level of security compared to software-only solutions for storing sensitive data and performing cryptographic operations.
*   **Secure Key Storage:**
    *   **LoRaWAN Keys:** Protects `AppKey`, `NwkKey` (for LoRaWAN 1.0.x) or `AppSKey`, `NwkSKey` (for LoRaWAN 1.1.x after join), `DevEUI`, and `AppEUI` (or `JoinEUI`). If these keys are compromised, the device's communication can be intercepted or impersonated.
    *   **LESC Private Key:** Stores the device's private key for the Elliptic Curve Diffie-Hellman (ECDH) key exchange during the LESC process. Compromise of this key would allow an attacker to impersonate the device during pairing.
    *   **OTA Verification Key/Certificate:** Stores the public key or a root certificate hash used to verify the authenticity and integrity of downloaded firmware images. This prevents malicious firmware from being loaded.
*   **Cryptographic Offloading:**
    *   Performs cryptographic operations like ECDH key generation and shared secret calculation for LESC.
    *   Performs signature verification (e.g., ECDSA) for OTA firmware images.
    *   This offloading reduces the computational burden on the main MCU and protects sensitive operations from being exposed in the main MCU's less secure environment.
*   **Tamper Resistance:** Secure Elements are designed to be resistant to physical attacks aimed at extracting stored keys.
*   **Simplified Compliance:** Using a certified Secure Element can help in meeting security requirements for certain LoRaWAN deployments or industry standards.

## Sensors

The specific choice of sensors will depend on the exact parameters to be monitored for the CPV system. Common examples include:

*   **Solar Irradiance Sensors (Pyranometers):** To measure the solar energy incident on the CPV panels.
    *   *Interface:* Typically analog output, requiring an ADC, or digital (e.g., Modbus over RS485, I2C).
*   **Temperature Sensors:**
    *   *Ambient Temperature:* To correlate environmental conditions with CPV performance.
    *   *Panel Temperature:* CPV performance is often sensitive to temperature.
    *   *Interface:* Analog (thermistor, requiring ADC and calibration), or digital (e.g., DS18B20 via 1-Wire, I2C sensors like TMP117).
*   **Voltage/Current Sensors:** To monitor the electrical output of PV strings or individual modules.
    *   *Interface:* Often involves shunt resistors and operational amplifiers feeding into ADCs, or dedicated ICs with I2C/SPI interfaces (e.g., INA219).
*   **GPS Module (Optional):**
    *   *Purpose:* For accurate location tracking of deployed loggers, which can be useful for large fields or mobile CPV units. Also provides an accurate time source.
    *   *Interface:* Typically UART.
    *   *Consideration:* GPS modules can be power-hungry, so their usage needs to be managed carefully in battery-powered devices.

## LoRa Antenna

*   The choice of antenna (chip antenna, PCB trace antenna, or external SMA/u.FL connectorized antenna) will depend on factors like:
    *   Desired range and link budget.
    *   Physical enclosure constraints.
    *   Cost.
    *   Regulatory certification requirements (e.g., for specific antenna gain).
*   Proper impedance matching (typically 50 ohms) is crucial for optimal LoRa performance.

This covers the initial hardware selection rationale. More specific part numbers and detailed interface plans will be developed as the project progresses.
