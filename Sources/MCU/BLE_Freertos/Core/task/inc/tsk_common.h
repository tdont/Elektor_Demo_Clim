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
/* Prevent multiple inclusions */
#ifndef INC_TSK_COMMON_H_
#define INC_TSK_COMMON_H_

/******************** INCLUDES ***********************************************/
#include <stdint.h>
#include <FreeRTOSConfig.h>

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
    uint8_t tsk_id;          /* Identification of task */
    uint8_t hb_counter;      /* HeartBeatCounter, Shall keep incrementing */
}tskCommon_Hb_t;

typedef enum
{
    TC_CLIM_MODE_HEAT = 0,
    TC_CLIM_MODE_COLD
}tskCommon_clim_mode_e;

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** API FUNCTION PROTOTYPE *********************************/
void vTskCommon_ErrorLoop(void);


#endif /* INC_TSK_COMMON_H_ */

/**\} */
/**\} */

/* EOF */
