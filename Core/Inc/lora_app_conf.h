#ifndef __LORA_APP_CONF_H__
#define __LORA_APP_CONF_H__

// --- Debugging ---
// Enable/disable basic debug messages
#define LORAWAN_DEBUG_APP     1 // Enable app-level debug messages

#if LORAWAN_DEBUG_APP
#include <stdio.h> // For basic printf, map to SWO or UART in real app
// In a real app, UTIL_ADV_TRACE_Printf or a similar macro would be defined by ST's utilities (e.g. in sys_debug.h)
// For this simulation, we provide a simple printf wrapper. Ensure newlines are in the format string.
// The PRINTF_ARGS must be a double-parenthesized argument list, e.g., APP_LOG(("Value: %d\n", val));
#define APP_LOG(PRINTF_ARGS) do { printf PRINTF_ARGS; } while(0)
#else
#define APP_LOG(PRINTF_ARGS)
#endif

// Region: EU868. This define is typically used by the stack.
#define LORAMAC_REGION_EU868 1

// OTAA activation mode
#define OVER_THE_AIR_ACTIVATION 1

// Adaptive Data Rate
#define LORAWAN_ADR_STATE LORAWAN_ADR_ON

// Public Network
#define LORAWAN_PUBLIC_NETWORK LORAWAN_PUBLIC_NETWORK_ON

// Application port
#define LORAWAN_APP_PORT                            2

// How often to send data (in milliseconds)
#define APP_TX_DUTYCYCLE                            15000 // 15 seconds - FOR TESTING ONLY, respect duty cycle in real apps

// Max buffer size for application data
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            64

// Max payload size (this is an estimate, actual depends on DR)
#define LORAWAN_APP_TX_MAX_PAYLOAD_SIZE             51


// --- Dummy LoRaWAN Credentials - REPLACE WITH REAL VALUES ---
// These should be unique for each device.
// For EU868, DevEUI and AppEUI are 8 bytes, AppKey is 16 bytes.

// Device EUI (LSB) - IMPORTANT: MUST BE UNIQUE FOR EACH DEVICE
#define LORAWAN_DEVICE_EUI { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }

// Application EUI / Join EUI (LSB) - Provided by Network Server
#define LORAWAN_JOIN_EUI   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }

// Application Key (MSB) - Provided by Network Server
#define LORAWAN_APP_KEY    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }

// Network Key (MSB) - Only for LoRaWAN 1.1+, for 1.0.x AppKey is used for NwkSKey derivation too.
// For simplicity, we'll assume 1.0.x behavior or that AppKey can serve as NwkKey if stack needs it distinctly.
#define LORAWAN_NWK_KEY    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }

// LESC Application Port (LoRaWAN spec recommends 223 for commissioning, or use a proprietary one)
#define LESC_APP_PORT                               1 // Using 1 for this example, could be 223.

#endif /* __LORA_APP_CONF_H__ */
