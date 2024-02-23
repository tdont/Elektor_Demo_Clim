/******************************************************************************
* Copyright(c) 2024 Thibaut DONTAIL
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
* along with ELEKTO_DEMO_CLIM.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/******************** FILE HEADER ********************************************/
/**\addtogroup ModuleName_External_Documentation
 * \{ */
/**\addtogroup SubModuleName
 * \{ */

/**
 * \file tsk_WDGT.h
 *
 * File description
 * \author Thibaut DONTAIL
 * \date 21 jan. 2024
 */

/**
* \cond INTERNAL_DOC
*
* \mainpage External documentation of module
*
* \section Description
* This module does what you want him to do....
*
* \section Features
* The main features of the module are described here.
*
* \endcond
*/
/* Prevent multiple inclusions */
#ifndef INC_TSK_COMMON_H_
#define INC_TSK_COMMON_H_

/******************** INCLUDES ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOSConfig.h>

/******************** CONSTANTS OF MODULE ************************************/

/******************** MACROS DEFINITION **************************************/


/******************** TYPE DEFINITION ****************************************/
typedef struct
{
    uint8_t tsk_id;          /* Identification of task */
    uint8_t hb_counter;      /* HeartBeatCounter, Shall keep incrementing */
}tskCommon_Hb_t;

typedef enum
{
    TC_CLIM_MODE_HEAT = 0,
    TC_CLIM_MODE_COLD,
    TC_CLIM_MODE_OFF
}tskCommon_clim_mode_e;

typedef enum
{
    TC_BLE_MODE_BLE = 0,
    TC_BLE_MODE_MANUAL
}tskCommon_ble_mode_e;

typedef enum
{
    TC_HMI_STPT_TYPE_CTRL_MODE = 0,
    TC_HMI_STPT_TYPE_CLIM_MODE,
    TC_HMI_STPT_TYPE_TEMPERATEURE,
    TC_HMI_STPT_TYPE_BLE_PAIRING
}tskCommon_hmi_stpt_msg_type_e;

typedef struct __attribute__((packed))
{
    tskCommon_hmi_stpt_msg_type_e msg_type;
}tskCommon_hmi_stpt_msg_header_t;

typedef struct __attribute__((packed))
{
    tskCommon_ble_mode_e val;
}tskCommon_hmi_stpt_payload_cftrl_mode_t;

typedef struct __attribute__((packed))
{
    tskCommon_clim_mode_e val;
}tskCommon_hmi_stpt_payload_clim_mode_t;

typedef struct __attribute__((packed))
{
    float val;
}tskCommon_hmi_stpt_payload_temperature_t;

typedef struct __attribute__((packed))
{
    bool val;
}tskCommon_hmi_stpt_payload_pairing_t;
typedef union __attribute__((packed))
{
    tskCommon_hmi_stpt_payload_cftrl_mode_t     ctrl_mode;
    tskCommon_hmi_stpt_payload_clim_mode_t      clim_mode;
    tskCommon_hmi_stpt_payload_temperature_t    temperature;
    tskCommon_hmi_stpt_payload_pairing_t        pairing;
}tskCommon_hmi_stpt_msg_payload_t;

typedef struct __attribute__((packed))
{
    tskCommon_hmi_stpt_msg_header_t     header;
    tskCommon_hmi_stpt_msg_payload_t    payload;
}tskCommon_hmi_stpt_msg_t;

typedef struct __attribute__((packed))
{
    tskCommon_ble_mode_e    ctrl_mode;
    tskCommon_clim_mode_e   clim_mode;
    float                   temperature_stpt;
    bool                    ble_pairing_in_progress;
    uint16_t                ble_pairing_pin_code;
    uint8_t                 ble_nb_device;
    float                   temperature_current;
}tskCommon_system_status_t;

/******************** GLOBAL VARIABLES OF MODULE *****************************/

/******************** API FUNCTION PROTOTYPE *********************************/
void vTskCommon_ErrorLoop(void);


#endif /* INC_TSK_COMMON_H_ */

/**\} */
/**\} */

/* EOF */
