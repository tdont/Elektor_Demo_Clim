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
 * \file HMI_screen_ctrl_mode.c
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
#include "HMI_screen_ctrl_mode.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <YACSGL.h>
#include <YACSWL.h>
#include <YACSGL_font_8x16.h>

#include "HMI_screen.h"

/******************** CONSTANTS OF MODULE ************************************/
typedef struct 
{
    bool init_complete;
    bool edit_in_progress;
}HMICM_status_t;
/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/

/******************** GLOBAL VARIABLES OF MODULE *****************************/
tsk_HMI_screen_metadata_t hmi_ctrl_mode_metadata =  {   "Control mode",
                                                        vHMICM_init,
                                                        vHMICM_enter_screen,
                                                        vHMICM_leave_screen,
                                                        vHMICM_update,
                                                        vHMICM_enter_edit,
                                                        vHMICM_validate_edit,
                                                        vHMICM_cancel_edit
                                                    };

/* Widget of the screen */
static HMICM_status_t           HMI_CM_status = {0};
static YACSWL_widget_t          HMI_CM_root_widget = {0};
static YACSWL_label_t           HMI_CM_lbl_ctrl_mode = {0};
static YACSWL_progress_bar_t    HMI_CM_progress_bar = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/
static void vHMICM_refresh_widget(void);

/******************** API FUNCTIONS ******************************************/
void vHMICM_init(const void* const screen_cm_data, YACSWL_widget_t* const root_widget)
{
    /* Avoid multiple init */
    if(HMI_CM_status.init_complete)
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
    YACSWL_label_set_text(&HMI_CM_lbl_ctrl_mode, "BLE mode");
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), 0u);
    YACSWL_widget_add_child(&HMI_CM_root_widget, &HMI_CM_lbl_ctrl_mode.widget);    
    YACSWL_widget_center_in_parent(&HMI_CM_lbl_ctrl_mode.widget);

    /* Init progress bar used to help user visualize range sensor */
    YACSWL_progress_bar_init(&HMI_CM_progress_bar);
    YACSWL_widget_set_border_width(&HMI_CM_progress_bar.widget, 0u);
    YACSWL_widget_set_size(&HMI_CM_progress_bar.widget, 100u, 1u);
    YACSWL_widget_set_pos(&HMI_CM_progress_bar.widget, 0u, 0u);
    HMI_CM_progress_bar.progress = 75u;
    YACSWL_widget_add_child(&HMI_CM_root_widget, &HMI_CM_progress_bar.widget);
    YACSWL_widget_center_width_in_parent(&HMI_CM_progress_bar.widget);
    YACSWL_widget_set_displayed(&HMI_CM_progress_bar.widget, false);

    /* Add a progress bar to help visualize range distance */

    /* Indicate init is complete */
    HMI_CM_status.init_complete = true;
}

void vHMICM_enter_screen()
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

void vHMICM_leave_screen()
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

void vHMICM_enter_edit(void)
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }

    HMI_CM_status.edit_in_progress = true;
}

void vHMICM_validate_edit(void)
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }

    HMI_CM_status.edit_in_progress = false;
}

void vHMICM_cancel_edit(void)
{    
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
    {
        return;
    }

    HMI_CM_status.edit_in_progress = false;
}

void vHMICM_update(const void* const screen_cm_data, tskHMI_range_t* range)
{
    /* Ensure init was completed */
    if(HMI_CM_status.init_complete == false)
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

    const HMI_screen_ctrl_mode_sts_t* const data = (const HMI_screen_ctrl_mode_sts_t* const) screen_cm_data;

    /* Update progress bar value */
    uint16_t divider = (range->val_max - range->val_min);
    /* Ensure divider is not 0 */
    if (divider != 0)
    {
        HMI_CM_progress_bar.progress = (100 * (range->val - range->val_min)) / divider;
    }
    else
    {
        HMI_CM_progress_bar.progress = 0;
    }

    switch(data->ctrl_mode)
    {
        case TC_BLE_MODE_BLE:
            YACSWL_label_set_text(&HMI_CM_lbl_ctrl_mode, "BLE mode");
            break;
        case TC_BLE_MODE_MANUAL:
            YACSWL_label_set_text(&HMI_CM_lbl_ctrl_mode, "MAN mode");
            break;
        default:
            YACSWL_label_set_text(&HMI_CM_lbl_ctrl_mode, "Unknown mode");
            break;
    }

    YACSWL_widget_center_in_parent(&HMI_CM_lbl_ctrl_mode.widget);
    vHMICM_refresh_widget();

    return;
}

/******************** LOCAL FUNCTIONS ****************************************/
void vHMICM_refresh_widget(void)
{
    
    YACSWL_widget_set_displayed(&HMI_CM_progress_bar.widget, HMI_CM_status.edit_in_progress);
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), HMI_CM_status.edit_in_progress ? 1u : 0u);
}
/**\} */
/**\} */

/* EOF */
