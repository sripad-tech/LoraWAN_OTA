/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stsafe_a110_core.h"
#include "lorawan_app.h"
#include "lesc_app.h"
#include "lora_app_conf.h" // For APP_LOG and LESC_APP_PORT
#include "ota_app_flags.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_JOIN_WAIT_ITERATIONS 200 // Approx 20 seconds with 100ms delay
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static void Trigger_OTA_Update(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  if (stsafe_init(&hi2c1) == STSAFE_OK) {
    APP_LOG(("MAIN: STSAFE Initialized successfully.\n"));
    for(int i=0; i<10; ++i) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(100); }
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); HAL_Delay(100);

    if (lesc_app_init(&hi2c1) != 0) {
        APP_LOG(("MAIN: LESC App Initialization FAILED.\n"));
        while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(30); }
    } else {
        APP_LOG(("MAIN: LESC App Initialized.\n"));
        if (lesc_app_start_ecdh_key_generation() == 0) {
            APP_LOG(("MAIN: LESC ephemeral key generation successful.\n"));
            uint8_t commissioning_payload[LESC_COMMISSIONING_PAYLOAD_SIZE];
            size_t actual_payload_len = 0;
            if (lesc_app_prepare_commissioning_payload(commissioning_payload, sizeof(commissioning_payload), &actual_payload_len) == 0) {
                APP_LOG(("MAIN: LESC Commissioning Payload prepared (%lu bytes).\n", (unsigned long)actual_payload_len));
                
                APP_LOG(("MAIN: Initializing LoRaWAN App for LESC communication...\n"));
                LoRaWAN_App_Init(); 

                APP_LOG(("MAIN: Waiting for LoRaWAN Join to complete before sending LESC request...\n"));
                uint32_t join_wait_counter = 0;
                while(LmHandlerJoinStatus() != LORAMAC_HANDLER_SUCCESS && join_wait_counter < MAX_JOIN_WAIT_ITERATIONS) {
                    LoRaWAN_App_Process(); 
                    HAL_Delay(100); 
                    join_wait_counter++;
                }
                
                if (LmHandlerJoinStatus() == LORAMAC_HANDLER_SUCCESS) {
                    APP_LOG(("MAIN: LoRaWAN Join successful. Sending LESC Commissioning Request...\n"));
                    if (LoRaWAN_App_Send_Commissioning_Request(commissioning_payload, actual_payload_len) != 0) {
                        APP_LOG(("MAIN: FAILED to send LESC Commissioning Request.\n"));
                         while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(60); } // LESC Send fail
                    } else {
                        APP_LOG(("MAIN: LESC Commissioning Request sent. Waiting for (simulated) response in OnRxData.\n"));
                    }
                } else {
                     APP_LOG(("MAIN: LoRaWAN Join FAILED or timed out. Cannot send LESC request.\n"));
                     while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(70); } // Join fail blink
                }

            } else { 
                APP_LOG(("MAIN: FAILED to prepare LESC Commissioning Payload.\n"));
                while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(15); }
            }
        } else { 
            APP_LOG(("MAIN: LESC ephemeral key generation FAILED.\n"));
            while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(20); }
        }
    }
  } else { 
    APP_LOG(("MAIN: STSAFE Initialization FAILED.\n"));
    while(1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(50); }
  }

  APP_LOG(("MAIN: Entering main processing loop.\n"));
  /* USER CODE END 2 */

  /* Infinite loop */
  uint32_t main_loop_counter = 0;
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    LoRaWAN_App_Process();

    main_loop_counter++;

    if (main_loop_counter > 500000 && (main_loop_counter % 10000 ==0) ) { // Reduced frequency of check after initial period
        APP_LOG(("MAIN: OTA trigger check (loop counter = %lu).\n", main_loop_counter));
        // This condition needs to be more realistic, e.g. based on a flag set in OnRxData
        // For now, keeping it as a long-running counter for simulation.
        // Trigger_OTA_Update(); // OTA trigger temporarily disabled for LESC flow focus
    }
    
    if (main_loop_counter % 2000 == 0) { // Slowed down heartbeat
         HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); 
    }
    /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON; 
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11; // 48 MHz
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; 
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                                |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                                |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI; 
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC; 
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2MSK_DISABLE;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) { Error_Handler(); }
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) { Error_Handler(); }
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) { Error_Handler(); }
}

/* USER CODE BEGIN 4 */
static void Trigger_OTA_Update(void) {
    APP_LOG(("Trigger_OTA_Update: Setting OTA magic value at 0x%08lX to 0x%08lX\n",
             (uint32_t)OTA_UPDATE_MAGIC_ADDRESS, (uint32_t)OTA_UPDATE_MAGIC_VALUE));
    volatile uint32_t *p_ota_magic = OTA_UPDATE_MAGIC_ADDRESS;
    *p_ota_magic = OTA_UPDATE_MAGIC_VALUE;
    APP_LOG(("Trigger_OTA_Update: OTA magic value set. Requesting system reset.\n"));
    HAL_Delay(100);
    NVIC_SystemReset();
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  APP_LOG(("!!! Application Error_Handler() called !!!\n")); 
  __disable_irq();
  while (1) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); HAL_Delay(50); }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  APP_LOG(("Assert failed: file %s on line %lu\n", (char *)file, line));
  Error_Handler(); 
}
#endif /* USE_FULL_ASSERT */
