/******************************************************************************
* Copyright(c) 2024 Thibaut DONTAIL
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
* along with ELEKTO_DEMO_CLIM.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/******************** FILE HEADER ********************************************/
/**\addtogroup ModuleName_External_Documentation
 * \{ */
/**\addtogroup SubModuleName
 * \{ */

/**
 * \file tsk_WDGT.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date 21 jan. 2024
 */

/**
* \cond INTERNAL_DOC
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

/******************** INCLUDES ***********************************************/
#include "tsk_common.h"

#include <FreeRTOS.h>
#include <task.h>

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** LOCAL FUNCTION PROTOTYPE *******************************/

/******************** API FUNCTIONS ******************************************/
void vTskCommon_ErrorLoop(void)
{
    while (1)
    {
        vTaskDelay(configTICK_RATE_HZ);
    }
}

/******************** LOCAL FUNCTIONS ****************************************/

/**\} */
/**\} */

/* EOF */
