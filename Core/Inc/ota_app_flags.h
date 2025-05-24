#ifndef OTA_APP_FLAGS_H
#define OTA_APP_FLAGS_H

#include <stdint.h>

// --- RAM Flag for Bootloader Communication ---
#define OTA_UPDATE_MAGIC_ADDRESS    ((volatile uint32_t*)0x20003FF0) // Example RAM address
#define OTA_UPDATE_MAGIC_VALUE      0xBOOTLOAD /* 0x424F4F54 - B O O T */

// --- OTA Firmware Staging Area Definitions (Conceptual for 256KB Flash STM32WL55) ---
// These MUST be aligned with actual Flash page sizes and linker script.
#define OTA_APP_MAIN_ADDRESS        0x08008000UL // Start address of the main application
#define OTA_APP_MAIN_MAX_SIZE       (96 * 1024)  // 96KB

#define OTA_STAGING_AREA_ADDRESS    0x08020000UL // Start address of the firmware staging area
#define OTA_STAGING_AREA_MAX_SIZE   (96 * 1024)  // 96KB, must be >= OTA_APP_MAIN_MAX_SIZE

// --- Firmware Metadata Structure ---
// This metadata will be stored at the beginning of the OTA_STAGING_AREA_ADDRESS.
// The actual firmware image will follow this structure in the staging area.
typedef struct {
    uint32_t firmware_size;       // Actual size of the firmware image in bytes
    uint32_t firmware_checksum;   // e.g., CRC32 or simple sum of the firmware image
    uint32_t firmware_version;    // Optional version number
    uint32_t metadata_checksum;   // Checksum of the preceding metadata fields (size, fw_checksum, version)
                                  // to verify metadata integrity itself.
} ota_firmware_metadata_t;

// Address where the ota_firmware_metadata_t structure is stored.
// For this example, it's at the very beginning of the staging area.
#define OTA_METADATA_ADDRESS        OTA_STAGING_AREA_ADDRESS

// The actual firmware image will be stored after the metadata.
#define OTA_FIRMWARE_IMAGE_OFFSET   sizeof(ota_firmware_metadata_t)
#define OTA_FIRMWARE_IMAGE_ADDRESS  (OTA_STAGING_AREA_ADDRESS + OTA_FIRMWARE_IMAGE_OFFSET)


#endif // OTA_APP_FLAGS_H
