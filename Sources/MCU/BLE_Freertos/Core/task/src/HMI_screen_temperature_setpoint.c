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
 * \file HMI_screen_temperature_setpoint.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 18, 2024
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
#include "HMI_screen_temperature_setpoint.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <YACSGL.h>
#include <YACSWL.h>
#include <YACSGL_font_8x16.h>

#include "HMI_screen.h"

/******************** CONSTANTS OF MODULE ************************************/
#define HMI_SCREEN_TEMP_STPT_MIN    16  /* Minimal temperature is 16°C */
#define HMI_SCREEN_TEMP_STPT_MAX    30  /* Maximal temperature of 30°C */

#define HMI_SCREEN_TEMP_STPT_RANGE  (HMI_SCREEN_TEMP_STPT_MAX - HMI_SCREEN_TEMP_STPT_MIN)

#define HMI_SCREEN_TEMP_BUFF_CONV_TEMP   (20u)



/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    bool init_complete;
    bool edit_in_progress;
}HMITS_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
tsk_HMI_screen_metadata_t hmi_temperature_stpt_metadata =  {   "Temperature setpoint",
                                                                vHMITS_init,
                                                                vHMITS_enter_screen,
                                                                vHMITS_leave_screen,
                                                                vHMITS_update,
                                                                vHMITS_enter_edit,
                                                                vHMITS_validate_edit,
                                                                vHMITS_cancel_edit
                                                            };

/* Widget of the screen */
static HMITS_status_t           HMI_TS_status = {0};
static YACSWL_widget_t          HMI_TS_root_widget = {0};
static YACSWL_label_t           HMI_TS_lbl_temp_stpt = {0};
static YACSWL_progress_bar_t    HMI_TS_progress_bar = {0};

static HMI_screen_temperature_setpoint_t  HMI_TS_setpoint = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
static void vHMITS_refresh_widget(void);

/******************** API FUNCTIONS ******************************************/
void vHMITS_init(const void* const screen_cm_data, YACSWL_widget_t* const root_widget)
{
    /* Avoid multiple init */
    if(HMI_TS_status.init_complete)
    {
        return;
    }
    if(screen_cm_data == NULL)
    {
        return;
    }
    if(root_widget == NULL)
    {
        return;
    }

    /* Init root widget of screen (usefull to hide all at once) */
    YACSWL_widget_init(&HMI_TS_root_widget);
    YACSWL_widget_set_size(&HMI_TS_root_widget, 
                                YACSWL_widget_get_width(root_widget), 
                                YACSWL_widget_get_height(root_widget));
    YACSWL_widget_set_border_width(&HMI_TS_root_widget, 0u);
    YACSWL_widget_set_pos(&HMI_TS_root_widget, 
                                YACSWL_widget_get_pos_x(root_widget),
                                YACSWL_widget_get_pos_y(root_widget));

    /* Add child to root widget */
    YACSWL_widget_add_child(root_widget, &HMI_TS_root_widget);

    /* Init ambient temperature label */
    YACSWL_label_init(&HMI_TS_lbl_temp_stpt);
    YACSWL_label_set_font(&HMI_TS_lbl_temp_stpt, &YACSGL_font_8x16);
    YACSWL_label_set_text(&HMI_TS_lbl_temp_stpt, "0.0 oC");
    YACSWL_widget_set_border_width(&(HMI_TS_lbl_temp_stpt.widget), 0u);
    YACSWL_widget_add_child(&HMI_TS_root_widget, &HMI_TS_lbl_temp_stpt.widget);    
    YACSWL_widget_center_in_parent(&HMI_TS_lbl_temp_stpt.widget);

    /* Init progress bar used to help user visualize range sensor */
    YACSWL_progress_bar_init(&HMI_TS_progress_bar);
    YACSWL_widget_set_border_width(&HMI_TS_progress_bar.widget, 0u);
    YACSWL_widget_set_size(&HMI_TS_progress_bar.widget, 100u, 1u);
    YACSWL_widget_set_pos(&HMI_TS_progress_bar.widget, 0u, 0u);
    HMI_TS_progress_bar.progress = 75u;
    YACSWL_widget_add_child(&HMI_TS_root_widget, &HMI_TS_progress_bar.widget);
    YACSWL_widget_center_width_in_parent(&HMI_TS_progress_bar.widget);
    YACSWL_widget_set_displayed(&HMI_TS_progress_bar.widget, false);

    /* Add a progress bar to help visualize range distance */

    /* Indicate init is complete */
    HMI_TS_status.init_complete = true;
}

