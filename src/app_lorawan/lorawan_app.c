#include "lorawan_app.h"
#include "lora_app_conf.h" // For credentials, config, and LESC_APP_PORT
#include "LmHandler.h"          // Main LoRaWAN Handler
#include "Commissioning.h"      // For Commissioning parameters like EUI, Keys
#include "timer_if.h"         // Placeholder for timer interface
#include "lesc_app.h"         // Include LESC app header for STSAFE_P256_PUBLIC_KEY_SIZE and lesc_app_process_join_server_public_key

// APP_LOG is defined in lora_app_conf.h

// --- Application Data ---
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
// Structure: { Port, BufferSize (max), NbBytes (actual), *Buffer }
static LmHandlerAppData_t AppData = {LORAWAN_APP_PORT, LORAWAN_APP_DATA_BUFFER_MAX_SIZE, 0, AppDataBuffer};


// --- Timer for sending data ---
static UTIL_TIMER_Object_t TxTimer;
static uint8_t HelloMsg[] = "Hello LoRaWAN!";

// --- Simulated Join Server Ephemeral Public Key (65 bytes: 0x04 || X_coord (32B) || Y_coord (32B)) ---
// This is a dummy key for simulation purposes.
static const uint8_t simulated_js_ephemeral_public_key[STSAFE_P256_PUBLIC_KEY_SIZE] = {
    0x04, // Uncompressed format tag
    0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, // X (dummy)
    0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, // X (dummy)
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, // Y (dummy)
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0  // Y (dummy)
};

// State variable to track if we are waiting for commissioning response
static uint8_t g_waiting_for_commissioning_response = 0;


// --- Forward declarations for callbacks ---
static void OnMacProcessNotify(void);
static void OnNwkData(LmHandlerNwkData_t *nwkData, LmHandlerAppData_t *appData);
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);
static void OnTxData(LmHandlerTxParams_t *params);
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);
static void SendTxData(void);


// --- LoRaWAN Handler Callbacks Structure ---
static LmHandlerCallbacks_t LmHandlerCallbacks = {
    .GetBatteryLevel = NULL, 
    .GetTemperature = NULL,  
    .OnMacProcessNotify = OnMacProcessNotify,
    .OnNwkData = OnNwkData,
    .OnJoinRequest = OnJoinRequest,
    .OnTxData = OnTxData,
    .OnRxData = OnRxData
};

// --- LoRaWAN Handler Parameters ---
#ifndef ACTIVE_REGION
  #if defined(LORAMAC_REGION_EU868)
    #define ACTIVE_REGION LORAMAC_REGION_EU868
  #elif defined(LORAMAC_REGION_US915)
    #define ACTIVE_REGION LORAMAC_REGION_US915
  #else
    #warning "No active region defined, defaulting to EU868"
    #define ACTIVE_REGION LORAMAC_REGION_EU868 
  #endif
#endif
#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS CLASS_A
#endif
#ifndef LORAWAN_DEFAULT_DATA_RATE
#define LORAWAN_DEFAULT_DATA_RATE DR_0 
#endif
#ifndef LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY 0 
#endif

static LmHandlerParams_t LmHandlerParams = {
    .ActiveRegion = ACTIVE_REGION,
    .DefaultClass = LORAWAN_DEFAULT_CLASS,
    .AdrEnable = LORAWAN_ADR_STATE,
    .TxDatarate = LORAWAN_DEFAULT_DATA_RATE, 
    .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY,
    .PublicNetworkEnable = LORAWAN_PUBLIC_NETWORK
};

// External Error_Handler from main.c
extern void Error_Handler(void);

