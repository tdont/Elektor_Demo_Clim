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
  * @brief  Capture callback in non blocking mode - level change
  * @param  htim: TIM handle
  * @retval None
  */
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
//{
//  if (RFDemoStatus == RC5_DEC)
//  {
//    /* - Timer Falling Edge Event:
//    The Timer interrupt is used to measure the period between two
//    successive falling edges (The whole pulse duration).
//
//    - Timer Rising Edge Event:
//    It is also used to measure the duration between falling and rising
//    edges (The low pulse duration).
//    The two durations are useful to determine the bit value. Each bit is
//    determined according to the last bit.
//
//    Update event:InfraRed decoders time out event.
//    ---------------------------------------------
//    It resets the InfraRed decoders packet.
//    - The Timer Overflow is set to 3.6 ms .*/
//
//    /* IC2 Interrupt*/
//    if (htim->Channel == IR_TIM_DEC_CH_ACTIV_A)
//    {
//      ICValue2 = HAL_TIM_ReadCapturedValue(&TimHandleDEC, IR_TIM_DEC_CHANNEL_A);
//      /* RC5 */
//      RC5_DataSampling(ICValue2 - ICValue1 , 0);
//
//    }  /* IC1 Interrupt */
//    else if (htim->Channel == IR_TIM_DEC_CH_ACTIV_B)
//    {
//      ICValue1 =  HAL_TIM_ReadCapturedValue(&TimHandleDEC, IR_TIM_DEC_CHANNEL_B);
//      RC5_DataSampling(ICValue1 , 1);
//    }
//  }
//  else if (RFDemoStatus == SIRC_DEC)
//  {
//    /*The Timer interrupt is used to measure the different period between
//    two successive falling edges in order to identify the frame bits.
//
//    We measure the low pulse duration:
//    - If the period measured is equal to T = 1200 micros and the low pulse
//    duration is equal to T/2 = 600 micros => the bit is logic '0'.
//    - If the period measured is equal to 3T/2 = 1800 micros and the low pulse
//    duration is equal to T = 1200micros => the bit is logic '1'.
//    - If the whole period measured is equal to 3000 micros and the low pulse
//    duration is equal to 2400 micros => the bit is �start bit�.
//
//    Update event:InfraRed decoders time out event
//    ----------------------------------------------
//    It resets the InfraRed decoders packet.
//    - The Timer Overflow is set to 4 ms.  */
//
//    /* IC2 Interrupt */
//    if (htim->Channel == IR_TIM_DEC_CH_ACTIV_A)
//    {
//      /* Get the Input Capture value */
//      ICValue2 = HAL_TIM_ReadCapturedValue(&TimHandleDEC , IR_TIM_DEC_CHANNEL_A);
//      SIRC_DataSampling(ICValue1, ICValue2);
//    }  /* IC1 Interrupt*/
//    else  if (htim->Channel == IR_TIM_DEC_CH_ACTIV_B)
//    {
//      /* Get the Input Capture value */
//      ICValue1 = HAL_TIM_ReadCapturedValue(&TimHandleDEC , IR_TIM_DEC_CHANNEL_B);
//    }
//  }
//}

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
//  if (htim == &TimHandleDEC )
//  {
//    if (RFDemoStatus == (uint8_t)SIRC_DEC)
//    {
//      SIRC_ResetPacket();
//    }
//    else if (RFDemoStatus == (uint8_t)RC5_DEC)
//    {
//      RC5_ResetPacket();
//    }
//  }
//  if (htim == &TimHandleLED )
//  {
//    Demo_LedShowCallback();
//  }
}

// /**
//   * @brief  Identify TIM clock
//   * @param  None
//   * @retval Timer clock
//   */
// uint32_t TIM_GetCounterCLKValue(void)
// {
//   uint32_t apbprescaler = 0, apbfrequency = 0;
//   uint32_t timprescaler = 0;

//   /* Get the clock prescaler of APB1 */
//   apbprescaler = ((RCC->CFGR >> 8) & 0x7);
//   apbfrequency = HAL_RCC_GetPCLK1Freq();
//   timprescaler = TIM_PRESCALER;

//   /* If APBx clock div >= 4 */
//   if (apbprescaler >= 4)
//   {
//     return ((apbfrequency * 2) / (timprescaler + 1));
//   }
//   else
//   {
//     return (apbfrequency / (timprescaler + 1));
//   }
// }

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
