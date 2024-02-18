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
/**\addtogroup ModuleName_External_Documentation
 * \{ */
/**\addtogroup SubModuleName
 * \{ */

/**
 * \file tsk_HMI.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 7, 2024
 */

 /**
 * \cond EXTERNAL_DOC
 *
 * \mainpage External documentation of module
 *
 * \section Description
 * This module does what you want him to do....
 *
 * \section Features
 * The main features of the module are described here.
 *
 * \endcond
 */


/* Prevent multiple inclusions */
#ifndef TASK_INC_TSK_HMI_H_
#define TASK_INC_TSK_HMI_H_

/******************** INCLUDES ***********************************************/
#include <FreeRTOS.h>
#include <queue.h>
#include <stdint.h>

/******************** CONSTANTS OF MODULE ************************************/
#define HMI_HB_SEND_TIME_MS     200  /* Indicate when the task shall send a heartbeat */
#define HMI_TASK_LOOP_TIME_MS   20  

#define HMI_DURATION_ROLLBACK_MAIN_SCREEN_MS    10000   /* 10 second to go back to main */

typedef enum
{
    HMI_MSG_FDBK_ID_TEMP = 0
}tskHMI_msg_fdbk_id_e;



/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
 xQueueHandle queue_hmi_feedback;
 xQueueHandle queue_hmi_btn;
 xQueueHandle queue_hmi_setpoint;
 xQueueHandle queue_hb_to_watchdog;
}tskHMI_TaskParam_t;


typedef struct  __attribute__((packed))
{
    tskHMI_msg_fdbk_id_e fdbk_id;
}tskHMI_msg_fdbk_header_t;

typedef struct __attribute__((packed))
{
    float temperature;
}tskHMI_msg_fdbk_pld_temperature_t;

typedef union __attribute__((packed))
{
    uint8_t                             rawData[1];
    tskHMI_msg_fdbk_pld_temperature_t   temp_pld;
}tskHMI_msg_fdbk_payload_t;

typedef struct  __attribute__((packed))
{
    tskHMI_msg_fdbk_header_t header;
    tskHMI_msg_fdbk_payload_t payload;
}tskHMI_msg_fdbk_msg_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/
/**
******************************************************************************
* \par Description :
* Task loop for HMI handling
*
* \param[in] pvParameters   Pointer the the thread parameters
*
*****************************************************************************/
void vHMI_task(void *pvParameters);

#endif /* TASK_INC_TSK_HMI_H_ */

/**\} */
/**\} */

/* EOF */
