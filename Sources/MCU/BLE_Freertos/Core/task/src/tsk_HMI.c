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
#include <stdbool.h>
#include <FreeRTOS.h>
#include <HMI_screen.h>
#include <HMI_screen_main.h>
#include <HMI_status_bar.h>
#include <HMI_screen_ctrl_mode.h>
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
#include "tsk_TOF.h"


/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct
{
    uint8_t         cur_screen_idx;
    portTickType    screen_change_tick;
    bool            edit_in_progress;
    tskHMI_range_t  range;
}tsk_hmi_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static YACSGL_frame_t hmi_lcd_frame = {0};
static tskHMI_status_bar_data_t status_bar_data = {0};
static HMI_screen_main_t        screen_main_data = {0};
static HMI_screen_ctrl_mode_sts_t   screen_ctrl_mode_data = {0};

static YACSWL_widget_t hmi_root_widget={0};
static YACSWL_label_t  hmi_screen_label = {0};
static YACSWL_widget_t hmi_screen_area_widget = {0};

static xQueueHandle         queue_btn_to_hmi = 0;
static tsk_hmi_status_t     tsk_status = {0};

/* Register screens here */
static tsk_HMI_screen_t hmi_screens[] = {   /* First screen in array is the main screen */
                                            {
                                                (void*)&screen_main_data,
                                                &hmi_main_metadata
                                            }
                                            ,{
                                                (void*)&screen_ctrl_mode_data,
                                                &hmi_ctrl_mode_metadata
                                            }
                                        };

#define HMI_NB_SCREEN   (sizeof(hmi_screens)/sizeof(tsk_HMI_screen_t))

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
void HMI_init_display(void);
void HMI_init_widget(void);
void HMI_set_screen_label(const char* const text);

void HMI_handle_incomming_messages(tskHMI_TaskParam_t* task_param, 
                                        xQueueSetHandle queue_set_hdl, 
                                        uint16_t timeout_ms);

void HMI_handle_incomming_messages_feedback(tskHMI_TaskParam_t* task_param);

void HMI_handle_incomming_messages_fdbk_temperature(tskHMI_TaskParam_t* task_param, 
                                        const tskHMI_msg_fdbk_pld_temperature_t* const temp_pld);

void HMI_handle_incomming_messages_btn(tskHMI_TaskParam_t* task_param);
void HMI_handle_incomming_messages_range(tskHMI_TaskParam_t* task_param);

void HMI_go_to_main_screen(void);
void HMI_go_to_next_screen(void);
void HMI_evaluate_autotransition_to_main(void);
void HMI_enter_leaved_edit_mode(void);
void HMI_cancel_edit_mode(void);

void HMI_init_button(void);
void BSP_PB_Callback(Button_TypeDef Button);

