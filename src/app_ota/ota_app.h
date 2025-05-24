#ifndef OTA_APP_H
#define OTA_APP_H

#include <stdint.h> // For uint32_t etc.

/**
 * @brief Simulates the download of a new firmware image, calculates its metadata,
 *        and conceptually writes both to the Flash staging area.
 *
 * @retval 0 on successful simulation.
 * @retval -1 on failure (e.g., dummy image too large, checksum error).
 */
int ota_app_simulate_download_and_stage_firmware(void);

#endif // OTA_APP_H
