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
 * \file tsk_TEMP.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 14, 2024
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
#include "tsk_TEMP.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stm32wb5mm_dk.h>
#include <stm32wb5mm_dk_env_sensors.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_HMI.h"


/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** LOCAL FUNCTION PROTOTYPE *******************************/

/******************** API FUNCTIONS ******************************************/
void vTEMP_task(void *pv_param_task)
{
    tskTEMP_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_TEMP;
    heartbeat.hb_counter = 0;

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskTEMP_TaskParam_t*) pv_param_task;

    /* Init temperature sensor */
    if(BSP_ENV_SENSOR_Init(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE) != BSP_ERROR_NONE)
    {
        vTskCommon_ErrorLoop();
    }

    /* Activate temperature sensor */
    if(BSP_ENV_SENSOR_Enable(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE) != BSP_ERROR_NONE)
    {
        vTskCommon_ErrorLoop();
    }

    float temp_raw_value = 0.0;
    float filtered_value = 10.0;
    static tskHMI_msg_fdbk_msg_t msg_temp_to_hmi = {0};
    tskHMI_msg_fdbk_pld_temperature_t* const temperature_payload = &(msg_temp_to_hmi.payload.temp_pld);
    msg_temp_to_hmi.header.fdbk_id = HMI_MSG_FDBK_ID_TEMP;

    /* TODO fix dummy wait (synchronise with task that shall create a queue set )*/
    vTaskDelay(1000 / portTICK_RATE_MS);

    while (1) /* Task loop */
    {
        /* Wait sufficient time prior to read another sensor value */
    	vTaskDelay(TEMP_SENSOR_ACQ_MS/portTICK_RATE_MS);

        /* Read temperature sensor */
        BSP_ENV_SENSOR_GetValue(ENV_SENSOR_STTS22H_0, ENV_TEMPERATURE, &temp_raw_value);

        /* Filter value */
        filtered_value = ((filtered_value * TEMP_SENSOR_RC_COEF)  +  temp_raw_value) /
                             (TEMP_SENSOR_RC_COEF + 1);

        /* TODO Send value to main */
        temperature_payload->temperature = filtered_value - TEMP_SENSOR_TEMP_OFFSET;
        xQueueSend(task_param->queue_temperature_sts, &msg_temp_to_hmi, 0); /* Don't wait on queue*/


        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > TEMP_HB_SEND_TIME_MS)
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

/**\} */
/**\} */

/* EOF */
