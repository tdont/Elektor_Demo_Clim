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

/******************** INCLUDES ***********************************************/
#include "tsk_WDGT.h"
#include "tsk_common.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/
#define WDGT_DEBUG              1
#define WDGT_TASK_STATS_NB      20

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
static void vWDGT_iRetrieveHbMessage(xQueueHandle hbQueue, uint8_t* const hb_counter, uint32_t* const aliveTskBitmask, uint16_t timeoutMs, uint8_t nbMaxTaskMonitored);

static void vWDGT_icheckWacthdogFeeding(uint32_t aliveTskBitmask, uint32_t maskForAliveTask, uint32_t excludeMonitoredTskBitmask);
/******************** API FUNCTIONS ******************************************/
void vWDGT_task(void *pvParameters)
{
    uint8_t hbCounters[WDGT_TASK_NB_MAX_MONITORED_TSKS] = {0};
    uint32_t cycleCount = 0;
    tskWDGT_TaskParam_t* pstTaskParam = 0;
    uint8_t i = 0;
    uint32_t currentTimeMs = 0;
    uint32_t tmpCycleCount = 0;
    uint16_t timeoutMs = 0;

#if WDGT_TASK_NB_MAX_MONITORED_TSKS <= 32
    uint32_t aliveTskBitmask = 0;
    uint32_t maskForAliveTask = 0;
#else
    uint64_t aliveTskBitmask = 0;
    uint64_t maskForAliveTask = 0;
#error The number of task monitored exeed the authorized number of 32. Change this message to Warning if you know what you are doing (think of changing type to every function call using those variables)
#endif

    /* Check parameters */
    if (pvParameters == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    /* Cast parameters */
    pstTaskParam = (tskWDGT_TaskParam_t*) pvParameters;

    /* Check number of tasks to monitor does not exceed maximum possible */
    if (pstTaskParam->u8_nbMonitoredTasks > WDGT_TASK_NB_MAX_MONITORED_TSKS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    /* Build the mask for aliveness checking */
    for (i = 0; i < pstTaskParam->u8_nbMonitoredTasks; i++)
    {
        maskForAliveTask |= (1 << i);
    }

    /* Watchdog loop */
    while (1)
    {
        /* Retrieve current time */
        currentTimeMs = xTaskGetTickCount() * portTICK_RATE_MS;

        /* Compute new timeout */
        timeoutMs = WDGT_HARD_WDG_FEED_TIME_MS - (currentTimeMs % WDGT_HARD_WDG_FEED_TIME_MS);

        /* Retrieve message and report if task is alived (Timeout protected call) */
        vWDGT_iRetrieveHbMessage(pstTaskParam->queueHbFromMonitoredTask, hbCounters, &aliveTskBitmask, timeoutMs, pstTaskParam->u8_nbMonitoredTasks);

        /* Retrieve current time */
        currentTimeMs = xTaskGetTickCount() * portTICK_RATE_MS;

        /* Compute Cycle count */
        tmpCycleCount = currentTimeMs / WDGT_HARD_WDG_FEED_TIME_MS;

        /* If Cycle count is different, then enough time has elpased */
        if (tmpCycleCount !=  cycleCount)
        {
            /* Check whether the wachtdog can be fed */
            vWDGT_icheckWacthdogFeeding(aliveTskBitmask, maskForAliveTask, pstTaskParam->excludeMonitoredTskBitmask);

            /* Store new cycle count */
            cycleCount = tmpCycleCount;

            /* Reset alive Bitmask to 0 */
            aliveTskBitmask = 0;
        }
    }

    return;
}

/******************** LOCAL FUNCTIONS ****************************************/
static void vWDGT_iRetrieveHbMessage(xQueueHandle hbQueue, uint8_t* const hbCounters, uint32_t* const aliveTskBitmask, uint16_t timeoutMs, uint8_t nbMaxTaskMonitored)
{
    tskCommon_Hb_t stHbMsg = {0};
    portBASE_TYPE ret = 0;
    uint8_t withinWindow = 0;

    /* Check parameters */
    if (   (hbCounters == 0)
        || (aliveTskBitmask == 0)
       )
    {
        /* Null pointer  return immediately */
        return;
    }


    /* Retrieve message from the Queue (Timeout protected call) */
    ret = xQueueReceive(hbQueue, (void*) &stHbMsg, (timeoutMs / portTICK_RATE_MS));

    /* Check whether data was received */
    if (ret == pdPASS)
    {
        /* Ensure the Task Id is below the maximum possible */
        if (stHbMsg.tsk_id < nbMaxTaskMonitored)
        {

            /* Check whether the heartbeat match the requirement (is within the windows) */
            /* Distinguish the case far from 0 reset */
            if (stHbMsg.hb_counter >= WDGT_HB_TOLERATED_WINDOW)
            {
                /* If previous value is within the windows */
                if ( (hbCounters[stHbMsg.tsk_id] < stHbMsg.hb_counter)
                    && (hbCounters[stHbMsg.tsk_id] >= (stHbMsg.hb_counter - WDGT_HB_TOLERATED_WINDOW))
                   )
                {
                    /* Value is within range */
                    withinWindow = 1;
                }
            }
            /* Within the overflow reset to 0 of heartbeat counters */
            else
            {
                if (   (hbCounters[stHbMsg.tsk_id] >= (255 - (WDGT_HB_TOLERATED_WINDOW - stHbMsg.hb_counter)))
                    || (hbCounters[stHbMsg.tsk_id] >= 0 && hbCounters[stHbMsg.tsk_id] < stHbMsg.hb_counter)
                    )
                {
                    /* Value is within range */
                    withinWindow = 1;
                }
            }

            /* If detected inside window report task as alive */
            if (withinWindow != 0)
            {
                *aliveTskBitmask = *aliveTskBitmask | (1 << stHbMsg.tsk_id);
            }

            /* Store the new heartbeat value */
            hbCounters[stHbMsg.tsk_id] = stHbMsg.hb_counter;
        }
    }

    return;
}


static void vWDGT_icheckWacthdogFeeding(uint32_t aliveTskBitmask, uint32_t maskForAliveTask, uint32_t excludeMonitoredTskBitmask)
{
    /* If Both are equal, all the task were seen alived */
    if ((aliveTskBitmask | excludeMonitoredTskBitmask) == maskForAliveTask)
    {
        /* TODO Feed the HW watchdog */

        /* TODO Toogle life led */
    }
}


/**\} */
/**\} */

/* EOF */
