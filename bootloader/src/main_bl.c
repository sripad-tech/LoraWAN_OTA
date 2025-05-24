#include "main_bl.h"
#include "ota_app_flags.h" // For OTA flags, addresses, and metadata structure
#include "stm32wlxx_hal.h"   // For HAL types, NVIC_SystemReset etc.

#if 1
#include <stdio.h>
#define BL_LOG(PRINTF_ARGS) do { printf PRINTF_ARGS; } while(0)
#else
#define BL_LOG(PRINTF_ARGS)
#endif

typedef void (*pFunction)(void);

// --- Static function prototypes ---
static void BL_Platform_Init(void);
static uint8_t is_valid_application(uint32_t app_address);
static void jump_to_application(uint32_t app_address);
static uint8_t perform_ota_update(void); // New function for OTA

// Simple checksum function (sum of bytes) - bootloader needs its own or a shared one
// For simulation, this doesn't need to actually read flash.
// It will be used conceptually with dummy/assumed data.
static uint32_t bl_calculate_checksum(const void* data, size_t len) {
    // In a real scenario, this would read from Flash memory block by block.
    // For simulation with no actual Flash reads, this function is more conceptual
    // when called by perform_ota_update unless we pass it a dummy image pointer.
    // For now, let's assume perform_ota_update will just simulate a checksum value.
    if (data == NULL) return 0; // Should not happen with real data
    const uint8_t* p_data = (const uint8_t*)data;
    uint32_t checksum = 0;
    for (size_t i = 0; i < len; ++i) {
        checksum += p_data[i];
    }
    return checksum;
}


void main_bl(void) {
    BL_LOG(("\nBOOTLOADER: Starting...\n")); // Add a newline for better log separation

    BL_Platform_Init();

    // Check for OTA Update Request
    volatile uint32_t *p_ota_magic = OTA_UPDATE_MAGIC_ADDRESS;
    if (*p_ota_magic == OTA_UPDATE_MAGIC_VALUE) {
        BL_LOG(("BOOTLOADER: OTA Update Request detected (Magic: 0x%08lX).\n", (unsigned long)*p_ota_magic));

        // Clear the magic number to prevent re-entry on failure/next boot
        *p_ota_magic = 0x00000000;
        BL_LOG(("BOOTLOADER: OTA Magic cleared from RAM.\n"));

        if (perform_ota_update()) {
            BL_LOG(("BOOTLOADER: OTA Update successful. Resetting system...\n"));
            HAL_Delay(100); // Allow log to flush
            NVIC_SystemReset();
            // Should not reach here
        } else {
            BL_LOG(("BOOTLOADER: OTA Update FAILED. Attempting to boot existing application.\n"));
            // Fall through to boot existing application
        }
    } else {
        BL_LOG(("BOOTLOADER: No OTA Update Request found (Value @ 0x%08lX is 0x%08lX).\n",
                 (unsigned long)OTA_UPDATE_MAGIC_ADDRESS, (unsigned long)*p_ota_magic));
    }

    BL_LOG(("BOOTLOADER: Checking main application integrity...\n"));
    if (is_valid_application(OTA_APP_MAIN_ADDRESS)) {
        BL_LOG(("BOOTLOADER: Main application appears valid. Jumping to application @ 0x%08lX.\n",
                 (unsigned long)OTA_APP_MAIN_ADDRESS));
        jump_to_application(OTA_APP_MAIN_ADDRESS);
    } else {
        BL_LOG(("BOOTLOADER: ERROR - Main application invalid or not found!\n"));
        volatile uint32_t i = 0;
        while (1) { /* Error Loop */ for(i=0; i<1000000; ++i); }
    }
    BL_LOG(("BOOTLOADER: ERROR - Failed to jump or returned unexpectedly.\n"));
    while(1);
}

static void BL_Platform_Init(void) {
    HAL_Init(); // Simplified init
    BL_LOG(("BOOTLOADER: Minimal platform init complete (simulated).\n"));
}

// is_valid_application and jump_to_application remain as in the previous step
static uint8_t is_valid_application(uint32_t app_address) {
#define RAM_START              0x20000000UL
#define RAM_SIZE_WL55JC_64KB   (64 * 1024)
#define RAM_END                (RAM_START + RAM_SIZE_WL55JC_64KB)
#define APP_FLASH_END          (OTA_APP_MAIN_ADDRESS + OTA_APP_MAIN_MAX_SIZE)
    uint32_t app_initial_msp = *((volatile uint32_t*)app_address);
    uint32_t app_reset_handler_addr = *((volatile uint32_t*)(app_address + 4));
    BL_LOG(("BOOTLOADER: App Validation - Initial MSP: 0x%08lX, Reset Handler: 0x%08lX\n",
             (unsigned long)app_initial_msp, (unsigned long)app_reset_handler_addr));
    if (app_initial_msp < RAM_START || app_initial_msp > RAM_END) { BL_LOG(("BOOTLOADER: App Validation - MSP out of RAM.\n")); return 0; }
    if (app_reset_handler_addr < OTA_APP_MAIN_ADDRESS || app_reset_handler_addr >= APP_FLASH_END) { BL_LOG(("BOOTLOADER: App Validation - Reset Handler out of App Flash.\n")); return 0; }
    if ((app_reset_handler_addr & 1) == 0) { BL_LOG(("BOOTLOADER: App Validation - Reset Handler not Thumb.\n")); return 0; }
    BL_LOG(("BOOTLOADER: App Validation - Basic checks passed.\n"));
    return 1;
}

