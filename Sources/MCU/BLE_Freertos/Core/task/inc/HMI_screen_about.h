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
 * \file HMI_screen_about.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Mar 3, 2024
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
#ifndef TASK_INC_HMI_SCREEN_ABOUT_H_
#define TASK_INC_HMI_SCREEN_ABOUT_H_

/******************** INCLUDES ***********************************************/
#include <stdint.h>

#include "HMI_screen.h"

/******************** CONSTANTS OF MODULE ************************************/


/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
    char        letter;
    uint8_t     major;
    uint8_t     minor;
    uint8_t     release;
    //char*       author;
}HMI_screen_about_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
extern tsk_HMI_screen_metadata_t hmi_about_metadata;


/******************** API FUNCTION PROTOTYPE *********************************/
void vHMIA_init(const void* const screen_cm_data, YACSWL_widget_t* const root_widget);

void vHMIA_enter_screen(void);
void vHMIA_leave_screen(void);

void vHMIA_update(const void* const screen_cm_data, tskHMI_range_t* range);

#endif /* TASK_INC_HMI_SCREEN_ABOUT_H_ */

/**\} */
/**\} */

/* EOF */
