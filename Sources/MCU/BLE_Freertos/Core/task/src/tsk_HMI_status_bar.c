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
 * \file tsk_HMI_status_bar.c
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
#include "tsk_HMI_status_bar.h"

#include <stdbool.h>
#include <stddef.h>

#include <YACSGL.h>
#include <YACSWL.h>
#include <YACSGL_font_5x7.h>

/******************** CONSTANTS OF MODULE ************************************/
#define HMI_SB_HEIGHT   (12u)
/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    bool init_complete;
}HMISB_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
static HMISB_status_t  HMI_SB_status = {0};
static YACSWL_widget_t HMI_SB_root_widget = {0};
static YACSWL_label_t  HMI_SB_lbl_manual_ble = {0};
static YACSWL_label_t  HMI_SB_lbl_temp_setpoint = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/

/******************** API FUNCTIONS ******************************************/
void vHMISB_init(const tskHMI_status_bar_data_t* const status_bar_data, YACSWL_widget_t* const root_widget)
{
    /* Avoid multiple init */
    if(HMI_SB_status.init_complete)
    {
        return;
    }
    if(status_bar_data == NULL)
    {
        return;
    }
    if(root_widget == NULL)
    {
        return;
    }

    /* Init status bar widget here */
    YACSWL_widget_init(&HMI_SB_root_widget);
    YACSWL_widget_set_size(&HMI_SB_root_widget, YACSWL_widget_get_width(root_widget), HMI_SB_HEIGHT);
    YACSWL_widget_set_border_width(&HMI_SB_root_widget, 1u);
    YACSWL_widget_set_pos(&HMI_SB_root_widget, 0u, YACSWL_widget_get_height(root_widget) 
                                    - YACSWL_widget_get_height(&HMI_SB_root_widget) +1);

    /* Add child to root widget */
    YACSWL_widget_add_child(root_widget, &HMI_SB_root_widget);

    /* Init subwidget */

    /* Create label for manual/ble mode */
    YACSWL_label_init(&HMI_SB_lbl_manual_ble);
    YACSWL_label_set_font(&HMI_SB_lbl_manual_ble, &YACSGL_font_5x7);
    YACSWL_label_set_text(&HMI_SB_lbl_manual_ble, "BLE setpoint");
    YACSWL_widget_set_border_width(&(HMI_SB_lbl_manual_ble.widget), 0u);
    YACSWL_widget_set_pos(&(HMI_SB_lbl_manual_ble.widget), 
                                1, 
                                1);

    /* Attach label to status bar root widget */
    YACSWL_widget_add_child(&HMI_SB_root_widget, &(HMI_SB_lbl_manual_ble.widget));


    /* Create label for clim mode */

    /* Create label for temperature setpoint */


    return;
}

void vHMISB_update(const tskHMI_status_bar_data_t* const status_bar_data) 
{
    /* Ensure init was completed */
    if(HMI_SB_status.init_complete == false)
    {
        return;
    }
    if(status_bar_data == NULL)
    {
        return;
    }

    /* Update widget value */

    /* Refresh screen */

    return;
}

/******************** LOCAL FUNCTIONS ****************************************/

/**\} */
/**\} */

/* EOF */
