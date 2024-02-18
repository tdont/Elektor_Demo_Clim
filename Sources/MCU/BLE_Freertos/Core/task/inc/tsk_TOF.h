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
 * \file tsk_TOF.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 18, 2024
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
#ifndef TASK_INC_TSK_TOF_H_
#define TASK_INC_TSK_TOF_H_

/******************** INCLUDES ***********************************************/
#include <FreeRTOS.h>
#include <queue.h>
#include <stdint.h>

/******************** CONSTANTS OF MODULE ************************************/
#define TOF_HB_SEND_TIME_MS     200  /* Indicate when the task shall send a heartbeat */
#define TOF_SENSOR_ACQ_MS       100  /* Sampling period of the TOF sensor */


/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
 xQueueHandle queue_tof_distance;
 xQueueHandle queue_hb_to_watchdog;
}tskTOF_TaskParam_t;

typedef struct
{
    uint16_t distance_mm;
}tskTOF_queue_msg_t;


/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/
/**
******************************************************************************
* \par Description :
* Task loop for TEMPERATURE sensor
*
* \param[in] pvParameters   Pointer the the thread parameters
*
*****************************************************************************/
void vTOF_task(void *pvParameters);

#endif /* TASK_INC_TSK_TOF_H_ */

/**\} */
/**\} */

/* EOF */
