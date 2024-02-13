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
#include <task.h>

#include <stm32_lcd.h>
#include <stm32wb5mm_dk_lcd.h>
#include <stm32wb5mm_dk.h>

#include <YACSGL.h>
#include <YACSGL_font_8x16.h>
#include <YACSGL_font_5x7.h>
#include <YACSWL.h>
#include <ssd1315.h>

#include "tsk_common.h"
#include "tsk_config.h"
#include "tsk_HMI_screen.h"
#include "tsk_HMI_status_bar.h"
#include "tsk_HMI_screen_main.h"


/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static YACSGL_frame_t hmi_lcd_frame = {0};
static tskHMI_status_bar_data_t status_bar_data = {0};
static tskHMI_screen_main_t     screen_main_data = {0};

static YACSWL_widget_t hmi_root_widget={0};
static YACSWL_label_t  hmi_screen_label = {0};
static YACSWL_widget_t hmi_screen_area_widget = {0};

/* Register screens here */
static tsk_HMI_screen_t hmi_screens[] = { {"Ambient temperature",
                                            (void*)&screen_main_data,
                                            vHMISM_init,
                                            vHMISM_enter_screen,
                                            vHMISM_leave_screen,
                                            vHMISM_update}
                                        };

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void HMI_init_display(void);
void HMI_init_widget(void);
void HMI_set_screen_label(const char* const text);

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

    /* Init widget */
    HMI_init_widget();

    while (1) /* Task loop */
    {
        /* TODO handle incomming messages */

        /* TODO handle transition between screen */
        
        /* TODO handle screen refresh */

    	vTaskDelay(50/portTICK_RATE_MS);

        /* Refresh status bar */
        vHMISB_update(&status_bar_data);
        /* TODO check whether it is only current screen */
        /* Refresh all screen */
        for (uint8_t i = 0; i < sizeof(hmi_screens)/sizeof(tsk_HMI_screen_t); i++)
        {
            hmi_screens[i].update(hmi_screens[i].data);
        }

        YACSWL_widget_draw(&hmi_root_widget, &hmi_lcd_frame);
        BSP_LCD_Refresh(0);

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
    
     /* Set LCD ON */
    BSP_LCD_DisplayOn(0);

    return;
}

void HMI_init_widget(void)
{
    /* Create root widget */
    YACSWL_widget_init(&hmi_root_widget);
    YACSWL_widget_set_size(&hmi_root_widget, hmi_lcd_frame.frame_x_width - 1u, hmi_lcd_frame.frame_y_heigth - 1u);
    YACSWL_widget_set_border_width(&hmi_root_widget, 0u);

    /* Set background and foreground TODO make it configurable */
    YACSWL_widget_set_foreground_color(&hmi_root_widget, YACSGL_P_BLACK);
    YACSWL_widget_set_background_color(&hmi_root_widget, YACSGL_P_WHITE);

    /* Init status bar */
    vHMISB_init(&status_bar_data, &hmi_root_widget);

    /* TODO export label and screen area widget into tsk_HMI_screen */
    /* Init screen label widget on top */
    YACSWL_label_init(&hmi_screen_label);
    YACSWL_label_set_font(&hmi_screen_label, &YACSGL_font_5x7);
    HMI_set_screen_label("title");
    YACSWL_widget_set_border_width(&(hmi_screen_label.widget), 0u);
    YACSWL_widget_add_child(&hmi_root_widget, &hmi_screen_label.widget);

    /* Init screen area widget */
    YACSWL_widget_init(&hmi_screen_area_widget);
    YACSWL_widget_set_size(&hmi_screen_area_widget, 
                                hmi_lcd_frame.frame_x_width, 
                                (hmi_lcd_frame.frame_y_heigth
                                        - (YACSWL_widget_get_height(&(hmi_screen_label.widget))
										+ u16HMISB_get_height()
										+ 7u))
						   );
    YACSWL_widget_set_pos(&(hmi_screen_area_widget),
                                0u,
                                YACSWL_widget_get_height(&(hmi_screen_label.widget))- 2u);
    YACSWL_widget_set_border_width(&hmi_screen_area_widget, 0u);
    /* Add screen child to root widget */
    YACSWL_widget_add_child(&hmi_root_widget, &hmi_screen_area_widget);

    /* Init each screens */
    for (uint8_t i = 0; i < sizeof(hmi_screens)/sizeof(tsk_HMI_screen_t); i++)
    {
        hmi_screens[i].init(hmi_screens[i].data, &hmi_screen_area_widget);
        if (i != 0)
        {
            /* For all but first screen, indicate to leave the screen */
            hmi_screens[i].leave_screen();
        }
        else
        {
            /* For first screen indicate to enter the screen */
            hmi_screens[i].enter_screen();
            HMI_set_screen_label(hmi_screens[i].title);
        }
    }

    /* Finally draw widgets */
    YACSWL_widget_draw(&hmi_root_widget, &hmi_lcd_frame);

    /* Refresh LCD screen */
    BSP_LCD_Refresh(0);

    return;
}

void HMI_set_screen_label(const char* const text)
{
    /* Set Text */
    YACSWL_label_set_text(&hmi_screen_label, text);

    /* Center on screen */
    YACSWL_widget_set_pos(&(hmi_screen_label.widget), 
                            (YACSWL_widget_get_width(&hmi_root_widget) -
                                    YACSWL_widget_get_width(&hmi_screen_label.widget)) /2,
                            0u);
    return;
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