void vHMITS_enter_screen()
{
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }

    /* Display widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_TS_root_widget, true);

    return;
}

void vHMITS_leave_screen()
{
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }

    /* Hide widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_TS_root_widget, false);

    return;
}

void vHMITS_enter_edit(void)
{
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }

    HMI_TS_status.edit_in_progress = true;
}

void vHMITS_validate_edit(tskCommon_hmi_stpt_msg_t* const msg_setpt,
                            xQueueHandle queue_hmi_stpt)
{
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }
    /* Check parameters are valid */
    if(msg_setpt == NULL)
    {
        return;
    }
    if(queue_hmi_stpt == NULL)
    {
        return;
    }

    /* Configure message to be send using provided buffer */
    msg_setpt->header.msg_type = TC_HMI_STPT_TYPE_TEMPERATEURE;
    msg_setpt->payload.temperature.val = HMI_TS_setpoint.new_temperature_stpt;

    /* Send new setpoint to main */
    xQueueSend(queue_hmi_stpt, &msg_setpt, 2); 

    HMI_TS_status.edit_in_progress = false;
}

void vHMITS_cancel_edit(void)
{    
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }

    HMI_TS_status.edit_in_progress = false;
}

void vHMITS_update(const void* const screen_cm_data, tskHMI_range_t* range)
{
    /* Ensure init was completed */
    if(HMI_TS_status.init_complete == false)
    {
        return;
    }
    if(screen_cm_data == NULL)
    {
        return;
    }
    if(range == NULL)
    {
        return;
    }

    const HMI_screen_temp_stpt_sts_t* const data = (const HMI_screen_temp_stpt_sts_t* const) screen_cm_data;

    /* Update progress bar value */
    uint16_t divider = (range->val_max - range->val_min);
    uint8_t progress = 0;
    /* Ensure divider is not 0 */
    if (divider != 0)
    {
        progress = (100 * (range->val - range->val_min)) / divider;
        HMI_TS_progress_bar.progress = progress;
    }
    else
    {
        HMI_TS_progress_bar.progress = 0;
    }

    float temperature_to_display = 0;

    if(HMI_TS_status.edit_in_progress == false)
    {
        temperature_to_display = data->temperature_stpt;
    }
    else
    {
        temperature_to_display = (progress * HMI_SCREEN_TEMP_STPT_RANGE) /100;
        temperature_to_display += HMI_SCREEN_TEMP_STPT_MIN;
    }

    /* Convert value to text */
    static char temp_conf[HMI_SCREEN_TEMP_BUFF_CONV_TEMP] = {};
    snprintf(temp_conf, HMI_SCREEN_TEMP_BUFF_CONV_TEMP, "%2.0f oC", temperature_to_display);

    /* Display value */
    YACSWL_label_set_text(&HMI_TS_lbl_temp_stpt, temp_conf);

    YACSWL_widget_center_in_parent(&HMI_TS_lbl_temp_stpt.widget);
    vHMITS_refresh_widget();

    return;
}


/******************** LOCAL FUNCTIONS ****************************************/
void vHMITS_refresh_widget(void)
{
    YACSWL_widget_set_displayed(&HMI_TS_progress_bar.widget, HMI_TS_status.edit_in_progress);
    YACSWL_widget_set_border_width(&(HMI_TS_lbl_temp_stpt.widget), HMI_TS_status.edit_in_progress ? 1u : 0u);
}

/**\} */
/**\} */

/* EOF */
