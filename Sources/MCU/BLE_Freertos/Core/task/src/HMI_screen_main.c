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
 * \file tsk_HMI_screen_main.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Feb 11, 2024
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
#include <HMI_screen_main.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <YACSGL.h>
#include <YACSWL.h>
#include <YACSGL_font_8x16.h>

/******************** CONSTANTS OF MODULE ************************************/
#define HMI_SM_BUFF_CONV_TEMP   (20u)

/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    bool init_complete;
}HMISM_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static HMISM_status_t  HMI_CM_status = {0};
static YACSWL_widget_t HMI_CM_root_widget = {0};
static YACSWL_label_t  HMI_CM_lbl_ctrl_mode = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
static void vHMISM_display_temp_value(YACSWL_label_t* const lbl_temp_ambiant, float temperature);

/******************** API FUNCTIONS ******************************************/
void vHMISM_init(const void* const screen_main_data, YACSWL_widget_t* const root_widget)
{
    /* Avoid multiple init */
    if(HMI_CM_status.init_complete)
    {
        return;
    }
    if(screen_main_data == NULL)
    {
        return;
    }
    if(root_widget == NULL)
    {
        return;
    }

    /* Init root widget of screen (usefull to hide all at once) */
    YACSWL_widget_init(&HMI_CM_root_widget);
    YACSWL_widget_set_size(&HMI_CM_root_widget, 
                                YACSWL_widget_get_width(root_widget), 
                                YACSWL_widget_get_height(root_widget));
    YACSWL_widget_set_border_width(&HMI_CM_root_widget, 0u);
    YACSWL_widget_set_pos(&HMI_CM_root_widget, 
                                YACSWL_widget_get_pos_x(root_widget),
                                YACSWL_widget_get_pos_y(root_widget));

    /* Add child to root widget */
    YACSWL_widget_add_child(root_widget, &HMI_CM_root_widget);

    /* Init ambient temperature label */
    YACSWL_label_init(&HMI_CM_lbl_ctrl_mode);
    YACSWL_label_set_font(&HMI_CM_lbl_ctrl_mode, &YACSGL_font_8x16);
    YACSWL_label_set_text(&HMI_CM_lbl_ctrl_mode, "29.0 oC");
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), 0u);
    YACSWL_widget_add_child(&HMI_CM_root_widget, &HMI_CM_lbl_ctrl_mode.widget);    
    YACSWL_widget_center_in_parent(&HMI_CM_lbl_ctrl_mode);

    /* Indicate init is complete */
    HMI_CM_status.init_complete = true;
}

void vHMISM_enter_screen()
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }

    /* Display widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_CM_root_widget, true);

    return;
}

void vHMISM_leave_screen()
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }

    /* Hide widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_CM_root_widget, false);

    return;
}

void vHMISM_update(const void* const screen_main_data)
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }
    if(screen_main_data == NULL)
    {
        return;
    }
    const HMI_screen_main_t* const data = (const HMI_screen_main_t* const) screen_main_data;

    /* Update temperature */
    vHMISM_display_temp_value(&HMI_CM_lbl_ctrl_mode, data->ambient_temperature);

    return;
}

/******************** LOCAL FUNCTIONS ****************************************/
static void vHMISM_display_temp_value(YACSWL_label_t* const lbl_temp, float temperature)
{
    static char temp_conf[HMI_SM_BUFF_CONV_TEMP] = {};

    snprintf(temp_conf, HMI_SM_BUFF_CONV_TEMP, "%2.1f oC", temperature);

    YACSWL_label_set_text(lbl_temp, temp_conf);
}

/**\} */
/**\} */

/* EOF */
