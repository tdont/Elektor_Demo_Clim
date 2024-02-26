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
 * \file tsk_IR_ATL.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 26, 2024
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
#ifndef TASK_INC_TSK_IR_ATL_H_
#define TASK_INC_TSK_IR_ATL_H_

/******************** INCLUDES ***********************************************/
#include <FreeRTOS.h>
#include <queue.h>
#include <stdint.h>

/******************** CONSTANTS OF MODULE ************************************/
#define IRATL_HB_SEND_TIME_MS           200  /* Indicate when the task shall send a heartbeat */

#define IRATL_DELAY_PRIOR_TO_SEND_MS    1000 /* Ensure that enough time has elapsed since last setpoint reception */

/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
 xQueueHandle       queue_to_ir_atl;
 xQueueHandle       queue_hb_to_watchdog;
}tskIRATL_TaskParam_t;



/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/
/**
******************************************************************************
* \par Description :
* Task loop for IR atlantic sender
*
* \param[in] pvParameters   Pointer the the thread parameters
*
*****************************************************************************/
void vIRATL_task(void *pvParameters);

#endif /* TASK_INC_TSK_IR_ATL_H_ */

/**\} */
/**\} */

/* EOF */