static void jump_to_application(uint32_t app_address) {
    BL_LOG(("BOOTLOADER: Preparing to jump to application...\n"));
    HAL_DeInit();
    __disable_irq();
    uint32_t app_initial_msp = *((volatile uint32_t*)app_address);
    __set_MSP(app_initial_msp);
    BL_LOG(("BOOTLOADER: MSP set to 0x%08lX.\n", (unsigned long)app_initial_msp));
    uint32_t app_reset_handler_addr = *((volatile uint32_t*)(app_address + 4));
    pFunction app_reset_handler = (pFunction)app_reset_handler_addr;
    BL_LOG(("BOOTLOADER: Jumping to Reset Handler @ 0x%08lX...\n", (unsigned long)app_reset_handler_addr));
    app_reset_handler();
    while(1); // Should not reach here
}

/**
 * @brief Performs the OTA update process (simulated).
 *        Reads metadata, verifies firmware, and conceptually copies it.
 * @return 1 if OTA process is successful (simulated), 0 on failure.
 */
static uint8_t perform_ota_update(void) {
    BL_LOG(("BOOTLOADER: perform_ota_update - Starting simulated OTA process...\n"));

    // 1. Simulate Reading Metadata from Staging Area
    //    In a real bootloader, this would be:
    //    ota_firmware_metadata_t received_metadata;
    //    memcpy(&received_metadata, (void*)OTA_METADATA_ADDRESS, sizeof(ota_firmware_metadata_t));
    //    For simulation, we'll assume the metadata is "read" and create a dummy one
    //    that matches what the ota_app would have "written".
    //    To make this simulation more meaningful, we need some expected values.
    //    The `ota_app.c` defined `DUMMY_FIRMWARE_VERSION`.
    //    The size is `sizeof(dummy_firmware_image)` used in `ota_app.c`.
    //    The checksums were calculated there.
    //    This bootloader doesn't have access to `dummy_firmware_image` directly to recalculate.
    //    So, for this simulation, we'll assume metadata is read and proceed to "verify" it.

    BL_LOG(("BOOTLOADER: (SIMULATED) Reading metadata from Flash @ 0x%08lX.\n", (unsigned long)OTA_METADATA_ADDRESS));
    // Let's assume we have a way to know the expected size and checksums for the DUMMY image
    // that ota_app.c prepared. For this simulation, we can't directly access ota_app.c's static data.
    // So, the verification will be highly conceptual.
    // A real bootloader would read the metadata structure from OTA_METADATA_ADDRESS.
    // For simulation:
    ota_firmware_metadata_t staged_metadata; // This would be populated by a Flash read
    // Here, a real bootloader would: flash_read(OTA_METADATA_ADDRESS, &staged_metadata, sizeof(staged_metadata));
    // We will just log this.
    BL_LOG(("BOOTLOADER: (SIMULATED) Metadata structure would be read here.\n"));
    // Let's invent some values for the metadata that would have been written by the app,
    // so we can simulate checking them.
    // This part is tricky because the bootloader doesn't know the app's dummy image content.
    // For the sake of simulation, let's assume the metadata check passes.
    // A real metadata check:
    // uint32_t calculated_meta_checksum = bl_calculate_checksum(&staged_metadata, offsetof(ota_firmware_metadata_t, metadata_checksum));
    // if (calculated_meta_checksum != staged_metadata.metadata_checksum) {
    //    BL_LOG(("BOOTLOADER: Metadata checksum verification FAILED!\n"));
    //    return 0; // Metadata corrupted
    // }
    // BL_LOG(("BOOTLOADER: Metadata checksum OK.\n"));
    // For this SIMULATION, assume metadata is OK.
    // We also need firmware_size from this conceptual metadata to "verify" the image.
    // Let's assume a plausible size for logging purposes.
    uint32_t simulated_firmware_size_from_metadata = 64; // Must match roughly dummy_firmware_image size from ota_app.c
    BL_LOG(("BOOTLOADER: (SIMULATED) Assumed firmware_size from metadata: %lu bytes.\n", (unsigned long)simulated_firmware_size_from_metadata));


    // 2. Simulate Firmware Verification
    BL_LOG(("BOOTLOADER: (SIMULATED) Verifying firmware image at 0x%08lX (size %lu bytes).\n",
             (unsigned long)OTA_FIRMWARE_IMAGE_ADDRESS, (unsigned long)simulated_firmware_size_from_metadata));
    // In a real bootloader:
    // uint32_t calculated_firmware_checksum = bl_calculate_checksum_from_flash(OTA_FIRMWARE_IMAGE_ADDRESS, staged_metadata.firmware_size);
    // if (calculated_firmware_checksum != staged_metadata.firmware_checksum) {
    //    BL_LOG(("BOOTLOADER: Firmware image checksum verification FAILED!\n"));
    //    return 0; // Firmware corrupted
    // }
    BL_LOG(("BOOTLOADER: (SIMULATED) Firmware checksum verification PASSED.\n"));


    // 3. Simulate Firmware Copy
    BL_LOG(("BOOTLOADER: (SIMULATED) Copying %lu bytes from staging area (0x%08lX) to main app area (0x%08lX).\n",
             (unsigned long)simulated_firmware_size_from_metadata,
             (unsigned long)OTA_FIRMWARE_IMAGE_ADDRESS,
             (unsigned long)OTA_APP_MAIN_ADDRESS));
    // In a real bootloader:
    // HAL_FLASH_Unlock();
    // Erase main application flash pages.
    // Program main application flash pages with data from staging area.
    // HAL_FLASH_Lock();
    // This is a complex and critical operation.
    BL_LOG(("BOOTLOADER: (SIMULATED) Firmware copy successful.\n"));

    return 1; // Success
}
