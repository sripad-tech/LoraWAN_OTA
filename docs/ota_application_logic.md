# OTA Update Application-Side Logic

This document outlines the application-side logic for Over-The-Air (OTA) firmware updates. This logic resides in the main application and is responsible for managing the download and initial verification of a new firmware image before control is handed over to the bootloader.

## 1. Location and Structure

*   The OTA update application logic will primarily reside in `src/app_ota/`.
*   Key files might include:
    *   `ota_handler.c`/`.h`: Manages the overall OTA process, state machine, and interaction with LoRaWAN and flash storage.
    *   `firmware_downloader.c`/`.h`: Handles the reception of firmware chunks over LoRaWAN.
    *   `firmware_verifier.c`/`.h`: Performs initial checks on the downloaded image.

## 2. Update Notification

*   **Mechanism:** The primary mechanism for update notification will be a **LoRaWAN downlink message**.
    *   This message could be a simple command indicating an update is available, followed by separate messages for metadata (version, size, hash/signature).
    *   Alternatively, a more structured approach using FUOTA (Firmware Update Over The Air) protocols defined by the LoRa Alliance (e.g., Application Layer Clock Synchronization, Remote Multicast Setup, Fragmented Data Block Transport) could be adopted if the chosen LoRaWAN stack and network server support them. For initial simplicity, a basic custom command scheme will be assumed.
*   **Content of Notification:** The notification message (or sequence of messages) should ideally provide:
    *   New Firmware Version.
    *   Total Firmware Size.
    *   Cryptographic Hash (e.g., SHA256) of the complete firmware image. This hash is separate from the signature verified by the bootloader but provides an integrity check for the download process itself.
    *   Block size for download (if applicable).
    *   Source/URL for download (less common for LoRaWAN, usually the LNS pushes data).

## 3. Firmware Download Process

This is the most challenging part over LoRaWAN due to low data rates, duty cycle limitations, and potential for packet loss.

*   **Chunking/Fragmentation:** The firmware image will be downloaded in small chunks.
    *   The size of each chunk must be compatible with LoRaWAN payload limits (factoring in headers and any protocol overhead).
    *   Each chunk will need a sequence number.
*   **Transport:**
    *   **Unicast Downlinks:** For Class A devices, chunks are sent in response to uplinks. This is slow but power-efficient for the device. The application may need to send periodic "keep-alive" or "request next chunk" uplinks.
    *   **Multicast Downlinks (FUOTA):** If supported (requires Class B or Class C device operation temporarily, or specific multicast windows), this is more efficient for updating multiple devices. This plan initially assumes Class A, so unicast is the baseline.
*   **Reliability:**
    *   Each chunk should have a CRC or hash to ensure its integrity upon reception.
    *   The application must request retransmission of missing or corrupted chunks.
    *   A timeout mechanism should be in place for the overall download process.
*   **`firmware_downloader.c` Role:**
    *   Manages the state of the download (e.g., expecting chunk `N`).
    *   Requests chunks from the server (implicitly by opening RX windows, or explicitly via an uplink).
    *   Receives chunks, verifies their integrity (e.g., CRC), and writes them to the staging area in flash.
    *   Handles retransmissions and out-of-order chunks (if the protocol allows).

## 4. Storage of Downloaded Image

*   **Location:** The downloaded firmware image is stored in the **Firmware Staging Area** in flash memory, as defined in the `bootloader_design.md`.
*   **Flash Operations:**
    *   The `ota_handler.c` will interface with a flash driver module.
    *   The staging area must be erased before starting a new download.
    *   Chunks are written sequentially to the staging area.

## 5. Application-Side Verification

Once the entire firmware image is downloaded:

*   **Integrity Check:** The `firmware_verifier.c` module will calculate a cryptographic hash (e.g., SHA256) of the entire image stored in the staging area.
*   **Comparison:** This calculated hash is compared against the expected hash received in the update notification message.
*   **Outcome:**
    *   **Match:** The downloaded image is considered intact (at least from a download perspective). The application can now proceed to signal the bootloader.
    *   **Mismatch:** The download is considered corrupt. The application should discard the image (or mark it as invalid), potentially notify the server of the failure, and await further instructions or a retry.

## 6. Signaling the Bootloader

If application-side verification is successful:

1.  **Set Update Flag:** The `ota_handler.c` sets the "update request flag" (e.g., in shared RAM or a dedicated flash location) that the bootloader checks on startup. This flag signals the bootloader to attempt an update from the staging area.
2.  **Store Metadata (Optional but Recommended):** Information like the new version number or the location/size of the image in the staging area can be written to a shared memory location for the bootloader to use.
3.  **System Reset:** The application then triggers a software system reset.

    ```c
    // In ota_handler.c (simplified)
    void trigger_bootloader_for_update(void) {
        // 1. Set the magic number/flag in shared RAM/flash
        //    SHARED_MEM->update_request_flag = BOOTLOADER_UPDATE_REQUEST_MAGIC;
        //    SHARED_MEM->firmware_size_in_staging = downloaded_firmware_size;
        //    SHARED_MEM->firmware_hash_in_staging = calculated_firmware_hash; // For bootloader to double check if needed

        // 2. Ensure all data is written to memory/flash (cache coherency if necessary)
        //    __DSB(); // Data Synchronization Barrier

        // 3. Trigger a system reset
        //    NVIC_SystemReset();
    }
    ```

## 7. State Management

The `ota_handler.c` will implement a state machine to manage the OTA process:

*   `OTA_IDLE`: Awaiting update notification.
*   `OTA_DOWNLOADING`: Receiving and storing firmware chunks.
*   `OTA_VERIFYING`: Performing hash check on the downloaded image.
*   `OTA_READY_FOR_BOOTLOADER`: Verification successful, ready to signal bootloader.
*   `OTA_ERROR`: An error occurred (download failed, verification failed).

This application-side logic ensures that the image is correctly received and has a basic level of integrity before involving the bootloader, which then performs the critical cryptographic signature verification and the actual swap of firmware.
```
