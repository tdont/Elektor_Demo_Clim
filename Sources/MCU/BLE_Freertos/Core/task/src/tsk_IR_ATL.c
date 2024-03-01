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
#include "ir_common.h"
#include "ir_atl_encode.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    tskMAIN_clim_stpt_to_IR_msg_t setpoint_msg;
    portTickType last_rcvd_msg_tick;
}tskIRATL_last_setpoint_received_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static uint8_t common_byte[IRATL_IR_FRAME_FIRST_BYTE_SIZE] = {0x14, 0x63, 0x00, 0x10, 0x10};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void IRATL_handle_incomming_message(xQueueHandle queue,
                            tskIRATL_last_setpoint_received_t* const last_setpoint_received, 
                            uint16_t timeout_ms);

void IRATL_evaluate_sending_over_IR(tskIRATL_last_setpoint_received_t* const setpoint);

void IRATL_build_and_send_off_message(IRATL_IR_frame_helper_t* const ir_frame_helper, tskIRATL_last_setpoint_received_t* const setpoint);
void IRATL_build_and_send_on_message(IRATL_IR_frame_helper_t* const ir_frame_helper, tskIRATL_last_setpoint_received_t* const setpoint);
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
    SIRC_Encode_Init();

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
        IRATL_evaluate_sending_over_IR(&last_setpoint_received);

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

void IRATL_evaluate_sending_over_IR(tskIRATL_last_setpoint_received_t* const setpoint)
{
    /* Check parameters */
    if(setpoint == NULL)
    {
        return;
    }

    if(setpoint->last_rcvd_msg_tick == 0)
    {
        /* No new setpoint, nothing to send */
        return;
    }

    if(((xTaskGetTickCount() - setpoint->last_rcvd_msg_tick) * portTICK_RATE_MS) > IRATL_DELAY_PRIOR_TO_SEND_MS)
    {
        static IRATL_IR_frame_helper_t ir_frame_helper = {0};
        static IRATL_IR_frame_t* const ir_frame = &(ir_frame_helper.frame);

        /* Reset buffer */
        memset(ir_frame_helper.raw_message, 0, sizeof(IRATL_IR_frame_helper_t));
        
        /* Build frame, apply common accross all messages header part of IR frame */
        memcpy(&(ir_frame->header.common), common_byte, IRATL_IR_FRAME_FIRST_BYTE_SIZE);

        /* Select message builder based on clim mode */
        switch(setpoint->setpoint_msg.clim_mode)
        {
            case TC_CLIM_MODE_OFF:
                IRATL_build_and_send_off_message(&ir_frame_helper, setpoint);
                break;
            case TC_CLIM_MODE_HEAT: /* FALLTHROUGH */
            case TC_CLIM_MODE_COLD:
                IRATL_build_and_send_on_message(&ir_frame_helper, setpoint);
                break;
            default:
                /* Drop setpoint */
                break;
        }

        /* Indicate the setpoint was send over IR*/
        setpoint->last_rcvd_msg_tick = 0;
    }

    return;
}

void IRATL_build_and_send_off_message(IRATL_IR_frame_helper_t* const ir_frame_helper, 
                                        tskIRATL_last_setpoint_received_t* const setpoint)
{
    /* Check parameters */
    if(setpoint == NULL)
    {
        return;
    }

    IRATL_IR_frame_t* const ir_frame = &(ir_frame_helper->frame);

    /* Indicate it is an OFF frame */
    ir_frame->header.on_off_toggler = IRATL_IR_FRAME_ONOFF_TOGGLER_OFF;

    /* Set the OFF cheksum */
    ir_frame->payload.pld_off.checksum = IRATL_IR_FRAME_OFF_PLD_CHECKSUM;

    /* Send Frame */
    IRATL_transmit_frame(ir_frame_helper->raw_message, sizeof (IRATL_IR_header_t) + sizeof(IRATL_IR_pld_OFF_t));

    return;
}

