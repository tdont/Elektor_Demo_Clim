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
#include "tsk_TEMP.h"
#include "tsk_IR_ATL.h"
#include "version.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    bool                    pairing_in_progress;
    uint16_t                pairing_pin_code;
    uint8_t                 nb_device_connected;
}tskMAIN_ble_status_t;

typedef struct 
{
    float                   temperature_stpt;
    tskCommon_clim_mode_e   clim_mode;
}tskMAIN_clim_status_t;

typedef struct 
{
    tskCommon_ctrl_mode_e   ctrl_mode;
    tskMAIN_clim_status_t   clim_status;
    tskMAIN_ble_status_t    ble_status;
    float                   ambient_temperature;
}tskMAIN_system_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static tskMAIN_system_status_t main_system_status = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void MAIN_init(tskMAIN_TaskParam_t* task_param, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);

void MAIN_handle_incom_msgs(tskMAIN_TaskParam_t* task_param, 
                                xQueueSetHandle queue_set_hdl, 
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                uint16_t timeout_ms);

                                
void MAIN_handle_incom_msgs_temperature(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);

void MAIN_handle_incom_msgs_hmi_setpoint(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);

void MAIN_handle_incom_msgs_hmi_setpoint_ctrlmode(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                const tskCommon_hmi_stpt_payload_cftrl_mode_t* const ctrlmode_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_clim_mode(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                const tskCommon_hmi_stpt_payload_clim_mode_t* const clim_mode_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_temperature(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                const tskCommon_hmi_stpt_payload_temperature_t* const temperature_pld);

void MAIN_handle_incom_msgs_hmi_setpoint_pairing(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                const tskCommon_hmi_stpt_payload_pairing_t* const pairing);

void MAIN_send_hmi_feedback_clim_status(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_ble_status(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_ctrl_mode(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_temperature(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);
void MAIN_send_hmi_feedback_about(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback);


void MAIN_send_IR_setpoint(xQueueHandle queue_to_IR);

/******************** API FUNCTIONS ******************************************/
void vMAIN_task(void *pv_param_task)
{
    tskMAIN_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;
    xQueueSetHandle queue_set_hdl = 0;
    portBASE_TYPE ret = pdFAIL;

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

    /* Create queue set to listen from multiple queue */
    queue_set_hdl = xQueueCreateSet(TSK_CNFG_QUEUE_SET_MAIN_SIZE);
    /* Check for an error */
    if (queue_set_hdl == 0)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    /* Add queue to queue set */
    ret = xQueueAddToSet(task_param->queue_hmi_setpoint, queue_set_hdl);
    if (ret != pdPASS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    ret = xQueueAddToSet(task_param->queue_temperature_sensor, queue_set_hdl);
    if (ret != pdPASS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    /* TODO Wait HMI init completion */
    vTaskDelay(1000 / portTICK_RATE_MS);

    /* Init status */
    MAIN_init(task_param, &hmi_feedback_msg);

    /* Send version to HMI */
    MAIN_send_hmi_feedback_about(task_param->queue_hmi_feedback, &hmi_feedback_msg);

    while (1) /* Task loop */
    {
        /* Compute remaining time before feeding the watchdog */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* If there is enough time */
        if (elapsed_time_ms <= MAIN_HB_SEND_TIME_MS)
        {
            /* Handle incomming messages */
            if ((MAIN_HB_SEND_TIME_MS - elapsed_time_ms) > MAIN_TASK_LOOP_TIME_MS)
            {
                /* Delay before feeding the watchdog is greater than the desired task loop duration */
                MAIN_handle_incom_msgs(task_param, queue_set_hdl, &hmi_feedback_msg, MAIN_TASK_LOOP_TIME_MS);
            }
            else
            {
                /* Delay before feeding the watchdog is lower than the desired task loop duration, only wait message for the remaining amount of time */
                MAIN_handle_incom_msgs(task_param, queue_set_hdl, &hmi_feedback_msg,
                                                (MAIN_HB_SEND_TIME_MS - elapsed_time_ms));
            }
        }

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
                                        tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                        uint16_t timeout_ms)
{
    xQueueSetMemberHandle queue_hdl_data_available;
    /* Check parameters */
    if (task_param == NULL)
    {
        /* Error null pointer */
        return;
    }
    if (queue_set_hdl == NULL)
    {
        /* Error null pointer */
        return;
    }

    /* Look for available message on Queues */
    queue_hdl_data_available = xQueueSelectFromSet(queue_set_hdl, timeout_ms / portTICK_RATE_MS);

    /* HMI Setpoint messages */
    if(queue_hdl_data_available == task_param->queue_hmi_setpoint)
    {
        MAIN_handle_incom_msgs_hmi_setpoint(task_param, hmi_msg_feedback);
    }
    else if (queue_hdl_data_available == task_param->queue_temperature_sensor)
    {
        MAIN_handle_incom_msgs_temperature(task_param, hmi_msg_feedback);
    }
    else
    {
        /* If queue is not null (timeout), then this message is unexpected */
        if(queue_hdl_data_available != NULL)
        {
            /* Unsupported message */
            vTskCommon_ErrorLoop();
        }
    }

    return;
}


void MAIN_handle_incom_msgs_temperature(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(task_param == NULL)
    {
        return;
    }

    portBASE_TYPE ret = 0;
    static tskTEMP_queue_msg_t msg_temperature = {0};

    /* Retrieve message from queue */
    ret = xQueueReceive(task_param->queue_temperature_sensor, (void*) &msg_temperature, 0);

    /* If message was well received */
    if(ret == pdPASS)
    {   
        /* Save ambient temperature */
        main_system_status.ambient_temperature = msg_temperature.temperature;

        /* Send feedback to HMI */
        MAIN_send_hmi_feedback_temperature(task_param->queue_hmi_feedback, hmi_msg_feedback);

        /* TODO create dedicated function  : Forward temperature to BLE */
        static tskMAIN_BLE_feedback_msg_t ble_feedback_msg = {0};
        ble_feedback_msg.ambient_temperature = main_system_status.ambient_temperature;
        xQueueSend(task_param->queue_ble_feedback, &ble_feedback_msg, 1);
    }

    return;
}

void MAIN_handle_incom_msgs_hmi_setpoint(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(task_param == NULL)
    {
        return;
    }

    portBASE_TYPE ret = 0;
    static tskCommon_hmi_stpt_msg_t msg_hmi_stpt = {0};

    /* Retrieve message from queue */
    ret = xQueueReceive(task_param->queue_hmi_setpoint, (void*) &msg_hmi_stpt, 0);

    /* If message was well received */
    if(ret == pdPASS)
    {
        /* Dispatch message */
        switch (msg_hmi_stpt.header.msg_type)
        {
            case TC_HMI_STPT_TYPE_CTRL_MODE:
                MAIN_handle_incom_msgs_hmi_setpoint_ctrlmode(task_param, 
                                                    hmi_msg_feedback,
                                                    &(msg_hmi_stpt.payload.ctrl_mode));
                break;
            case TC_HMI_STPT_TYPE_CLIM_MODE:
                MAIN_handle_incom_msgs_hmi_setpoint_clim_mode(task_param, 
                                                    hmi_msg_feedback,
                                                    &(msg_hmi_stpt.payload.clim_mode));
                break;
            case TC_HMI_STPT_TYPE_TEMPERATURE:
                MAIN_handle_incom_msgs_hmi_setpoint_temperature(task_param, 
                                                    hmi_msg_feedback,
                                                    &(msg_hmi_stpt.payload.temperature));
                break;
            case TC_HMI_STPT_TYPE_BLE_PAIRING:
                MAIN_handle_incom_msgs_hmi_setpoint_pairing(task_param, 
                                                    hmi_msg_feedback,
                                                    &(msg_hmi_stpt.payload.pairing));
                break;
            default:
                /* Drop messages */
                break;
        }
    }
}

void MAIN_handle_incom_msgs_hmi_setpoint_ctrlmode(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
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

    /* Save control mode */
    main_system_status.ctrl_mode = ctrlmode_pld->val;

    /* Provide feedback to HMI */
    MAIN_send_hmi_feedback_ctrl_mode(task_param->queue_hmi_feedback, hmi_msg_feedback);

    /* Send to IR new setpoint */
    MAIN_send_IR_setpoint(task_param->queue_to_ir);
}

void MAIN_handle_incom_msgs_hmi_setpoint_clim_mode(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
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

    /* Reject setpoint if not in manual mode */
    if(main_system_status.ctrl_mode != TC_BLE_MODE_MANUAL)
    {
        return;
    }

    /* Save Clim mode */
    main_system_status.clim_status.clim_mode = clim_mode_pld->val;

    /* Provide feedback to HMI */
    MAIN_send_hmi_feedback_clim_status(task_param->queue_hmi_feedback, hmi_msg_feedback);

    /* Send to IR new setpoint */
    MAIN_send_IR_setpoint(task_param->queue_to_ir);
}
void MAIN_handle_incom_msgs_hmi_setpoint_temperature(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
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
    
    /* Reject setpoint if not in manual mode */
    if(main_system_status.ctrl_mode != TC_BLE_MODE_MANUAL)
    {
        return;
    }

    /* Save temperature setpoint */
    main_system_status.clim_status.temperature_stpt = temperature_pld->val;

    /* Provide feedback to HMI */
    MAIN_send_hmi_feedback_clim_status(task_param->queue_hmi_feedback, hmi_msg_feedback);

    /* Send to IR new setpoint */
    MAIN_send_IR_setpoint(task_param->queue_to_ir);
}

void MAIN_handle_incom_msgs_hmi_setpoint_pairing(tskMAIN_TaskParam_t* task_param,
                                tskHMI_msg_fdbk_msg_t* hmi_msg_feedback,
                                const tskCommon_hmi_stpt_payload_pairing_t* const pairing)
{
    /* TODO handle pairing mode */

    MAIN_send_hmi_feedback_ble_status(task_param->queue_hmi_feedback, hmi_msg_feedback);
    return;
}

void MAIN_send_hmi_feedback_clim_status(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_hmi == NULL)
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

    xQueueSend(queue_to_hmi, hmi_msg_feedback, 1);
    return;
}

void MAIN_send_hmi_feedback_ble_status(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_hmi == NULL)
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

    xQueueSend(queue_to_hmi, hmi_msg_feedback, 1);
}

void MAIN_send_hmi_feedback_ctrl_mode(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_hmi == NULL)
    {
        return;
    }
    if(hmi_msg_feedback == NULL)
    {
        return;
    }

    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_CTRL_MODE;
    hmi_msg_feedback->payload.control_mode.ctrl_mode = main_system_status.ctrl_mode;

    xQueueSend(queue_to_hmi, hmi_msg_feedback, 1);
}

void MAIN_send_hmi_feedback_temperature(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{    
    if(queue_to_hmi == NULL)
    {
        return;
    }
    if(hmi_msg_feedback == NULL)
    {
        return;
    }

    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_TEMP;
    hmi_msg_feedback->payload.temperature.temperature = main_system_status.ambient_temperature;

    xQueueSend(queue_to_hmi, hmi_msg_feedback, 1);
}

void MAIN_send_hmi_feedback_about(xQueueHandle queue_to_hmi, tskHMI_msg_fdbk_msg_t* hmi_msg_feedback)
{
    if(queue_to_hmi == NULL)
    {
        return;
    }
    if(hmi_msg_feedback == NULL)
    {
        return;
    }

    hmi_msg_feedback->header.fdbk_id = HMI_MSG_FDBK_ID_ABOUT;
    hmi_msg_feedback->payload.about.letter = version[VERSION_OFFSET_LETTER];
    hmi_msg_feedback->payload.about.major = version[VERSION_OFFSET_MAJOR];
    hmi_msg_feedback->payload.about.minor = version[VERSION_OFFSET_MINOR];
    hmi_msg_feedback->payload.about.release = version[VERSION_OFFSET_RELEASE];


    xQueueSend(queue_to_hmi, hmi_msg_feedback, 1);
}

void MAIN_send_IR_setpoint(xQueueHandle queue_to_IR)
{
    if(queue_to_IR == NULL)
    {
        return;
    }

    static tskMAIN_clim_stpt_to_IR_msg_t msg_to_ir = {0};

    /* Build message to IR */
    msg_to_ir.temperature_stpt = main_system_status.clim_status.temperature_stpt;
    msg_to_ir.clim_mode = main_system_status.clim_status.clim_mode;

    /* Send message to IR transmitter task */
    xQueueSend(queue_to_IR, &msg_to_ir, 1);
}
/**\} */
/**\} */

/* EOF */
