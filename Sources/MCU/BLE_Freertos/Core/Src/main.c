/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "ipcc.h"
#include "irtim.h"
#include "memorymap.h"
#include "rf.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include <stm32wbxx_hal.h>
#include <stm32wb5mm_dk.h>
#include <stm32wb5mm_dk_env_sensors.h>
#include <stm32wb5mm_dk_motion_sensors.h>
#include <stm32wb5mm_dk_errno.h>
#include <stm32wb5mm_dk_conf.h>
#include <stm32wb5mm_dk_bus.h>
#include <stm32wb5mm_dk_lcd.h>
#include <stm32_lcd.h>
#include <stm32wbxx.h>
#include <stm32wbxx_it.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_WDGT.h"
#include "tsk_HMI.h"
#include "tsk_TEMP.h"
#include "tsk_MAIN.h"
#include "tsk_TOF.h"
#include "tsk_IR_ATL.h"
#include "tsk_BLE.h"

#include "ir_common.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static tskWDGT_TaskParam_t          param_WDGT = {0};
static tskHMI_TaskParam_t           param_HMI = {0};
static tskTEMP_TaskParam_t          param_TEMP = {0};
static tskTOF_TaskParam_t           param_TOF = {0};
static tskMAIN_TaskParam_t          param_MAIN = {0};
static tskIRATL_TaskParam_t         param_IRATL = {0};
static tskBLE_TaskParam_t           param_BLE = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void vSetupOsExchangeObject(void);
static void vStartTasks(void);
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
  /* Config code for STM32_WPAN (HSE Tuning must be done before system clock configuration) */
  MX_APPE_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* IPCC initialisation */
  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_MEMORYMAP_Init();
//  MX_SPI1_Init();
//  MX_I2C3_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_IRTIM_Init();
  MX_RF_Init();
  /* USER CODE BEGIN 2 */
  // BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);

  // (void)BSP_ENV_SENSOR_Init(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE);
  // (void)BSP_ENV_SENSOR_Enable(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE);

  // int i = 0;
  // do
  // {
	//   float temperature;
	//   temperature = 0;
	//   (void)BSP_ENV_SENSOR_GetValue(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE, &temperature);
	//   char temp[64];
	//   sprintf(temp,"Temp=%f", temperature);
	//   UTIL_LCD_DisplayStringAt(0, 37, (uint8_t *)temp, CENTER_MODE);
	//   BSP_LCD_Refresh(0);
	//   i++;
  // } while(i < 10);

  vStartTasks();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Init code for STM32_WPAN */
  MX_APPE_Init();
  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

static void vSetupOsExchangeObject(void)
{
    xQueueHandle tmpQueueHandle = 0;
    xSemaphoreHandle tmpSemaHandle = 0;

    /***********************************************************************************/
    /* Create Queue for HeartBeat transmission to Watchdog */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_HB_TO_WDG, sizeof(tskCommon_Hb_t));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_WDGT.queueHbFromMonitoredTask = tmpQueueHandle;
    param_HMI.queue_hb_to_watchdog = tmpQueueHandle;
    param_TEMP.queue_hb_to_watchdog = tmpQueueHandle;
    param_TOF.queue_hb_to_watchdog = tmpQueueHandle;
    param_MAIN.queue_hb_to_watchdog = tmpQueueHandle;
    param_IRATL.queue_hb_to_watchdog = tmpQueueHandle;
    param_BLE.queue_hb_to_watchdog = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_HB_TO_WDG);

    /***********************************************************************************/
    /* Create Queue for message to HMI transmission */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_STS_TO_HMI, sizeof(tskHMI_msg_fdbk_msg_t));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_HMI.queue_hmi_feedback = tmpQueueHandle;
    param_MAIN.queue_hmi_feedback = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_STS_TO_HMI);

    /***********************************************************************************/
    /* Create Queue for btn to hmi transmission */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_BTN_TO_HMI, sizeof(Button_TypeDef));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_HMI.queue_hmi_btn = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_BTN_TO_HMI);

    /***********************************************************************************/
    /* Create Queue for tof to hmi transmission */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_TOF_TO_HMI, sizeof(tskTOF_queue_msg_t));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_HMI.queue_hmi_range = tmpQueueHandle;
    param_TOF.queue_tof_distance = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_TOF_TO_HMI);

    /***********************************************************************************/
    /* Create Queue for hmi to main transmission */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_HMI_SETPOINT, sizeof(tskCommon_hmi_stpt_msg_t));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_HMI.queue_hmi_setpoint = tmpQueueHandle;
    param_MAIN.queue_hmi_setpoint = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_HMI_SETPOINT);

    /***********************************************************************************/
    /* Create Queue for message to HMI transmission */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_TO_IR_ATL, sizeof(tskMAIN_clim_stpt_to_IR_msg_t));
    /* Check for an error */
    if (tmpQueueHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* Store the value to tasks parameters */
    param_IRATL.queue_to_ir_atl = tmpQueueHandle;
    param_MAIN.queue_to_ir = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_TO_IR_ATL);

    /***********************************************************************************/
    /* Create Semaphore for delaying start of TEMP_sensor looping */
    tmpSemaHandle = xSemaphoreCreateCounting(TSK_CNFG_SEMA_LENGTH_START_TEMP_SNSR, 0);
    /* Check for an error */
    if (tmpSemaHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    /* TODO Feed the semaphore to temp task to wait upon prior starting */
    
    /* Add Semaphore to registry */
    vQueueAddToRegistry(tmpSemaHandle, TSK_CNFG_SEMA_NAME_START_TEMP_SNSR);

    /***********************************************************************************/
    /* Create Semaphore for feeding temperature to main */
    tmpQueueHandle = xQueueCreate(TSK_CNFG_QUEUE_LENGTH_TEMP_TO_MAIN, sizeof(tskTEMP_queue_msg_t));
    /* Check for an error */
    if (tmpSemaHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    param_MAIN.queue_temperature_sensor = tmpQueueHandle;
    param_TEMP.queue_temperature_sts = tmpQueueHandle;
    /* Add Semaphore to registry */
    vQueueAddToRegistry(tmpSemaHandle, TSK_CNFG_QUEUE_NAME_TEMP_TO_MAIN);

    /***********************************************************************************/
    /* Create Mutex to be used by all tasks using I2C (as the driver does not support concurent accesses ) */
    tmpSemaHandle = xSemaphoreCreateMutex();
    /* Check for an error */
    if (tmpSemaHandle == 0)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }
    param_TEMP.mutex_i2c = tmpSemaHandle;
    param_TOF.mutex_i2c = tmpSemaHandle;
    /* Add Semaphore to registry */
    vQueueAddToRegistry(tmpSemaHandle, TSK_CNFG_MUTEX_NAME_I2C);

    /* TODO create more semaphore here*/
}

