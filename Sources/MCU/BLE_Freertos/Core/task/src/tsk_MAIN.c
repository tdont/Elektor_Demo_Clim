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
 * \file tsk_MAIN.c
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
#include "tsk_MAIN.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_HMI.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct __attribute__((packed))
{
    bool                    pairing_in_progress;
    uint16_t                pairing_pin_code;
    uint8_t                 nb_device_connected;
}tskMAIN_ble_status_t;

typedef struct __attribute__((packed))
{
    float                   temperature_stpt;
    tskCommon_clim_mode_e   clim_mode;
}tskMAIN_clim_status_t;

typedef struct __attribute__((packed))
{
    tskCommon_ctrl_mode_e   ctrl_mode;
    tskMAIN_clim_status_t   clim_status;
    tskMAIN_ble_status_t    ble_status;
    float                   ambient_temperature;
}tskMAIN_system_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
tskMAIN_system_status_t main_system_status = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void MAIN_init(tskMAIN_TaskParam_t* task_param, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);

void MAIN_handle_incom_msgs(tskMAIN_TaskParam_t* task_param, 
                                xQueueSetHandle queue_set_hdl, 
                                uint16_t timeout_ms);

void MAIN_handle_incom_msgs_hmi_setpoint(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_msg_t* const msg_hmi_stpt);

void MAIN_handle_incom_msgs_hmi_setpoint_ctrlmode(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_cftrl_mode_t* const ctrlmode_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_clim_mode(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_clim_mode_t* const clim_mode_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_temperature(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_temperature_t* const temperature_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_pairing(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_pairing_t* const pairing);

void MAIN_send_hmi_feedback_clim_status(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_ble_status(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_ctrl_mode(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_temperature(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);

/******************** API FUNCTIONS ******************************************/
void vMAIN_task(void *pv_param_task)
{
    tskMAIN_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    /* Reserve area for messages to hmi */
    static tskHMI_msg_fdbk_msg_t hmi_feedback_msg = {0};

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_HMI;
    heartbeat.hb_counter = 0;

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskMAIN_TaskParam_t*) pv_param_task;

    /* Create Queue set */

    /* TODO Wait HMI init completion */
    vTaskDelay(1000 / portTICK_RATE_MS);

    /* Init status */
    MAIN_init(task_param, &hmi_feedback_msg);

    while (1) /* Task loop */
    {
        /* TODO handle incomming messages */
    	vTaskDelay(50/portTICK_RATE_MS);

        /* TODO handle incomming message and transition to the SM */

        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > MAIN_HB_SEND_TIME_MS)
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
void MAIN_init(tskMAIN_TaskParam_t* task_param, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    /* TODO Load from EEPROM */
    /* For now only set to default value */
    main_system_status.ble_status.pairing_in_progress = false;
    main_system_status.ble_status.pairing_pin_code = 0;
    main_system_status.ble_status.nb_device_connected = 0;

    main_system_status.ctrl_mode = TC_BLE_MODE_MANUAL;

    main_system_status.clim_status.clim_mode = TC_CLIM_MODE_OFF;
    main_system_status.clim_status.temperature_stpt = 20.0;

    /* Send all feedback to HMI */
    MAIN_send_hmi_feedback_clim_status(task_param->queue_hmi_feedback, hmi_msg_feedback);
    MAIN_send_hmi_feedback_ble_status(task_param->queue_hmi_feedback, hmi_msg_feedback);
    MAIN_send_hmi_feedback_ctrl_mode(task_param->queue_hmi_feedback, hmi_msg_feedback);
}

void MAIN_handle_incom_msgs(tskMAIN_TaskParam_t* task_param, 
                                        xQueueSetHandle queue_set_hdl, 
                                        uint16_t timeout_ms)
{

}

void MAIN_handle_incom_msgs_hmi_setpoint(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_msg_t* const msg_hmi_stpt)
{
    if(task_param == NULL)
    {
        return;
    }
    if(msg_hmi_stpt == NULL)
    {
        return;
    }

    
}

void MAIN_handle_incom_msgs_hmi_setpoint_ctrlmode(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_cftrl_mode_t* const ctrlmode_pld)
{
    if(task_param == NULL)
    {
        return;
    }
    if(ctrlmode_pld == NULL)
    {
        return;
    }
}

void MAIN_handle_incom_msgs_hmi_setpoint_clim_mode(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_clim_mode_t* const clim_mode_pld)
{
    if(task_param == NULL)
    {
        return;
    }
    if(clim_mode_pld == NULL)
    {
        return;
    }
}
void MAIN_handle_incom_msgs_hmi_setpoint_temperature(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_temperature_t* const temperature_pld)
{
    if(task_param == NULL)
    {
        return;
    }
    if(temperature_pld == NULL)
    {
        return;
    }
}

void MAIN_handle_incom_msgs_hmi_setpoint_pairing(tskHMI_TaskParam_t* task_param, 
                                const tskCommon_hmi_stpt_payload_pairing_t* const pairing)
{
    /* TODO handle pairing mode */
    return;
}

void MAIN_send_hmi_feedback_clim_status(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_main == NULL)
    {
        return;
    }
    if(hmi_msg_feedback == NULL)
    {
        return;
    }

    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_CLIM;
    hmi_msg_feedback->payload.clim_status.clim_mode = main_system_status.clim_status.clim_mode;
    hmi_msg_feedback->payload.clim_status.temperature_stpt = main_system_status.clim_status.temperature_stpt;

    xQueueSend(queue_to_main, hmi_msg_feedback, 1);
    return;
}

void MAIN_send_hmi_feedback_ble_status(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_main == NULL)
    {
        return;
    }
    if(hmi_msg_feedback == NULL)
    {
        return;
    }

    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_BLE;
    hmi_msg_feedback->payload.ble_status.nb_device_connected = main_system_status.ble_status.nb_device_connected;
    hmi_msg_feedback->payload.ble_status.pairing_in_progress = main_system_status.ble_status.pairing_in_progress;
    hmi_msg_feedback->payload.ble_status.pairing_pin_code = main_system_status.ble_status.pairing_pin_code;

    xQueueSend(queue_to_main, hmi_msg_feedback, 1);
}

void MAIN_send_hmi_feedback_ctrl_mode(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_CTRL_MODE;
    hmi_msg_feedback->payload.control_mode.ctrl_mode = main_system_status.ctrl_mode;

    xQueueSend(queue_to_main, hmi_msg_feedback, 1);
}

void MAIN_send_hmi_feedback_temperature(xQueueHandle queue_to_main, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_TEMP;
    hmi_msg_feedback->payload.temperature.temperature = main_system_status.ambient_temperature;

    xQueueSend(queue_to_main, hmi_msg_feedback, 1);
}
/**\} */
/**\} */

/* EOF */
