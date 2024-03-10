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
 * \file HMI_screen_about.c
 *
 * File description
 * \author Thibaut DONTAIL
 * \date Mar 3, 2024
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
#include "HMI_screen_about.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <YACSGL.h>
#include <YACSWL.h>
#include <YACSGL_font_8x16.h>
#include <YACSGL_font_5x7.h>

#include "HMI_screen.h"


/******************** CONSTANTS OF MODULE ************************************/
#define HMIA_TXT_BUFFER_MAX_LEN     30

#define HMIA_LABEL_TXT_SOFT_VER     "Soft ver: " 
#define HMIA_LABEL_TXT_AUTHOR       "Author: "
/******************** MACROS DEFINITION **************************************/

/******************** TYPE DEFINITION ****************************************/
typedef struct 
{
    bool init_complete;
}HMIA_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/
tsk_HMI_screen_metadata_t hmi_about_metadata =  {   "About",
                                                        vHMIA_init,
                                                        vHMIA_enter_screen,
                                                        vHMIA_leave_screen,
                                                        vHMIA_update,
                                                        NULL,
                                                        NULL,
                                                        NULL
                                                    };

/* Widget of the screen */
static HMIA_status_t            HMI_A_status = {0};
static YACSWL_widget_t          HMI_A_root_widget = {0};
static YACSWL_label_t           HMI_A_lbl_soft_ver = {0};
// static YACSWL_label_t           HMI_A_lbl_author = {0};

/******************** LOCAL FUNCTION PROTOTYPE *******************************/

/******************** API FUNCTIONS ******************************************/
void vHMIA_init(const void* const screen_a_data, YACSWL_widget_t* const root_widget)
{
    /* Avoid multiple init */
    if(HMI_A_status.init_complete)
    {
        return;
    }
    if(screen_a_data == NULL)
    {
        return;
    }
    if(root_widget == NULL)
    {
        return;
    }

        /* Init root widget of screen (usefull to hide all at once) */
    YACSWL_widget_init(&HMI_A_root_widget);
    YACSWL_widget_set_size(&HMI_A_root_widget, 
                                YACSWL_widget_get_width(root_widget), 
                                YACSWL_widget_get_height(root_widget));
    YACSWL_widget_set_border_width(&HMI_A_root_widget, 0u);
    YACSWL_widget_set_pos(&HMI_A_root_widget, 
                                YACSWL_widget_get_pos_x(root_widget),
                                YACSWL_widget_get_pos_y(root_widget));

    /* Add child to root widget */
    YACSWL_widget_add_child(root_widget, &HMI_A_root_widget);

    /* Init Software version label */
    YACSWL_label_init(&HMI_A_lbl_soft_ver);
    YACSWL_label_set_font(&HMI_A_lbl_soft_ver, &YACSGL_font_8x16);
    YACSWL_label_set_text(&HMI_A_lbl_soft_ver, "Soft ver: X.0.0.0");
    YACSWL_widget_set_border_width(&(HMI_A_lbl_soft_ver.widget), 0u);
    YACSWL_widget_add_child(&HMI_A_root_widget, &HMI_A_lbl_soft_ver.widget);    
    YACSWL_widget_center_in_parent(&HMI_A_lbl_soft_ver.widget);

    /* Init progress bar used to help user visualize range sensor */
    // YACSWL_progress_bar_init(&HMI_A_lbl_author);
    // YACSWL_label_set_font(&HMI_A_lbl_author, &YACSGL_font_8x16);
    // YACSWL_label_set_text(&HMI_A_lbl_author, "Author:");
    // YACSWL_widget_set_pos(&HMI_A_lbl_author.widget, 0u, 0u);
    // YACSWL_widget_add_child(&HMI_A_root_widget, &HMI_A_lbl_author.widget);
    // YACSWL_widget_center_width_in_parent(&HMI_A_lbl_author.widget);

    /* Indicate init is complete */
    HMI_A_status.init_complete = true;

}

void vHMIA_enter_screen(void)
{
    /* Ensure init was completed */
    if(HMI_A_status.init_complete == false)
    {
        return;
    }

    /* Display widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_A_root_widget, true);
}

void vHMIA_leave_screen(void)
{
    /* Ensure init was completed */
    if(HMI_A_status.init_complete == false)
    {
        return;
    }

    /* Hide widget (and subwidget)*/
    YACSWL_widget_set_displayed(&HMI_A_root_widget, false);

    return;
}

void vHMIA_update(const void* const screen_a_data, tskHMI_range_t* range)
{
    /* Ensure init was completed */
    if(HMI_A_status.init_complete == false)
    {
        return;
    }
    if(screen_a_data == NULL)
    {
        return;
    }
    if(range == NULL)
    {
        return;
    }

    const HMI_screen_about_t* const data = (const HMI_screen_about_t* const) screen_a_data;

    /* Variable to store text to be displayed */
    static char soft_ver[HMIA_TXT_BUFFER_MAX_LEN] = {0};
    //static author[HMIA_TXT_BUFFER_MAX_LEN] = {0};
    static char buffer_compose[20] = {0};

    snprintf(buffer_compose, 
                    sizeof(buffer_compose), 
                    "%c.%u.%u.%u", 
                    data->letter, 
                    data->major,
                    data->minor,
                    data->release);

    /* Reset string */
    soft_ver[0]  = 0;
//    author[0] = 0;

    /* Build label for version */
    strncat(soft_ver, HMIA_LABEL_TXT_SOFT_VER, sizeof(soft_ver)-1);
    strncat(soft_ver, buffer_compose, sizeof(soft_ver)-1);

    YACSWL_label_set_text(&HMI_A_lbl_soft_ver, soft_ver);
    YACSWL_widget_center_in_parent(&HMI_A_lbl_soft_ver.widget);


    /* TODO Build label for author */

}

/******************** LOCAL FUNCTIONS ****************************************/

/**\} */
/**\} */

/* EOF */