static void vStartTasks(void)
{
    portBASE_TYPE ret = 0;

    /* Setup OS exchange object */
    vSetupOsExchangeObject();

    /* By default every task shall be monitored */
    param_WDGT.excludeMonitoredTskBitmask = 0;
    /* Indicate how many task shall be monitored */
    param_WDGT.u8_nbMonitoredTasks = TSK_CNFG_MONITORED_ID_CNT;

    /* WDGT thread */
    ret = xTaskCreate(vWDGT_task, (const char * const) TSK_CNFG_NAME_WDGT,
                        TSK_CNFG_STACKSIZE_WDGT, &param_WDGT, TSK_CNFG_PRIORITY_WDGT,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* HMI thread */
    ret = xTaskCreate(vHMI_task, (const char * const) TSK_CNFG_NAME_HMI,
                        TSK_CNFG_STACKSIZE_HMI, &param_HMI, TSK_CNFG_PRIORITY_HMI,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* TEMP thread */
    ret = xTaskCreate(vTEMP_task, (const char * const) TSK_CNFG_NAME_TEMP,
                        TSK_CNFG_STACKSIZE_TEMP, &param_TEMP, TSK_CNFG_PRIORITY_TEMP,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* TOF thread */
    ret = xTaskCreate(vTOF_task, (const char * const) TSK_CNFG_NAME_TOF,
                        TSK_CNFG_STACKSIZE_TOF, &param_TOF, TSK_CNFG_PRIORITY_TOF,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* MAIN thread */
    ret = xTaskCreate(vMAIN_task, (const char * const) TSK_CNFG_NAME_MAIN,
                        TSK_CNFG_STACKSIZE_MAIN, &param_MAIN, TSK_CNFG_PRIORITY_MAIN,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* IR_ATL thread */
    ret = xTaskCreate(vIRATL_task, (const char * const) TSK_CNFG_NAME_IR_ATL,
                        TSK_CNFG_STACKSIZE_IRATL, &param_IRATL, TSK_CNFG_PRIORITY_IRATL,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

    /* BLE thread */
    ret = xTaskCreate(vBLE_task, (const char * const) TSK_CNFG_NAME_BLE,
                        TSK_CNFG_STACKSIZE_BLE, &param_IRATL, TSK_CNFG_PRIORITY_BLE,
                        (xTaskHandle *) NULL);

    /* Check whether task was created */
    if (ret != pdTRUE)
    {
        /* Reset the board, try to allow a fix from bootloader */
        NVIC_SystemReset();
    }

#if (TSK_CNFG_DISABLE_HW_WATCHDOG == 0)
    /* TODO Start Watchdog just before entering scheduler */
#else
    #warning Watchdog disabled
#endif
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  HAL_TIM_PeriodElapsedCallback_IR(htim);
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