/******************** API FUNCTIONS ******************************************/
void vHMI_task(void* pv_param_task)
{
    tskHMI_TaskParam_t* task_param;
    uint16_t elapsed_time_ms = 0;
    tskCommon_Hb_t heartbeat = {0};
    portTickType hb_sending_tick = 0;    
    portBASE_TYPE ret = pdFAIL;
    xQueueSetHandle queue_set_hdl = 0;

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

    /* Create queue set to listen from multiple queue */
    queue_set_hdl = xQueueCreateSet(TSK_CNFG_QUEUE_SET_HMI_SIZE);
    /* Check for an error */
    if (queue_set_hdl == 0)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    /* Add queue to queue set */
    ret = xQueueAddToSet(task_param->queue_hmi_feedback, queue_set_hdl);
    if (ret != pdPASS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    ret = xQueueAddToSet(task_param->queue_hmi_btn, queue_set_hdl);
    if (ret != pdPASS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    ret = xQueueAddToSet(task_param->queue_hmi_range, queue_set_hdl);
    if (ret != pdPASS)
    {
        /* Enter error loop */
        vTskCommon_ErrorLoop();
    }
    /* TODO add here new queue (ex TOF sensor) */

    /* Now ready to set queue for BTN irq */
    queue_btn_to_hmi = task_param->queue_hmi_btn;

    while (1) /* Task loop */
    {
        /* Compute remaining time before feeding the watchdog */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* If there is enought time before feeding watchdog */
        if (elapsed_time_ms <= HMI_HB_SEND_TIME_MS)
        {
            /* Read incoming messages */
            if ((HMI_HB_SEND_TIME_MS - elapsed_time_ms) > HMI_TASK_LOOP_TIME_MS)
            {
                /* Delay before feeding the watchdog is greater than the desired task loop duration */
                HMI_handle_incomming_messages(task_param, queue_set_hdl, HMI_TASK_LOOP_TIME_MS);
            }
            else
            {
                /* Delay before feeding the watchdog is lower than the desired task loop duration, only wait message for the remaining amount of time */
                HMI_handle_incomming_messages(task_param, queue_set_hdl,
                                                (HMI_TASK_LOOP_TIME_MS - elapsed_time_ms));
            }
        }

        /* handle transition back to main */
        HMI_evaluate_autotransition_to_main();

        /* Refresh status bar */
        vHMISB_update(&status_bar_data);
        /* Refresh current screen */
        hmi_screens[tsk_status.cur_screen_idx].metadata->update(hmi_screens[tsk_status.cur_screen_idx].data,
                                                                    &tsk_status.range);

        YACSWL_widget_draw(&hmi_root_widget, &hmi_lcd_frame);
        BSP_LCD_Refresh(0);

        /* Compute elapsed time since last Heartbeat message */
        elapsed_time_ms = (xTaskGetTickCount() - hb_sending_tick) * portTICK_RATE_MS;

        /* Avoid flooding watchdog with HB messsages (send and increment only if enough time has elapsed since last sending */
        if (elapsed_time_ms > HMI_HB_SEND_TIME_MS)
        {
            /* Increment HB counter */
            heartbeat.hb_counter = heartbeat.hb_counter + 1;

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
    YACSWL_widget_set_border_width(&(hmi_screen_label.widget), 0u);
    YACSWL_widget_add_child(&hmi_root_widget, &hmi_screen_label.widget);
    HMI_set_screen_label("title");

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
    for (uint8_t i = 0; i < HMI_NB_SCREEN; i++)
    {
        hmi_screens[i].metadata->init(hmi_screens[i].data, &hmi_screen_area_widget);
        if (i != 0)
        {
            /* For all but first screen, indicate to leave the screen */
            hmi_screens[i].metadata->leave_screen();
        }
        else
        {
            /* For first screen indicate to enter the screen */
            hmi_screens[i].metadata->enter_screen();
            HMI_set_screen_label(hmi_screens[i].metadata->title);
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
    YACSWL_widget_center_width_in_parent(&(hmi_screen_label.widget));

    return;
}

void HMI_handle_incomming_messages(tskHMI_TaskParam_t* task_param, 
                                        xQueueSetHandle queue_set_hdl, 
                                        uint16_t timeout_ms)
{
    xQueueSetMemberHandle queue_hdl_data_available;
    /* Check parameters */
    if (task_param == NULL)
    {
        /* Error null pointer */
        return;
    }
    if (queue_set_hdl == NULL)
    {
        /* Error null pointer */
        return;
    }

    /* Look for available message on Queues */
    queue_hdl_data_available = xQueueSelectFromSet(queue_set_hdl, timeout_ms / portTICK_RATE_MS);

    /* Message From Autopilot */
    if(queue_hdl_data_available == task_param->queue_hmi_feedback)
    {
        /* Handle message from Autopilot */
        HMI_handle_incomming_messages_feedback(task_param);
    }
    else if(queue_hdl_data_available == task_param->queue_hmi_btn)
    {
        HMI_handle_incomming_messages_btn(task_param);
    }
    else if(queue_hdl_data_available == task_param->queue_hmi_range)
    {
        HMI_handle_incomming_messages_range(task_param);
    }
}

void HMI_handle_incomming_messages_feedback(tskHMI_TaskParam_t* task_param)
{
    static tskHMI_msg_fdbk_msg_t feedback_msg = {0};
    portBASE_TYPE ret = 0;

    /* Retrieve message from queue */
    ret = xQueueReceive(task_param->queue_hmi_feedback, (void*) &feedback_msg, 0);

    if(ret == pdPASS)
    {
        /* Switch message according to id */
        switch(feedback_msg.header.fdbk_id)
        {
            case HMI_MSG_FDBK_ID_TEMP:
                HMI_handle_incomming_messages_fdbk_temperature(task_param,
                                                &(feedback_msg.payload.temp_pld));
                break;
            default:
                /* Drop message */
                break;
        }
    }
}

void HMI_handle_incomming_messages_fdbk_temperature(tskHMI_TaskParam_t* task_param, 
                                            const tskHMI_msg_fdbk_pld_temperature_t* const temp_pld)
{
    /* Update data */
    screen_main_data.ambient_temperature = temp_pld->temperature;
}

void HMI_handle_incomming_messages_btn(tskHMI_TaskParam_t* task_param)
{
    static Button_TypeDef btn_pressed = BUTTON_USER1;
    portBASE_TYPE ret = 0;

    /* Retrieve message from queue */
    ret = xQueueReceive(task_param->queue_hmi_btn, (void*) &btn_pressed, 0);

    if(ret == pdPASS)
    {
        switch(btn_pressed)
        {
            case BUTTON_USER1:
                /* Go to next screen */
                HMI_go_to_next_screen();
                break;
            case BUTTON_USER2:
                /* Enter / Leave edit mode */
                HMI_enter_leaved_edit_mode();
                break;
            default:
                /* Do nothing */
                break;
        }
    }
}

void HMI_handle_incomming_messages_range(tskHMI_TaskParam_t* task_param)
{
    static tskTOF_queue_msg_t msg_tof = {0};
    portBASE_TYPE ret = 0;

    /* Retrieve message from queue */
    ret = xQueueReceive(task_param->queue_hmi_range, (void*) &msg_tof, 0);

    if(ret == pdPASS)
    {
        tsk_status.range.val = msg_tof.distance_mm;
        tsk_status.range.val_max = msg_tof.distance_max_mm;
        tsk_status.range.val_min = msg_tof.distance_min_mm;
    }
}

void HMI_go_to_main_screen(void)
{
    HMI_cancel_edit_mode();
    hmi_screens[tsk_status.cur_screen_idx].metadata->leave_screen();
    tsk_status.cur_screen_idx = 0;
    hmi_screens[tsk_status.cur_screen_idx].metadata->enter_screen();
    /* Update title */
    HMI_set_screen_label(hmi_screens[tsk_status.cur_screen_idx].metadata->title);
}

void HMI_go_to_next_screen(void)
{
    HMI_cancel_edit_mode();
    hmi_screens[tsk_status.cur_screen_idx].metadata->leave_screen();
    tsk_status.cur_screen_idx++;
    if(tsk_status.cur_screen_idx >= HMI_NB_SCREEN)
    {
        tsk_status.cur_screen_idx = 0;
    }
    hmi_screens[tsk_status.cur_screen_idx].metadata->enter_screen();
    /* Update title */
    HMI_set_screen_label(hmi_screens[tsk_status.cur_screen_idx].metadata->title);

    /* Store time to remember when last change occured */
    tsk_status.screen_change_tick = xTaskGetTickCount();
}

void HMI_evaluate_autotransition_to_main(void)
{
    if(tsk_status.edit_in_progress == true)
    {
        /* Do not switch screen if edit is in progress */
        return;
    }
    if(tsk_status.screen_change_tick == 0)
    {
        return;
    }

    if((xTaskGetTickCount() - tsk_status.screen_change_tick) > HMI_DURATION_ROLLBACK_MAIN_SCREEN_MS)
    {
        tsk_status.screen_change_tick = 0;
        HMI_go_to_main_screen();
    }
}

void HMI_enter_leaved_edit_mode(void)
{
    if(     (tsk_status.edit_in_progress == true)
         && (hmi_screens[tsk_status.cur_screen_idx].metadata->validate_edit != NULL)
      )
    {
        tsk_status.edit_in_progress = false;

        hmi_screens[tsk_status.cur_screen_idx].metadata->validate_edit();

        /* TODO Send new setpoint to main */
    }
    else if (       (tsk_status.edit_in_progress == false)
                && (hmi_screens[tsk_status.cur_screen_idx].metadata->enter_edit != NULL)
            )
    {
        tsk_status.edit_in_progress = true;

        hmi_screens[tsk_status.cur_screen_idx].metadata->enter_edit();
    }

    /* In all case store changing time */
    tsk_status.screen_change_tick = xTaskGetTickCount();
}

void HMI_cancel_edit_mode(void)
{
    if(tsk_status.edit_in_progress == false)
    {
        return;
    }

    tsk_status.edit_in_progress = false;

    if(hmi_screens[tsk_status.cur_screen_idx].metadata->cancel_edit != NULL)
    {
        hmi_screens[tsk_status.cur_screen_idx].metadata->cancel_edit();
    }
}

void HMI_init_button(void)
{
	BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);
	BSP_PB_Init(BUTTON_USER2, BUTTON_MODE_EXTI);
}

void BSP_PB_Callback(Button_TypeDef btn)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* If init is not complete */
    if(queue_btn_to_hmi == NULL)
    {
        return;
    }

    /* Send which btn was pressed */
    xQueueSendToBackFromISR(queue_btn_to_hmi, &btn, &xHigherPriorityTaskWoken);

    if(xHigherPriorityTaskWoken != pdFALSE)
    {
        /* Giving queue may have unlocked a task, call scheduler */
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}
/**\} */
/**\} */

/* EOF */
