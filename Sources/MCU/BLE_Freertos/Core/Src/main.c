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
#include "memorymap.h"
#include "rf.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include <tsk_WDGT.h>
#include <tsk_common.h>
#include <tsk_config.h>

#include "stm32wbxx_hal.h"
#include "stm32wb5mm_dk.h"
#include "stm32wb5mm_dk_env_sensors.h"
#include "stm32wb5mm_dk_motion_sensors.h"
#include "stm32wb5mm_dk_errno.h"
#include "stm32wb5mm_dk_conf.h"
#include "stm32wb5mm_dk_bus.h"
#include "stm32wb5mm_dk_lcd.h"
#include "stm32_lcd.h"
#include "stm32wbxx.h"
#include "stm32wbxx_it.h"
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
static tskWDGT_TaskParam_t          stParamWDGT = {0};
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
  //MX_SPI1_Init();
  //MX_I2C3_Init();
  MX_RF_Init();
  /* USER CODE BEGIN 2 */
  BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);
  BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);

  uint32_t x_size, y_size;

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  UTIL_LCD_SetFuncDriver(&LCD_Driver); /* SetFunc before setting device */
  UTIL_LCD_SetDevice(0);            /* SetDevice after funcDriver is set */
  BSP_LCD_DisplayOn(0);

  UTIL_LCD_SetFont(&Font12);
  /* Set the LCD Text Color */
  UTIL_LCD_SetTextColor(SSD1315_COLOR_WHITE);
  UTIL_LCD_SetBackColor(SSD1315_COLOR_BLACK);
  BSP_LCD_Clear(0,SSD1315_COLOR_BLACK);
  BSP_LCD_Refresh(0);

  UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"Elektor demo clim", CENTER_MODE);
  BSP_LCD_Refresh(0);

  (void)BSP_ENV_SENSOR_Init(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE);
  (void)BSP_ENV_SENSOR_Enable(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE);

  int i = 0;
  do
  {
	  float temperature;
	  temperature = 0;
	  (void)BSP_ENV_SENSOR_GetValue(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE, &temperature);
	  char temp[64];
	  sprintf(temp,"Temp=%f", temperature);
	  UTIL_LCD_DisplayStringAt(0, 37, (uint8_t *)temp, CENTER_MODE);
	  BSP_LCD_Refresh(0);
	  i++;
  } while(i < 10);

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
    stParamWDGT.queueHbFromMonitoredTask = tmpQueueHandle;
    /* Add queue to registry */
    vQueueAddToRegistry(tmpQueueHandle, TSK_CNFG_QUEUE_NAME_HB_TO_WDG);

    /* TODO create more semaphore here*/
}

static void vStartTasks(void)
{
    portBASE_TYPE ret = 0;

    /* Setup OS exchange object */
    vSetupOsExchangeObject();

    /* By default every task shall be monitored */
    stParamWDGT.excludeMonitoredTskBitmask = 0;
    /* Indicate how many task shall be monitored */
    stParamWDGT.u8_nbMonitoredTasks = TSK_CNFG_MONITORED_ID_CNT;

    /* WDGT thread */
    ret = xTaskCreate(vWDGT_task, (const char * const) TSK_CNFG_NAME_WDGT,
                        TSK_CNFG_STACKSIZE_WDGT, &stParamWDGT, TSK_CNFG_PRIORITY_WDGT,
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
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

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