void LoRaWAN_App_Init(void) {
    APP_LOG(("LoRaWAN_App_Init: Starting Initialization\n"));
    g_waiting_for_commissioning_response = 0; // Initialize state

    if (LmHandlerInit(&LmHandlerCallbacks, &LmHandlerParams) != LORAMAC_HANDLER_SUCCESS) {
        APP_LOG(("LoRaWAN_App_Init: LmHandlerInit failed!\n"));
        Error_Handler();
    }

    uint8_t dev_eui[] = LORAWAN_DEVICE_EUI;
    uint8_t join_eui[] = LORAWAN_JOIN_EUI;
    uint8_t app_key[] = LORAWAN_APP_KEY;
    uint8_t nwk_key[] = LORAWAN_NWK_KEY;

    LmHandlerSetDevEUI(dev_eui);
    LmHandlerSetJoinEUI(join_eui);
    LmHandlerSetAppKey(app_key);
    LmHandlerSetNwkKey(nwk_key);
    APP_LOG(("LoRaWAN_App_Init: Credentials Set\n"));

    LmHandlerJoin(ACTIVATION_TYPE_OTAA);
    APP_LOG(("LoRaWAN_App_Init: Join procedure started (OTAA).\n"));

    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, SendTxData, NULL);
}

void LoRaWAN_App_Process(void) {
    LmHandlerProcess();
}

int LoRaWAN_App_Send(uint8_t app_port, uint8_t* buffer, uint8_t length, uint8_t confirmed) {
    if (length == 0 || buffer == NULL) { return -1; }
    if (length > LORAWAN_APP_TX_MAX_PAYLOAD_SIZE) { 
        APP_LOG(("LoRaWAN_App_Send: Payload too large (%d bytes, max %d)\n", length, LORAWAN_APP_TX_MAX_PAYLOAD_SIZE)); 
        return -2; 
    }
    if (length > AppData.BufferSize) {
        APP_LOG(("LoRaWAN_App_Send: Payload length (%d) exceeds internal AppData.Buffer max size (%d)\n", length, AppData.BufferSize));
        return -4;
    }

    AppData.Port = app_port; 
    AppData.NbBytes = length; 
    memcpy(AppData.Buffer, buffer, length);

    LmHandlerMsgTypes_t msg_type = confirmed ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;
    if (LmHandlerSend(&AppData, msg_type, NULL, false) == LORAMAC_HANDLER_SUCCESS) {
        APP_LOG(("LoRaWAN_App_Send: Message queued (Port: %d, Size: %d)\n", app_port, length)); 
        return 0;
    } else { 
        APP_LOG(("LoRaWAN_App_Send: Failed to queue.\n")); 
        return -3; 
    }
}

int LoRaWAN_App_Send_Commissioning_Request(uint8_t* payload, uint8_t len) {
    if (len == 0 || payload == NULL) {
        return -1;
    }
     if (len > AppData.BufferSize) { // Check against max buffer capacity
        APP_LOG(("LoRaWAN: Commissioning payload too large for AppData.Buffer (%d vs %d max)\n", len, AppData.BufferSize));
        return -4;
    }

    AppData.Port = LESC_APP_PORT; 
    AppData.NbBytes = len;
    memcpy(AppData.Buffer, payload, len);

    if (LmHandlerSend(&AppData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, false) == LORAMAC_HANDLER_SUCCESS) {
        APP_LOG(("LoRaWAN: Sent LESC Commissioning Request (%d bytes on port %d).\n", len, LESC_APP_PORT));
        g_waiting_for_commissioning_response = 1; 
        return 0;
    } else {
        APP_LOG(("LoRaWAN: FAILED to send LESC Commissioning Request.\n"));
        g_waiting_for_commissioning_response = 0;
        return -3;
    }
}

static void OnMacProcessNotify(void) { APP_LOG(("- OnMacProcessNotify\n")); }
static void OnNwkData(LmHandlerNwkData_t *nwkData, LmHandlerAppData_t *appData) { APP_LOG(("- OnNwkData received.\n")); }

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams) {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS) {
        APP_LOG(("- OnJoinRequest: Joined successfully! Datarate: DR_%d\n", joinParams->Datarate));
        // Do NOT start TxTimer here if we are doing LESC first.
        // Main will trigger LESC request. If LESC is not used, main can start TxTimer.
    } else {
        APP_LOG(("- OnJoinRequest: Join failed! Status: %d. Retrying...\n", joinParams->Status));
    }
}

