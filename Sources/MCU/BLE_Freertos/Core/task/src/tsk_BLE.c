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
 * \file tsk_BLE.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Mar 1, 2024
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
#include "tsk_BLE.h"

#include <common_blesvc.h>

#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tsk_MAIN.h"
#include "tsk_common.h"
#include "tsk_config.h"



/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct
{
  uint16_t	EnvSensingSvcHdle;				/**< Service handle */
  uint16_t	TemperatureCharHdle;	/**< Characteristic handle */
}BLE_env_Context_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void BLE_handle_incomming_message(xQueueHandle queue, 
                            const BLE_env_Context_t* const ble_context,
                            uint16_t timeout_ms);
/******************** API FUNCTIONS ******************************************/
void vBLE_task(void *pv_param_task)
{
    tskBLE_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_BLE;
    heartbeat.hb_counter = 0;

    static BLE_env_Context_t  ble_env_context = {0};

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskBLE_TaskParam_t*) pv_param_task;

    /* Dummy wait to let the BLE stack initialize */
    /* TODO fix this with a semaphore */
    vTaskDelay(10000 / portTICK_RATE_MS);

    /* Add service */
    uint16_t uuid = ENVIRONMENTAL_SENSING_SERVICE_UUID;
    tBleStatus hciCmdResult = BLE_STATUS_SUCCESS;

    hciCmdResult = aci_gatt_add_service(UUID_TYPE_16,
                                   (Service_UUID_t *) &uuid,
                                   PRIMARY_SERVICE,
                                   4,
                                   &(ble_env_context.EnvSensingSvcHdle));

    if (hciCmdResult != BLE_STATUS_SUCCESS)
    {
        vTskCommon_ErrorLoop();
    }

    /* Add characteristic */
    /**
     *  Add temperature Characteristic
     */
    uuid = TEMPERATURE_UUID;
    hciCmdResult = aci_gatt_add_char(ble_env_context.EnvSensingSvcHdle,
                                    UUID_TYPE_16,
                                    (Char_UUID_t *) &uuid ,
                                    1,/** Measure */
                                    CHAR_PROP_NOTIFY,
                                    ATTR_PERMISSION_NONE,
                                    GATT_DONT_NOTIFY_EVENTS, /* gattEvtMask */
                                    10, /* encryKeySize */
                                    1, /* isVariable */
                                    &(ble_env_context.TemperatureCharHdle));

    if (hciCmdResult != BLE_STATUS_SUCCESS)
    {
        vTskCommon_ErrorLoop();
    }

    while (1) /* Task loop */
    {
        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Handle incomming messages from main */
        if(elapsed_time_ms <= BLE_HB_SEND_TIME_MS)
        {
            BLE_handle_incomming_message(task_param->queue_feedback_from_main, 
                            &ble_env_context,
                            BLE_HB_SEND_TIME_MS - elapsed_time_ms);

        }

        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > BLE_HB_SEND_TIME_MS)
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
void BLE_handle_incomming_message(xQueueHandle queue, 
                            const BLE_env_Context_t* const ble_env_context,
                            uint16_t timeout_ms)
{
    int8_t ambient_temperature = 0;

    static tskMAIN_BLE_feedback_msg_t feedback_msg = {0};
    portBASE_TYPE ret = 0;

    /* Retrieve message from queue */
    ret = xQueueReceive(queue, 
                        (void*) &feedback_msg,
                        (timeout_ms / portTICK_RATE_MS));

    if(ret == pdPASS)
    {
        /* Extract temperature */
        ambient_temperature = (int8_t) feedback_msg.ambient_temperature;

        /* Update temperature characteristics */
        aci_gatt_update_char_value(ble_env_context->EnvSensingSvcHdle,
                                    ble_env_context->TemperatureCharHdle,
                                    0,
                                    1,
                                    (uint8_t*)&ambient_temperature);
    }

    return;
}
/**\} */
/**\} */

/* EOF */
