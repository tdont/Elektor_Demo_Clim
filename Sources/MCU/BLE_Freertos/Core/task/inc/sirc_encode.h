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
void SIRC_Encode_SendFrame(uint8_t SIRC_Address, uint8_t SIRC_Instruction);

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
