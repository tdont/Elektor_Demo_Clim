/**
  ******************************************************************************
  * @file    sirc_encode.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the sirc encode
  *          firmware library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the 
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIRC_ENCODE_H
#define __SIRC_ENCODE_H

#define IRATL_IR_FRAME_FIRST_BYTE_SIZE          (5u)
#define IRATL_IR_FRAME_ONOFF_TOGGLER_ON         (0xFE)
#define IRATL_IR_FRAME_ONOFF_TOGGLER_OFF        (0x02)

#define IRATL_IR_FRAME_OFF_PLD_CHECKSUM         (0xFD)

#define IRATL_IR_FRAME_ON_PLD_B1                (0x09)
#define IRATL_IR_FRAME_ON_PLD_B2                (0x30)
#define IRATL_IR_FRAME_ON_PLD_B6_B7_B8          (0x00)

#define IRATL_IR_FRAME_ON_PLD_HEATMODE_AUTO     (0x00)
#define IRATL_IR_FRAME_ON_PLD_HEATMODE_COOL     (0x01)
#define IRATL_IR_FRAME_ON_PLD_HEATMODE_HEAT     (0x04)

#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_NOSWG    (0x00)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_SWNG_VER (0x10)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_SWNG_HOR (0x20)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_SWNG_DAL (0x30)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_FAN_AUTO (0x00)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_FAN_HIGH (0x01)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_FAN_MIDL (0x02)
#define IRATL_IR_FRAME_ON_PLD_SWNGMODE_FAN_LOW  (0x03)


#define IRATL_IR_FRAME_ON_PLD_ECOMODE_NOM       (0x20)
#define IRATL_IR_FRAME_ON_PLD_ECOMODE_ECO       (0x00)

#define IRATL_IR_FRAME_TEMP_OFFSET_VALUE            (16u)
#define IRATL_IR_FRAME_TEMP_MINIMAL_VALUE_DEG_C     (18u)
#define IRATL_IR_FRAME_TEMP_MAXIMAL_VALUE_DEG_C     (30u)

#define IRATL_IR_FRAME_BIT_0_TRAILING_ZERO_COUNT    (1u)  /* ir pattern like 10 */
#define IRATL_IR_FRAME_BIT_1_TRAILING_ZERO_COUNT    (3u)  /* ir patterne like 1000 */

#define IRATL_IR_FRAME_BIT_CODING_MAX_PWFORM_SIZE   (4u)  /* At maxwimum 4 bits is needed to code a data bit */

#define IRATL_IR_FRAME_START_OF_IR_FRAME_WITH_FIRST_BIT_PULSE   (0x21FF)
#define IRATL_IR_FRAME_START_LENGTH_IN_BIT                      (13u)


typedef struct __attribute__((packed))
{
    uint8_t common[IRATL_IR_FRAME_FIRST_BYTE_SIZE];
    uint8_t on_off_toggler;
}IRATL_IR_header_t;

typedef struct __attribute__((packed))
{
    uint8_t checksum;
}IRATL_IR_pld_OFF_t;

typedef struct __attribute__((packed))
{
    uint8_t b1;
    uint8_t b2;
    uint8_t temperature;
    uint8_t heat_mode;
    uint8_t swing_mode;
    uint8_t b6;
    uint8_t b7;
    uint8_t b8;
    uint8_t eco_mode;
    uint8_t checksum;
}IRATL_IR_pld_ON_t;

typedef union __attribute__((packed))
{
    IRATL_IR_pld_OFF_t   pld_off;
    IRATL_IR_pld_ON_t    pld_on;
}IRATL_IR_payload_t;


typedef struct __attribute__((packed))
{
    IRATL_IR_header_t    header;
    IRATL_IR_payload_t   payload;
}IRATL_IR_frame_t;

typedef union __attribute__((packed))
{
    uint8_t raw_message[sizeof(IRATL_IR_frame_t)];
    IRATL_IR_frame_t frame;
}IRATL_IR_frame_helper_t;



#define IRATL_PWFORM_MAX_SIZE_U8 ((sizeof(IRATL_IR_frame_t) * 8 * IRATL_IR_FRAME_BIT_CODING_MAX_PWFORM_SIZE \
                                     + IRATL_IR_FRAME_START_LENGTH_IN_BIT                                   \
                                     + ((IRATL_IR_FRAME_START_LENGTH_IN_BIT % 8) ? 1 : 0)) / 8)

#define IRATL_PWFORM_MAX_SIZE_U32 ((IRATL_PWFORM_MAX_SIZE_U8 / sizeof(uint32_t)) + 1)



/** @addtogroup IR_REMOTE
  * @{
  */

/** @addtogroup SIRC_ENCODE
  * @{
  */


/** @defgroup SIRC_Public_Constants
  * @{
  */
extern const uint8_t* aSIRCDevices[];
extern const uint8_t* aSIRCCommands[];

/**
  * @}
  */

/** @addtogroup SIRC_Exported_Functions
  * @{
  */
void SIRC_Encode_Init(void);
// void SIRC_Encode_SendFrame(uint8_t SIRC_Address, uint8_t SIRC_Instruction);

void IRATL_transmit_frame(uint8_t* raw_message, uint8_t nb_byte);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif  /*__SIRC_ENCODE_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
