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
 * \file version.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Mar 11, 2024
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
#ifndef TASK_INC_VERSION_H_
#define TASK_INC_VERSION_H_

/******************** INCLUDES ***********************************************/
#include "tsk_config.h"



/******************** CONSTANTS OF MODULE ************************************/
#define VERSION_OFFSET_LETTER   0
#define VERSION_OFFSET_MAJOR    1
#define VERSION_OFFSET_MINOR    2
#define VERSION_OFFSET_RELEASE  3

/* Version of softwre, A indicate a version not protected by hardware watchdog */
#if (TSK_CNFG_DISABLE_HW_WATCHDOG == 0)
        #define SOFTWARE_VERSION_LETTER     'V'             /* Validate version for delivery */
#else
    #define SOFTWARE_VERSION_LETTER     'A'                 /* Debug version only, no watchdog protection */
#endif
unsigned char version[4]={SOFTWARE_VERSION_LETTER, 0, 0, 9};

/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/


/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/

#endif /* TASK_INC_VERSION_H_ */

/**\} */
/**\} */

/* EOF */
