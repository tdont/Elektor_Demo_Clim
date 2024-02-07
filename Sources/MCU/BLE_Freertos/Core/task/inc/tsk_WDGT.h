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
* This module handle the hardware watchdog feeding after monitoring tasks aliveness
*
* \section Features
* The main features of the module are described here.
*
* \endcond
*/

/* Prevent multiple inclusions */
#ifndef INC_TSK_WDGT_H_
#define INC_TSK_WDGT_H_

/******************** INCLUDES ***********************************************/
#include <FreeRTOS.h>
#include <queue.h>

/******************** CONSTANTS OF MODULE ************************************/
#define WDGT_TASK_NB_MAX_MONITORED_TSKS      20

#define WDGT_HARD_WDG_TIMER_US          (2000 * 1000)       /* Hardware watchdog timer 2s en µs*/
#define WDGT_HARD_WDG_FEED_TIME_MS      (500)               /* Time in ms indicating when the watchdog shall be fed */
#define WDGT_HB_TOLERATED_WINDOW        (10)                /* A hearbeat counter which value is incremented within this windows compared to the previous value is considered OK */
/* Ensure coherent value */
#if WDGT_HB_TOLERATED_WINDOW <= 1
    #error the WDGT_HB_TOLERATED_WINDOW shall be greater or equal to 2
#endif
#if WDGT_HB_TOLERATED_WINDOW >= 255
    #error the WDGT_HB_TOLERATED_WINDOW shall be lower than 255
#endif


/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
 xQueueHandle queueHbFromMonitoredTask;
 uint8_t      u8_nbMonitoredTasks;
#if TSK_CMN_MONITORED_ID_CNT <= 32
    uint32_t excludeMonitoredTskBitmask;
#else
    uint64_t excludeMonitoredTskBitmask;
#error The number of task monitored exeed the authorized number of 32. Change this message to Warning if you know what you are doing (think of changing type to every function call using those variables)
#endif
}tskWDGT_TaskParam_t;


/******************** GLOBAL VARIABLES OF MODULE *****************************/


/******************** API FUNCTION PROTOTYPE *********************************/
/**
******************************************************************************
* \par Description :
* Task loop for Watchdog Task
*
* \param[in] pvParameters   Pointer the the task parameters
*
*****************************************************************************/
void vWDGT_task(void *pvParameters);

#endif /* INC_TSK_WDGT_H_ */

/**\} */
/**\} */

/* EOF */
