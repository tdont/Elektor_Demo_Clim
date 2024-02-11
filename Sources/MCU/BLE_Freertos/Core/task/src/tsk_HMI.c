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

#include <YACSGL.h>
#include <YACSGL_font_8x16.h>
#include <YACSWL.h>
#include <ssd1315.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_HMI_status_bar.h"

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static YACSGL_frame_t hmi_lcd_frame = {0};
static tskHMI_status_bar_data_t status_bar_data = {0};

YACSWL_widget_t hmi_root_widget;

static YACSWL_progress_bar_t progress_bar = {0};

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

    /* Init Button*/
    HMI_init_button();

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
    /* Init LCD screen */
    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);

    uint32_t x_size, y_size;

    /* Retrieve LCD size */
    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);    
    hmi_lcd_frame.frame_x_width = (uint16_t) x_size;
    hmi_lcd_frame.frame_y_heigth= (uint16_t) y_size;

    /* Retrieve framebuffer */
    if(BSP_LCD_GetFrameBuffer(0, &(hmi_lcd_frame.frame_buffer)) != BSP_ERROR_NONE)
    {
        vTskCommon_ErrorLoop();
    }

    // UTIL_LCD_SetFuncDriver(&LCD_Driver); /* SetFunc before setting device */
    // UTIL_LCD_SetDevice(0);            /* SetDevice after funcDriver is set */
    /* Set LCD ON */
    BSP_LCD_DisplayOn(0);
    
    // YACSGL_font_txt_disp(&hmi_lcd_frame, 0u, 0u, YACSGL_P_WHITE, 
    //             &YACSGL_font_8x16, "Elektor demo clim", YACSGL_NEWLINE_ENABLED);
    // YACSGL_rect_fill(&hmi_lcd_frame, 30u, 30u, 39u, 39u, YACSGL_P_WHITE);
    // YACSGL_circle_line(&hmi_lcd_frame, 50u, 50u, 20u, YACSGL_P_WHITE);

    /* Create root widget */
    YACSWL_widget_init(&hmi_root_widget);
    YACSWL_widget_set_size(&hmi_root_widget, hmi_lcd_frame.frame_x_width - 1u, hmi_lcd_frame.frame_y_heigth - 1u);
    YACSWL_widget_set_border_width(&hmi_root_widget, 0u);

    /* Set background and foreground TODO make it configurable */
    YACSWL_widget_set_foreground_color(&hmi_root_widget, YACSGL_P_BLACK);
    YACSWL_widget_set_background_color(&hmi_root_widget, YACSGL_P_WHITE);


    YACSWL_progress_bar_init(&progress_bar);
    YACSWL_widget_set_border_width(&progress_bar.widget, 1u);
    YACSWL_widget_set_size(&progress_bar.widget, 70u, 5u);
    YACSWL_widget_set_pos(&progress_bar.widget, 5u, 5u);

    progress_bar.progress = 75u;
    YACSWL_widget_add_child(&hmi_root_widget, &progress_bar.widget);

    /* Init status bar */
    vHMISB_init(&status_bar_data, &hmi_root_widget);


    /* Finaly draw widgets */
    YACSWL_widget_draw(&hmi_root_widget, &hmi_lcd_frame);



    // UTIL_LCD_SetFont(&Font12);
    // /* Set the LCD Text Color */
    // UTIL_LCD_SetTextColor(SSD1315_COLOR_WHITE);
    // UTIL_LCD_SetBackColor(SSD1315_COLOR_BLACK);
    // BSP_LCD_Clear(0,SSD1315_COLOR_BLACK);

    // UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"Elektor demo clim", CENTER_MODE);
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