static void OnTxData(LmHandlerTxParams_t *params) {
    // Check if the TX was for the commissioning request
    if (g_waiting_for_commissioning_response && params->AppData->Port == LESC_APP_PORT) {
         APP_LOG(("- OnTxData: LESC Commissioning Request TX Done. Datarate: DR_%d, Status %d\n", params->Datarate, params->Status));
         // Do not set g_waiting_for_commissioning_response = 0 here, wait for Rx or timeout.
         // Do not restart TxTimer here.
    } else { // Normal application data TX
        if (params->IsMcpsConfirm == 1) { 
            APP_LOG(("- OnTxData: AppData TX Confirmed. Datarate: DR_%d, ACK %s\n", params->Datarate, params->AckReceived ? "Rx" : "NoRx"));
        } else { 
            APP_LOG(("- OnTxData: AppData TX Done (Unconfirmed). Datarate: DR_%d\n", params->Datarate));
        }
        UTIL_TIMER_Start(&TxTimer); // Restart timer for next periodic app data send
    }
}

static void OnRxData(LmHandlerAppData_t *appDataRx, LmHandlerRxParams_t *params) {
    APP_LOG(("- OnRxData: Received on Port %d, %d bytes, RSSI %d, SNR %d\n",
             appDataRx->Port, appDataRx->NbBytes, params->Rssi, params->Snr));

    if (appDataRx->NbBytes > 0) {
        APP_LOG(("  Data: "));
        for (uint8_t i = 0; i < appDataRx->NbBytes; i++) {
            printf("%02X ", appDataRx->Buffer[i]); 
        }
        printf("\n"); 
    }

    if (g_waiting_for_commissioning_response && appDataRx->Port == LESC_APP_PORT) {
        APP_LOG(("LoRaWAN: Received response on LESC Port %d. Simulating it's JS Ephemeral PubKey.\n", LESC_APP_PORT));
        // In a real scenario, parse appDataRx->Buffer to get the JS public key.
        // Here, we use the hardcoded simulated_js_ephemeral_public_key.
        lesc_app_process_join_server_public_key(simulated_js_ephemeral_public_key, sizeof(simulated_js_ephemeral_public_key));
        g_waiting_for_commissioning_response = 0; // Processed response, ready for ECDH
        // Optionally, start the periodic "Hello" timer now if LESC is considered "done" for this phase
        // UTIL_TIMER_Start(&TxTimer); 
    } else {
        APP_LOG(("LoRaWAN: Received application data on port %d.\n", appDataRx->Port));
        // Handle other application downlink data here
    }
}

static void SendTxData(void) { // This sends the periodic "Hello"
    APP_LOG(("SendTxData: Timer expired, attempting to send 'Hello'.\n"));
    if (!LmHandlerIsBusy() && LmHandlerJoinStatus() == LORAMAC_HANDLER_SUCCESS && !g_waiting_for_commissioning_response) {
         // Ensure AppData is correctly set for this message
         AppData.Port = LORAWAN_APP_PORT;
         AppData.NbBytes = sizeof(HelloMsg) - 1;
         memcpy(AppData.Buffer, HelloMsg, AppData.NbBytes);
         LoRaWAN_App_Send(AppData.Port, AppData.Buffer, AppData.NbBytes, LORAMAC_HANDLER_UNCONFIRMED_MSG);
    } else {
        APP_LOG(("SendTxData: LoRaWAN busy, join not complete, or waiting for LESC response. Skipping 'Hello' TX.\n"));
        UTIL_TIMER_Start(&TxTimer); // Reschedule to try later
    }
}

// Note: Placeholder UTIL_TIMER_ and LmHandler functions from previous steps are assumed
// to be replaced by actual ST Middleware implementations. If not, they would need to be
// re-added here for the code to be "runnable" in a fully simulated environment without the stack.
// For this exercise, we assume the ST stack provides these.
```

**6. Modify `Core/Src/main.c`:**
This step applies the logic to initialize LoRaWAN, wait for join, then send the LESC commissioning request.
The `main.c` file was already modified in the prompt. I will apply this version.
