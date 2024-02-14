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
#ifndef INC_TSK_CONFIG_H_
#define INC_TSK_CONFIG_H_

/******************** INCLUDES ***********************************************/
#include <FreeRTOSConfig.h>

/******************** CONSTANTS OF MODULE ************************************/
#define TSK_CNFG_DISABLE_HW_WATCHDOG                1

#define TSK_CNFG_MONITORED_ID_HMI                  (0)
#define TSK_CNFG_MONITORED_ID_TEMP                 (1)
/* Must be at the end of the task id list (indicates the number of task monitored) */
#define TSK_CNFG_MONITORED_ID_CNT                   (TSK_CNFG_MONITORED_ID_TEMP + 1)

/* Definition of tasks priority */
#define TSK_CNFG_PRIORITY_HMI                   	(configMAX_PRIORITIES - 4)
#define TSK_CNFG_PRIORITY_TEMP                      (configMAX_PRIORITIES - 3)
#define TSK_CNFG_PRIORITY_WDGT                      (2)     /* Lowest priority task of monitored task */
#define TSK_CNFG_PRIORITY_NON_MONITORED             (1)     /* Non monitored task (if any) Just above idle task */

/* Definition of tasks StackSize */
#define TSK_CNFG_STACKSIZE_WDGT                     (128)
#define TSK_CNFG_STACKSIZE_HMI                      (368)
#define TSK_CNFG_STACKSIZE_TEMP                     (256)

/* Definition of MSGOBJ (1 is reserved for CMCH Task) */

/* Definition of tasks name for debug */
#define TSK_CNFG_NAME_WDGT               		"TaskWDGT"
#define TSK_CNFG_NAME_HMI               		"TaskHMI"
#define TSK_CNFG_NAME_TEMP                      "TaskTEMP"

/* Definition of Queue name for debug */
#define TSK_CNFG_QUEUE_NAME_HB_TO_WDG           "QueHbToWdg"

#define TSK_CNFG_QUEUE_LENGTH_HB_TO_WDG         (TSK_CNFG_MONITORED_ID_CNT * 3)

/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/


/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/

#endif /* TSK_CONFIG_H_ */

/**\} */
/**\} */

/* EOF */
