#ifndef OTA_APP_FLAGS_H
#define OTA_APP_FLAGS_H

#include <stdint.h>

// Define a specific RAM address for the bootloader flag.
// This address MUST be in a section of RAM that is not initialized by
// the C runtime startup code of EITHER the main application OR the bootloader
// when it starts up after a reset triggered by the main application.
// A dedicated "NOINIT" RAM section defined in the linker script is the robust way.
// For this example, we pick an address manually.
// STM32WL55xx has RAM starting at 0x20000000.
// Example: Use an address towards the end of a RAM bank, e.g., 0x20003FF0.
// IMPORTANT: This is a simplification. A production system needs a robust shared memory mechanism.
#define OTA_UPDATE_MAGIC_ADDRESS    ((uint32_t*)0x20003FF0) // Example address
#define OTA_UPDATE_MAGIC_VALUE      0xBOOTLOAD /* 0x424F4F54 - B O O T */

// Define a structure if more data needs to be passed (optional for now)
// typedef struct {
//     uint32_t magic_value;
//     uint32_t firmware_size;
//     uint32_t firmware_crc;
// } ota_shared_data_t;
// #define OTA_SHARED_DATA_ADDRESS ((ota_shared_data_t*)0x20003FF0)


#endif // OTA_APP_FLAGS_H
