#include "ota_app.h"
#include "ota_app_flags.h" // For staging area defs and metadata struct
#include "lora_app_conf.h"   // For APP_LOG (or include a local debug print)
#include <string.h>         // For memcpy (should not be needed if only logging)

// For APP_LOG - ensure this is consistent
// lora_app_conf.h should provide APP_LOG if LORAWAN_DEBUG_APP is set.
// This local definition is a fallback.
#ifndef APP_LOG
    #if 1 // Enable logging for OTA app debugging by default if not globally defined
    #include <stdio.h>
    #define APP_LOG(PRINTF_ARGS) do { printf PRINTF_ARGS; } while(0)
    #else
    #define APP_LOG(PRINTF_ARGS)
    #endif
#endif

// --- Dummy Firmware Image ---
// This is a very small, simple placeholder for a firmware image.
// In a real scenario, this would be received over LoRaWAN.
// Its content is arbitrary for this simulation.
static const uint8_t dummy_firmware_image[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0xFE, 0xED, 0xFA, 0xCE, 0xAB, 0xCD, 0xEF, 0x12,
    // Add a few more bytes to make it slightly larger than metadata
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
    0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
    0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x00,
    0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87,
};
#define DUMMY_FIRMWARE_VERSION 0x01000100 // v1.0.1.0

// Simple checksum function (sum of bytes)
static uint32_t calculate_checksum(const void* data, size_t len) {
    const uint8_t* p_data = (const uint8_t*)data;
    uint32_t checksum = 0;
    for (size_t i = 0; i < len; ++i) {
        checksum += p_data[i];
    }
    return checksum;
}

int ota_app_simulate_download_and_stage_firmware(void) {
    APP_LOG(("OTA_APP: Starting simulated firmware download and staging...\n"));

    if (sizeof(dummy_firmware_image) + OTA_FIRMWARE_IMAGE_OFFSET > OTA_STAGING_AREA_MAX_SIZE) {
        APP_LOG(("OTA_APP: ERROR - Dummy firmware image is too large for the staging area!\n"));
        return -1;
    }

    // 1. Simulate Flash Erase of the staging area
    APP_LOG(("OTA_APP: (SIMULATED) Erasing Flash staging area (0x%08lX - 0x%08lX).\n",
             (unsigned long)OTA_METADATA_ADDRESS,
             (unsigned long)(OTA_FIRMWARE_IMAGE_ADDRESS + sizeof(dummy_firmware_image) -1) ));
    // HAL_FLASH_Unlock();
    // FLASH_PageErase(...) for all necessary pages
    // HAL_FLASH_Lock();


    // 2. Prepare Metadata
    ota_firmware_metadata_t metadata;
    metadata.firmware_size = sizeof(dummy_firmware_image);
    metadata.firmware_version = DUMMY_FIRMWARE_VERSION;
    metadata.firmware_checksum = calculate_checksum(dummy_firmware_image, sizeof(dummy_firmware_image));

    // Calculate checksum of the metadata fields themselves
    // (excluding the metadata_checksum field itself initially)
    uint32_t temp_meta_checksum_calc_base_addr = (uint32_t)(uintptr_t)&metadata;
    size_t metadata_checksum_len = (uint32_t)(uintptr_t)&metadata.metadata_checksum - temp_meta_checksum_calc_base_addr;
    metadata.metadata_checksum = calculate_checksum(&metadata, metadata_checksum_len);


    APP_LOG(("OTA_APP: Prepared Metadata:\n"));
    APP_LOG(("  Firmware Size: %lu bytes\n", (unsigned long)metadata.firmware_size));
    APP_LOG(("  Firmware Version: 0x%08lX\n", (unsigned long)metadata.firmware_version));
    APP_LOG(("  Firmware Checksum: 0x%08lX\n", (unsigned long)metadata.firmware_checksum));
    APP_LOG(("  Metadata Checksum: 0x%08lX\n", (unsigned long)metadata.metadata_checksum));


    // 3. Simulate Writing Metadata to Flash Staging Area
    APP_LOG(("OTA_APP: (SIMULATED) Writing metadata to Flash @ 0x%08lX.\n", (unsigned long)OTA_METADATA_ADDRESS));
    // In a real app:
    // HAL_FLASH_Unlock();
    // for (size_t i = 0; i < sizeof(ota_firmware_metadata_t) / sizeof(uint32_t); ++i) {
    //    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, // Or appropriate type
    //                      OTA_METADATA_ADDRESS + (i * sizeof(uint32_t)), // Assuming uint32_t writes
    //                      ((uint32_t*)&metadata)[i]);
    // }
    // HAL_FLASH_Lock();


    // 4. Simulate Writing Firmware Image to Flash Staging Area
    APP_LOG(("OTA_APP: (SIMULATED) Writing dummy firmware image (%lu bytes) to Flash @ 0x%08lX.\n",
             (unsigned long)sizeof(dummy_firmware_image),
             (unsigned long)OTA_FIRMWARE_IMAGE_ADDRESS));
    // In a real app (example using double word programming):
    // HAL_FLASH_Unlock();
    // for (size_t i = 0; i < sizeof(dummy_firmware_image) / sizeof(uint64_t); ++i) { // Assuming 64-bit alignment for image data
    //    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
    //                           OTA_FIRMWARE_IMAGE_ADDRESS + (i * sizeof(uint64_t)),
    //                           ((uint64_t*)dummy_firmware_image)[i]) != HAL_OK) {
    //        // Handle error
    //        break;
    //    }
    // }
    // // Handle any remaining bytes if not perfectly aligned
    // HAL_FLASH_Lock();

    APP_LOG(("OTA_APP: Simulated firmware download and staging completed successfully.\n"));
    return 0; // Success
}