void IRATL_build_and_send_on_message(IRATL_IR_frame_helper_t* const ir_frame_helper, 
                                        tskIRATL_last_setpoint_received_t* const setpoint)
{
    /* Check parameters */
    if(setpoint == NULL)
    {
        return;
    }

    IRATL_IR_frame_t* const ir_frame = &(ir_frame_helper->frame);

    /* Indicate it is an ON frame */
    ir_frame->header.on_off_toggler = IRATL_IR_FRAME_ONOFF_TOGGLER_ON;

    /* Set standard byte at the default value */
    ir_frame->payload.pld_on.b1 = IRATL_IR_FRAME_ON_PLD_B1;
    ir_frame->payload.pld_on.b2 = IRATL_IR_FRAME_ON_PLD_B2;
    ir_frame->payload.pld_on.b6 = IRATL_IR_FRAME_ON_PLD_B6_B7_B8;
    ir_frame->payload.pld_on.b7 = IRATL_IR_FRAME_ON_PLD_B6_B7_B8;
    ir_frame->payload.pld_on.b8 = IRATL_IR_FRAME_ON_PLD_B6_B7_B8;
    
    /* Set swing mode to vertical with low fan (This point is never a concern in the office) */
    ir_frame->payload.pld_on.swing_mode = IRATL_IR_FRAME_ON_PLD_SWNGMODE_FAN_LOW | IRATL_IR_FRAME_ON_PLD_SWNGMODE_SWNG_VER;

    /* Set heat mode */
    switch(setpoint->setpoint_msg.clim_mode)
    {
        case TC_CLIM_MODE_HEAT:
            ir_frame->payload.pld_on.heat_mode = IRATL_IR_FRAME_ON_PLD_HEATMODE_HEAT;
            break;
        case TC_CLIM_MODE_COLD:
            ir_frame->payload.pld_on.heat_mode = IRATL_IR_FRAME_ON_PLD_HEATMODE_COOL;
            break;
        case TC_CLIM_MODE_OFF:
        default:
            /* Unhandled case, fallback to auto mode */
            ir_frame->payload.pld_on.heat_mode = IRATL_IR_FRAME_ON_PLD_HEATMODE_AUTO;
            break;
    }

    /* Set temperature setpoint */
        uint8_t temperature_u = (uint8_t) setpoint->setpoint_msg.temperature_stpt;
        /* Check range */
        if(temperature_u < IRATL_IR_FRAME_TEMP_MINIMAL_VALUE_DEG_C)
        {
            temperature_u = IRATL_IR_FRAME_TEMP_MINIMAL_VALUE_DEG_C;
        }
        if(temperature_u > IRATL_IR_FRAME_TEMP_MAXIMAL_VALUE_DEG_C)
        {
            temperature_u = IRATL_IR_FRAME_TEMP_MAXIMAL_VALUE_DEG_C;
        }

        /* Remove offset */
        temperature_u = temperature_u - IRATL_IR_FRAME_TEMP_OFFSET_VALUE;

        /* Store value */
        ir_frame->payload.pld_on.temperature = (temperature_u << 4);

    /* Set eco mode (never in eco mode so far */
    ir_frame->payload.pld_on.eco_mode = IRATL_IR_FRAME_ON_PLD_ECOMODE_NOM;

    /* TODO use RTC information to switch into eco mode during the night, or perform a shutdown */

    /* Compute checksum */
    uint8_t msb_sum = ((ir_frame->payload.pld_on.b1 & 0b11110000) >> 4) 
                        + ((ir_frame->payload.pld_on.b2 & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.temperature & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.heat_mode & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.swing_mode & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.b6 & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.b7 & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.b8 & 0b11110000) >> 4)
                        + ((ir_frame->payload.pld_on.eco_mode & 0b11110000) >> 4);

    uint8_t msb_checksum = IRATL_CHECKSUM_MSB_START_VALUE - (msb_sum % 16);
    ir_frame->payload.pld_on.checksum = msb_checksum << 4;

    int8_t lsb_sum = IRATL_CHECKSUM_LSB_START_VALUE                         /* Do not consider b1 for this checksum */
                        - (ir_frame->payload.pld_on.temperature & 0b1111)
                        - (ir_frame->payload.pld_on.heat_mode & 0b1111)
                        - (ir_frame->payload.pld_on.swing_mode & 0b1111)
                        - (ir_frame->payload.pld_on.b2 & 0b1111) 
                        - (ir_frame->payload.pld_on.b6 & 0b1111)
                        - (ir_frame->payload.pld_on.b7 & 0b1111)
                        - (ir_frame->payload.pld_on.b8 & 0b1111)
                        - (ir_frame->payload.pld_on.eco_mode & 0b1111);
    
    while( lsb_sum < 0)
    {
        lsb_sum += 15;
    }
    ir_frame->payload.pld_on.checksum |= (lsb_sum & 0b1111);

    /* Send Frame */
    IRATL_transmit_frame(ir_frame_helper->raw_message, sizeof (IRATL_IR_frame_t));
    
    return;
}

/**\} */
/**\} */

/* EOF */
