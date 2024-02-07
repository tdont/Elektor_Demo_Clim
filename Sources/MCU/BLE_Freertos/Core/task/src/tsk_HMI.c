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
/**\addtogroup ModuleName_Internal_Documentation
 * \{ */
/**\addtogroup SubModuleName
 * \{ */

/**
 * \file tsk_HMI.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 7, 2024
 */

 /**
 * \cond INTERNAL_DOC
 *
 * \mainpage Internal documentation of module
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
#include "tsk_HMI.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>

#include <stm32_lcd.h>
#include <stm32wb5mm_dk_lcd.h>
#include <stm32wb5mm_dk.h>

#include "tsk_common.h"
#include "tsk_config.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void HMI_init_display(void);
void HMI_init_button(void);
void BSP_PB_Callback(Button_TypeDef Button);

/******************** API FUNCTIONS ******************************************/
void vHMI_task(void* pv_param_task)
{
    tskHMI_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;

    heartbeat.tsk_id = TSK_CNFG_MONITORED_ID_HMI;
    heartbeat.hb_counter = 0;

    /* Check parameters */
    if (pv_param_task == NULL)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }

    task_param = (tskHMI_TaskParam_t*) pv_param_task;

    /* TODO perform init of the HMI part here */
    /* Init Display */
    HMI_init_display();

    while (1) /* Task loop */
    {
        /* TODO handle incomming messages */

        /* TODO handle transition between screen */
        
        /* TODO handle screen refresh */


        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > HMI_HB_SEND_TIME_MS)
        {
            /* Increment HB counter */
            heartbeat.hb_counter = heartbeat.hb_counter + 1;

            /*-----REQ_SW_S000001_APRT_011-----*/
            /* Send heartbeat */
            xQueueSend(task_param->queue_hb_to_watchdog, &heartbeat, 0); /* Don't wait on queue*/

            /* Store sending time */
            hb_sending_tick = xTaskGetTickCount();
        }
    }
}

/******************** LOCAL FUNCTIONS ****************************************/
void HMI_init_display(void)
{
    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);

    uint32_t x_size, y_size;

    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    UTIL_LCD_SetFuncDriver(&LCD_Driver); /* SetFunc before setting device */
    UTIL_LCD_SetDevice(0);            /* SetDevice after funcDriver is set */
    BSP_LCD_DisplayOn(0);

    UTIL_LCD_SetFont(&Font12);
    /* Set the LCD Text Color */
    UTIL_LCD_SetTextColor(SSD1315_COLOR_WHITE);
    UTIL_LCD_SetBackColor(SSD1315_COLOR_BLACK);
    BSP_LCD_Clear(0,SSD1315_COLOR_BLACK);
    BSP_LCD_Refresh(0);

    UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"Elektor demo clim", CENTER_MODE);
    BSP_LCD_Refresh(0);
}

void HMI_init_button(void)
{
	BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);
	BSP_PB_Init(BUTTON_USER2, BUTTON_MODE_EXTI);
}

void BSP_PB_Callback(Button_TypeDef btn)
{
	if(btn == BUTTON_USER1)
	{
		UTIL_LCD_DisplayStringAt(0, 15, (uint8_t *)"Btn 1 pressed", CENTER_MODE);
	}
	else
	{
		UTIL_LCD_DisplayStringAt(0, 30, (uint8_t *)"Btn 2 pressed", CENTER_MODE);
	}
	BSP_LCD_Refresh(0);
}
/**\} */
/**\} */

/* EOF */
