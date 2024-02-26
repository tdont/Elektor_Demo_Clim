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
 * \file tsk_IR_ATL.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 26, 2024
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
#include "tsk_IR_ATL.h"

#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stm32wb5mm_dk.h>

#include "tsk_MAIN.h"
#include "tsk_common.h"
#include "tsk_config.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    tskMAIN_clim_stpt_to_IR_msg_t setpoint_msg;
    portTickType last_rcvd_msg_tick;
}tskIRATL_last_setpoint_received_t;
/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void IRATL_handle_incomming_message(xQueueHandle queue,
                            tskIRATL_last_setpoint_received_t* const last_setpoint_received, 
                            uint16_t timeout_ms);
/******************** API FUNCTIONS ******************************************/
void vIRATL_task(void *pv_param_task)
{
    tskIRATL_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_IRATL;
    heartbeat.hb_counter = 0;

    static tskIRATL_last_setpoint_received_t last_setpoint_received = {0};

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskIRATL_TaskParam_t*) pv_param_task;

    /* Init IR timer */


    while (1) /* Task loop */
    {
        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* If there is enought time before feeding watchdog */
        if (elapsed_time_ms <= IRATL_HB_SEND_TIME_MS)
        {
            /* Delay before feeding the watchdog is lower than the desired task loop duration, only wait message for the remaining amount of time */
            IRATL_handle_incomming_message(task_param->queue_to_ir_atl,
                                            &last_setpoint_received,
                                            (IRATL_HB_SEND_TIME_MS - elapsed_time_ms));
        }

        /* Evaluate sending of message to IR (avoid sending too many IR signals) */

        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > IRATL_HB_SEND_TIME_MS)
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
void IRATL_handle_incomming_message(xQueueHandle queue, 
                            tskIRATL_last_setpoint_received_t* const last_setpoint_received, 
                            uint16_t timeout_ms)
{
    static tskMAIN_clim_stpt_to_IR_msg_t setpoint_msg = {0};
    portBASE_TYPE ret = 0;

    /* Retrieve message from queue */
    ret = xQueueReceive(queue, 
                        (void*) &setpoint_msg,
                        (timeout_ms / portTICK_RATE_MS));

    if(ret == pdPASS)
    {
        /* Save the message */
        memcpy(&(last_setpoint_received->setpoint_msg), &setpoint_msg, sizeof(tskMAIN_clim_stpt_to_IR_msg_t));

        /* Save the time when the last message was received */
        last_setpoint_received->last_rcvd_msg_tick = xTaskGetTickCount();
    }
}

/**\} */
/**\} */

/* EOF */
