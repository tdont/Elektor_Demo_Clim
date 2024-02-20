/******************************************************************************
 * COPYRIGHT (C) Thibaut DONTAIL 2024. 
 *
 * This file is part of ELEKTO_DEMO_CLIM.
 *
 * ELEKTO_DEMO_CLIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ELEKTO_DEMO_CLIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 *
 * The code hereafter might call dependencies from the FreeRTOS (C) software. 
 * along with ELEKTO_DEMO_CLIM.  If not, see <http://www.gnu.org/licenses/>.
 * The FreeRTOS software is licensed under the MIT license.
 *****************************************************************************/

/******************** FILE HEADER ********************************************/
/**\addtogroup ModuleName_Internal_Documentation
 * \{ */
/**\addtogroup SubModuleName
 * \{ */

/**
 * \file tsk_TOF.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 18, 2024
 */

 /**
 * \cond INTERNAL_DOC
 *
 * \mainpage Internal documentation of module
 *
 * \section Description
 * This module does what you want him to do....
 *
 * \section Features
 * The main features of the module are described here.
 *
 * \endcond
 */

/******************** INCLUDES ***********************************************/
#include "tsk_TOF.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include <stm32wb5mm_dk.h>
#include <stm32wb5mm_dk_bus.h>
#include <vl53l0x_def.h>
#include <vl53l0x_api.h>
#include <vl53l0x_tof.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_HMI.h"



/******************** CONSTANTS OF MODULE ************************************/
#define PROXIMITY_I2C_ADDRESS            0x53U
#define VL53L0X_ID                       0xEEAAU
#define VL53L0X_XSHUT_PIN                GPIO_PIN_6
#define VL53L0X_XSHUT_GPIO_PORT          GPIOB
#define VL53L0X_XSHUT_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define VL53L0X_XSHUT_GPIO_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static VL53L0X_Dev_t UserDev =
{
  .I2cHandle = &hbus_i2c3,
  .I2cDevAddr = PROXIMITY_I2C_ADDRESS
};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
static void VL53L0X_PROXIMITY_Init(void);
static uint16_t VL53L0X_PROXIMITY_GetDistance(void);
static void VL53L0X_PROXIMITY_MspInit(void);

/******************** API FUNCTIONS ******************************************/
void vTOF_task(void *pv_param_task)
{
    tskTOF_TaskParam_t* task_param = NULL;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_TOF;
    heartbeat.hb_counter = 0;

    static tskTOF_queue_msg_t msg_tof = {0};
    msg_tof.distance_max_mm = TOF_SENSOR_MAX_DISTANCE_MM;
    msg_tof.distance_min_mm = TOF_SENSOR_MIN_DISTANCE_MM;

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskTOF_TaskParam_t*) pv_param_task;

    /* TODO fix dummy wait (Temperature sensor will have enabled I2C)*/
    vTaskDelay(1500 / portTICK_RATE_MS);

    VL53L0X_PROXIMITY_Init();

    while (1) /* Task loop */
    {
        /* Wait sufficient time prior to read another sensor value */
    	  vTaskDelay(TOF_SENSOR_ACQ_MS/portTICK_RATE_MS);

        uint16_t distance = 0;

        /* Take semaphore to protect access of I2C */
        portBASE_TYPE ret = xSemaphoreTake(task_param->mutex_i2c, 1);
        if (ret == pdPASS)
        {
          /* Read TOF sensor */
          distance = VL53L0X_PROXIMITY_GetDistance();

          /* Give semaphore back */
          xSemaphoreGive(task_param->mutex_i2c);

          if(distance > msg_tof.distance_max_mm)
          {
            msg_tof.distance_mm = msg_tof.distance_max_mm;
          }
          if(distance < msg_tof.distance_min_mm)
          {
            msg_tof.distance_mm = msg_tof.distance_min_mm;
          }
          xQueueSend(task_param->queue_tof_distance, &msg_tof, 0); /* Don't wait on queue*/
        }

        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > TOF_HB_SEND_TIME_MS)
        {
            /* Increment HB counter */
            heartbeat.hb_counter = heartbeat.hb_counter + 1;

            /* Send heartbeat */
            xQueueSend(task_param->queue_hb_to_watchdog, &heartbeat, 0); /* Don't wait on queue*/

            /* Store sending time */
            hb_sending_tick = xTaskGetTickCount();
        }
    }
}

/******************** LOCAL FUNCTIONS ****************************************/
/**
  * @brief  VL53L0X proximity sensor Initialization.
  */
static void VL53L0X_PROXIMITY_Init(void)
{
  uint16_t vl53l0x_id = 0; 
  VL53L0X_DeviceInfo_t VL53L0X_DeviceInfo;
  
  /* Initialize IO interface */
  //STM32WB5MM_DK_I2C_Init();
  
  VL53L0X_PROXIMITY_MspInit();
  
  memset(&VL53L0X_DeviceInfo, 0, sizeof(VL53L0X_DeviceInfo_t));
  
  if (VL53L0X_ERROR_NONE == VL53L0X_GetDeviceInfo(&UserDev, &VL53L0X_DeviceInfo))
  {  
    if (VL53L0X_ERROR_NONE == VL53L0X_RdWord(&UserDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, (uint16_t *) &vl53l0x_id))
    {
      if (vl53l0x_id == VL53L0X_ID)
      {
        if (VL53L0X_ERROR_NONE == VL53L0X_DataInit(&UserDev))
        {
          UserDev.Present = 1;
          SetupSingleShot(&UserDev);
        }
      }
    }
  }
}

/**
  * @brief  Get distance from VL53L0X proximity sensor.
  * @retval Distance in mm
  */
static uint16_t VL53L0X_PROXIMITY_GetDistance(void)
{
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  
  VL53L0X_PerformSingleRangingMeasurement(&UserDev, &RangingMeasurementData);
  
  return RangingMeasurementData.RangeMilliMeter;  
}

/**
  * @brief  VL53L0X proximity sensor Msp Initialization.
  */
static void VL53L0X_PROXIMITY_MspInit(void)
{
  GPIO_InitTypeDef gpio_config = {0};
  
  VL53L0X_XSHUT_GPIO_CLK_ENABLE();
  
  /* Configure GPIO pin : VL53L0X_XSHUT_PIN */
  gpio_config.Pin   = VL53L0X_XSHUT_PIN;
  gpio_config.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull  = GPIO_PULLUP;
  gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(VL53L0X_XSHUT_GPIO_PORT, &gpio_config);
  
  HAL_GPIO_WritePin(VL53L0X_XSHUT_GPIO_PORT, VL53L0X_XSHUT_PIN, GPIO_PIN_SET);
  
  vTaskDelay(1000 / portTICK_RATE_MS);
}

/**\} */
/**\} */

/* EOF */
