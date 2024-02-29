/**
  ******************************************************************************
  * @file    ir_common.c
  * @author  MCD Application Team
  * @brief   This file provides the shared sirc/rc5 firmware functions
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ir_common.h"

/* Common_Private_Defines ----------------------------------------------------*/
#define  RC5HIGHSTATE     ((uint8_t )0x02)   /* RC5 high level definition*/
#define  RC5LOWSTATE      ((uint8_t )0x01)   /* RC5 low level definition*/

/* Common_Private_Variables --------------------------------------------------*/
uint32_t ICValue1 = 0;
uint32_t ICValue2 = 0;

/* Common_Public_Variables ---------------------------------------------------*/
uint8_t BitsSentCounter = 0;
uint8_t AddressIndex = 0;
uint8_t InstructionIndex = 0;
__IO StatusOperation_t RFDemoStatus;

/* Common_Exported_Functions -------------------------------------------------*/

/**
  * @brief  Period elapsed callback in non blocking mode - timeout
  * @param  htim: TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback_IR(TIM_HandleTypeDef *htim)
{
  /* Depending */
  if (htim == &TimHandleLF )
  {
    if (RFDemoStatus == (uint8_t)SIRC_ENC)
    {
      SIRC_Encode_SignalGenerate();
    }
    else if (RFDemoStatus == (uint8_t)RC5_ENC)
    {
//      RC5_Encode_SignalGenerate();
    }
  }

}

/**
  * @brief  Force new configuration to the output channel
  * @param  action: new configuration
  * @retval None
  */
void TIM_ForcedOC1Config(uint32_t action)
{
  uint32_t temporary = TimHandleLF.Instance->CCMR1;

  temporary &= ~TIM_CCMR1_OC1M;
  temporary |= action;
  TimHandleLF.Instance->CCMR1 = temporary;
}
