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
 * \file HMI_screen_temperature_setpoint.h
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
#ifndef TASK_INC_HMI_SCREEN_TEMPERATURE_SETPOINT_H_
#define TASK_INC_HMI_SCREEN_TEMPERATURE_SETPOINT_H_

/******************** INCLUDES ***********************************************/
#include <YACSWL.h>

#include "tsk_common.h"
#include "HMI_screen.h"

/******************** CONSTANTS OF MODULE ************************************/


/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    float temperature_stpt;
}HMI_screen_temp_stpt_sts_t;

typedef struct
{
    float new_temperature_stpt;
}HMI_screen_temperature_setpoint_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
extern tsk_HMI_screen_metadata_t hmi_temperature_stpt_metadata;

/******************** API FUNCTION PROTOTYPE *********************************/
void vHMITS_init(const void* const screen_cm_data, YACSWL_widget_t* const root_widget);

void vHMITS_enter_screen(void);
void vHMITS_leave_screen(void);
void vHMITS_enter_edit(void);
void vHMITS_validate_edit(tskCommon_hmi_stpt_msg_t* const msg_setpt,
                               xQueueHandle queue_hmi_stpt);
void vHMITS_cancel_edit(void);

void vHMITS_update(const void* const screen_cm_data, tskHMI_range_t* range);

#endif /* TASK_INC_HMI_SCREEN_TEMPERATURE_SETPOINT_H_ */

/**\} */
/**\} */

/* EOF */
