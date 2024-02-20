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

static HMICM_status_t  HMI_CM_status = {0};
static YACSWL_widget_t HMI_CM_root_widget = {0};
static YACSWL_label_t  HMI_CM_lbl_ctrl_mode = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/

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
    YACSWL_widget_center_in_parent(&HMI_CM_lbl_ctrl_mode);

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
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), 1u);
}

void vHMICM_validate_edit(void)
{
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), 0u);
}

void vHMICM_cancel_edit(void)
{
    YACSWL_widget_set_border_width(&(HMI_CM_lbl_ctrl_mode.widget), 0u);
}

void vHMICM_update(const void* const screen_cm_data)
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
    const HMI_screen_ctrl_mode_t* const data = (const HMI_screen_ctrl_mode_t* const) screen_cm_data;

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

    YACSWL_widget_center_in_parent(&HMI_CM_lbl_ctrl_mode);

    return;
}

/******************** LOCAL FUNCTIONS ****************************************/

/**\} */
/**\} */

/* EOF */
