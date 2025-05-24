# Application Core Logic - Data Acquisition and Transmission

This document outlines the core application logic for the CPV Field Logger, focusing on sensor data acquisition, processing, packaging, and scheduling transmission.

## 1. Location and Structure

*   The main application logic will reside primarily in the `src/app_core/` directory.
*   Key files might include:
    *   `app_core_main.c`/`.h`: Main entry point and loop for the application logic.
    *   `sensor_interface.c`/`.h`: Abstraction layer for interacting with various sensor drivers.
    *   `data_packer.c`/`.h`: Logic for formatting data into the chosen LoRaWAN payload format.
    *   `event_scheduler.c`/`.h` (Optional): A simple scheduler for periodic tasks like sensor reading.

## 2. Sensor Data Acquisition

*   **Abstraction:** The `sensor_interface.c` module will provide a unified way to read data from different sensors. Specific sensor drivers will be located in `drivers/components/sensor_xyz/` and will implement a common interface expected by `sensor_interface.c`.
*   **Placeholder Functions:** Initially, `sensor_interface.c` will contain placeholder functions for expected sensor types. These will be filled in as actual sensor drivers are developed. Examples:
    ```c
    // In sensor_interface.h
    typedef struct {
        float TBD_voltage;       // Volts
        float TBD_current;       // Amps
        float TBD_temperature_panel; // Celsius
        float TBD_temperature_ambient; // Celsius
        float TBD_irradiance_dni;    // W/m^2 (Direct Normal Irradiance)
        float TBD_irradiance_ghi;    // W/m^2 (Global Horizontal Irradiance)
        // Add other relevant CPV parameters: tracker status, etc.
    } cpv_data_t;

    bool read_cpv_data(cpv_data_t* data);
    ```
*   **Error Handling:** Each sensor reading function should return a status indicating success or failure, allowing the application to handle sensor malfunctions.

## 3. Data Packaging and Formatting

*   **Format:** Cayenne Low Power Payload (LPP) is recommended for its efficiency and ease of integration with many LoRaWAN network servers and IoT platforms. It provides a compact, standardized way to send various sensor data types.
*   **Implementation:** The `data_packer.c` module will contain functions to take the raw `cpv_data_t` structure and serialize it into a Cayenne LPP byte buffer.
    ```c
    // In data_packer.h
    #define MAX_LPP_BUFFER_SIZE 51 // Max payload for LoRaWAN at lowest data rates

    /**
     * @brief Packs CPV data into Cayenne LPP format.
     * @param data Pointer to the CPV data structure.
     * @param buffer Buffer to store the LPP payload.
     * @param buffer_size Max size of the buffer.
     * @return Actual size of the LPP payload in bytes, or 0 on error.
     */
    uint8_t pack_data_cayenne_lpp(const cpv_data_t* data, uint8_t* buffer, uint8_t buffer_size);
    ```
*   **Flexibility:** While Cayenne LPP is the default, the design should allow for custom binary formats if extreme data density is required, though this adds complexity on the server-side for decoding.

## 4. Main Application Loop Outline

The `app_core_main.c` will host the primary operational logic:

```c
// Simplified main loop in app_core_main.c
void application_core_run(void) {
    // Initialize all necessary modules: LoRaWAN, sensors, security, etc.
    // ...

    // Join LoRaWAN network (blocking or non-blocking with state machine)
    // ...

    for (;;) {
        // 1. Enter low-power mode until next scheduled event
        //    (e.g., using RTC wakeup or timer interrupt)
        //    power_manage_enter_low_power();

        // 2. On wakeup:
        //    bool send_data_now = false;
        //    if (event_is_due(SENSOR_READ_EVENT)) {
        //        cpv_data_t sensor_readings;
        //        if (read_cpv_data(&sensor_readings)) {
        //            uint8_t lpp_buffer[MAX_LPP_BUFFER_SIZE];
        //            uint8_t lpp_payload_size = pack_data_cayenne_lpp(&sensor_readings, lpp_buffer, sizeof(lpp_buffer));
        //
        //            if (lpp_payload_size > 0) {
        //                // Attempt to send data via LoRaWAN
        //                lorawan_send_request(lpp_buffer, lpp_payload_size, LORAWAN_UNCONFIRMED_MSG); // Or confirmed
        //                send_data_now = true; // Track if a send was attempted
        //            } else {
        //                // Handle data packing error
        //            }
        //        } else {
        //            // Handle sensor read error (e.g., log, schedule retry)
        //        }
        //        reschedule_event(SENSOR_READ_EVENT);
        //    }

        // 3. Process LoRaWAN events (join status, TX done, RX data)
        //    lorawan_process_events(); // This would internally handle callbacks from the stack

        // 4. Handle other application tasks (e.g., responding to downlink commands, OTA checks)
        //    if (lorawan_has_received_downlink()) {
        //        process_downlink_command(lorawan_get_downlink_data());
        //    }

        // 5. If no data was sent and no pressing LoRaWAN activity, go back to sleep.
        //    If data was sent, the LoRaWAN stack might keep the MCU awake for RX windows.
        //    The power_manage_enter_low_power() should be smart enough or coordinated
        //    with the LoRaWAN stack's state.
    }
}
```

## 5. Interaction with Other Modules

*   **LoRaWAN Module (`app_lorawan/`):**
    *   The core logic will invoke `app_lorawan` functions to request data transmission.
    *   It will receive status updates (e.g., join success, TX confirmation, downlink data) from `app_lorawan` via callbacks or a message queue.
*   **Security Module (`app_security/`):**
    *   May be invoked if any sensor data or application parameters need to be encrypted/decrypted or signed using the STSAFE (though this is less common for typical sensor data over LoRaWAN, which relies on network/application layer security).
*   **Power Management Module (`power_manager.c`/`.h` - to be defined):**
    *   The application core will frequently request entry into low-power modes.
    *   The power management module will coordinate with the LoRaWAN stack and other peripherals to ensure safe entry and exit from these modes.
*   **OTA Update Module (`app_ota/`):**
    *   Downlink commands processed by the application core might trigger the OTA update process managed by `app_ota`.

This structure provides a modular way to manage data acquisition, processing, and transmission, allowing for easier development, testing, and maintenance.
```
