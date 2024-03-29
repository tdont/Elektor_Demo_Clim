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
 * \file tsk_HMI_screen_main.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 11, 2024
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
#ifndef TASK_INC_HMI_SCREEN_MAIN_H_
#define TASK_INC_HMI_SCREEN_MAIN_H_

/******************** INCLUDES ***********************************************/
#include <YACSWL.h>
#include "HMI_screen.h"

/******************** CONSTANTS OF MODULE ************************************/


/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
    float   ambient_temperature;
}HMI_screen_main_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
extern tsk_HMI_screen_metadata_t hmi_main_metadata;

void vHMISM_init(const void* const screen_main_data, YACSWL_widget_t* const root_widget);

void vHMISM_enter_screen(void);
void vHMISM_leave_screen(void);

void vHMISM_update(const void* const screen_main_data, tskHMI_range_t* range);


/******************** API FUNCTION PROTOTYPE *********************************/

#endif /* TASK_INC_HMI_SCREEN_MAIN_H_ */

/**\} */
/**\} */

/* EOF */
