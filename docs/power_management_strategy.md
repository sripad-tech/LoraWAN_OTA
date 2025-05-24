# Initial Power Management Strategy

This document outlines the initial power management strategy for the STM32 LoRaWAN CPV Field Logger. The primary goal is to maximize battery life by minimizing energy consumption during idle periods and optimizing power usage during active periods.

## 1. Core Principles

*   **Maximize Time in Low-Power Modes:** The device should spend the vast majority of its time in the deepest possible sleep mode from which it can still meet its functional requirements (e.g., wake up for scheduled sensor readings).
*   **Wake-Up on Demand:** Utilize interrupts (RTC, GPIOs for sensor events, LoRaWAN events) to wake the MCU only when necessary.
*   **Peripheral Power Gating:** Disable clocks and power to peripherals that are not in active use.
*   **Optimize Active Mode:** When the MCU is awake, execute tasks efficiently and return to sleep as quickly as possible.
*   **Voltage Scaling (If Applicable):** Reduce core voltage at lower frequencies if supported and beneficial, though for STM32WL, selecting the right low-power mode is often more impactful.

## 2. Low-Power Modes (STM32WL)

The STM32WL series offers several low-power modes. The most relevant for this application are:

*   **Stop 2 Mode:**
    *   **Description:** Offers significant power savings while retaining SRAM content, RTC, and allowing wakeup from various sources including LPTIM, RTC, and IWDG, as well as external interrupts and LoRa radio events. CPU clock is stopped.
    *   **Use Case:** This will likely be the primary low-power mode between sensor readings and LoRaWAN transmissions. It provides a good balance between power saving and quick wakeup time.
*   **Standby Mode:**
    *   **Description:** Achieves very low power consumption. SRAM and register contents are lost (except for those in backup domain). Wakeup is possible via RTC, NRST, WKUP pins.
    *   **Use Case:** Could be considered for very long sleep periods if application state can be fully reconstructed or if data persistence is handled externally (e.g., in STSAFE or non-volatile memory before entering Standby). May require more complex state restoration.
*   **Shutdown Mode:**
    *   **Description:** Lowest power mode. Similar to Standby but with fewer wakeup sources.
    *   **Use Case:** Likely too deep for this application unless there are extremely long periods of inactivity and a very specific wakeup trigger (e.g., external button press for maintenance).

**Initial Target:** **Stop 2 mode** will be the primary target for regular sleep cycles.

## 3. Peripheral Power Management

*   **Clock Gating:** Before entering a low-power mode, disable the clock supply to unused peripherals via the RCC (Reset and Clock Control) registers. STM32Cube HAL provides functions for this (e.g., `__HAL_RCC_ADC_CLK_DISABLE()`, `__HAL_RCC_SPI1_CLK_DISABLE()`).
*   **GPIO Configuration:**
    *   Configure unused GPIO pins as analog inputs with no pull-up/pull-down resistors to prevent floating inputs and leakage currents.
    *   For pins connected to external components, ensure they are in a defined state that minimizes power draw of those components (e.g., disabling chip select lines).
*   **Sensor Power:** If possible, power down sensors completely between readings using a load switch controlled by a GPIO, especially if sensors have significant quiescent current.
*   **Flash Memory:** Put the Flash memory into power-down mode during Stop modes (this is often handled automatically by the MCU when entering Stop modes, but should be verified).

## 4. Real-Time Clock (RTC) for Wakeup

*   **Role:** The RTC will be the primary mechanism for waking the device from Stop 2 mode for scheduled tasks like:
    *   Periodic sensor readings.
    *   Scheduled LoRaWAN uplinks (if not event-driven).
*   **Configuration:** The RTC will be clocked by a low-speed external oscillator (LSE - 32.768 kHz) for accuracy and low power.
*   **Wakeup Timer:** The RTC's wakeup timer interrupt will be configured to trigger MCU wakeup.

## 5. Coordination with LoRaWAN Activity

*   **LoRa Radio Sleep:** The STM32WL's integrated LoRa radio has its own sleep modes. The LoRaWAN stack (I-CUBE-LRWAN) is responsible for managing these in coordination with the MCU's sleep modes.
*   **RX Windows (Class A):** After an uplink, the device must stay awake or in a light sleep mode (e.g., Low Power Run or Sleep mode, not Stop 2) to listen during the RX1 and RX2 receive windows. The power management module must account for this. The LoRaWAN stack typically provides signals or callbacks indicating when it's safe to enter deeper sleep.
*   **`UTIL_SEQ_Idle()` / `UTIL_LPM_EnterLowPower()`:** ST's utilities often provide sequencer and low-power manager functionalities that help coordinate tasks and ensure that all conditions are met before entering deep sleep. These should be utilized.

## 6. Power Management Module (`src/power_manager.c`/`.h`)

*   **Responsibilities:**
    *   Provide functions to safely enter and exit chosen low-power modes.
    *   Coordinate with different parts of the application (especially the LoRaWAN stack) to determine when it's safe to sleep.
    *   Manage peripheral clock states.
    *   Configure RTC wakeups.
*   **Example Interface:**
    ```c
    // In power_manager.h
    void power_manager_init(void);
    void power_manager_enter_low_power(void); // Enters the most appropriate low-power mode
    void power_manager_request_wakeup(uint32_t sleep_duration_ms); // Configures RTC
    bool power_manager_is_safe_to_sleep(void); // Checks conditions (e.g., LoRaWAN state)
    ```

## 7. Measurement and Optimization

*   **Tools:** Use tools like an oscilloscope with current probes, STM32CubeMonitor-Power, or specialized power analyzer tools to measure actual current consumption in different modes.
*   **Iterative Refinement:** Power management is an iterative process. After initial implementation, measure, identify hotspots, and refine the strategy.

This initial strategy provides a framework. Detailed implementation will involve careful configuration of STM32Cube HAL/LL functions and coordination with the LoRaWAN middleware.
```
