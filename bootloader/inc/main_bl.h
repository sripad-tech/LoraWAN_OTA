#ifndef MAIN_BL_H
#define MAIN_BL_H

#include <stdint.h> // For uint32_t, etc.

// Application start address - this should match what's in ota_app_flags.h
// but defined here for bootloader's independent reference if needed.
// It's better if both app and bootloader get this from a common source
// or the bootloader reads it from a fixed location set by the application.
// For now, we'll assume it's known and consistent.
// #define APPLICATION_START_ADDRESS  0x08008000UL // Matches OTA_APP_MAIN_ADDRESS

/**
 * @brief Main entry point for the Bootloader.
 */
void main_bl(void);

#endif // MAIN_BL_H
