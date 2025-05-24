#ifndef LORAWAN_APP_H
#define LORAWAN_APP_H

#include "stm32wlxx_hal.h" // For basic types

/**
 * @brief Initializes the LoRaWAN application and stack.
 *        This function should be called once at startup.
 */
void LoRaWAN_App_Init(void);

/**
 * @brief Runs the LoRaWAN application process.
 *        This function should be called periodically in the main loop
 *        to process LoRaWAN events and timers.
 */
void LoRaWAN_App_Process(void);

/**
 * @brief Sends data over LoRaWAN.
 * @param app_port Application port number.
 * @param buffer Pointer to the data buffer.
 * @param length Length of the data in bytes.
 * @param confirmed Whether to send a confirmed or unconfirmed message.
 * @return 0 on success, negative on error.
 */
int LoRaWAN_App_Send(uint8_t app_port, uint8_t* buffer, uint8_t length, uint8_t confirmed);

/**
 * @brief Sends the LESC commissioning request payload over LoRaWAN.
 * @param payload Pointer to the payload buffer.
 * @param len Length of the payload.
 * @return 0 on success, negative on error.
 */
int LoRaWAN_App_Send_Commissioning_Request(uint8_t* payload, uint8_t len);

#endif // LORAWAN_APP_H
