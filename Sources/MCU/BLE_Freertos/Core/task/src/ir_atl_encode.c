/**
  ******************************************************************************
  * @file    sirc_encode.c
  * @author  MCD Application Team
  * @brief   This file provides all the sirc encode firmware functions
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
#include "ir_atl_encode.h"
#include "ir_common.h"

#include <stdint.h>
#include <stm32wb5mm_dk.h>

/* Private_Defines -----------------------------------------------------------*/
#define  SIRC_HIGH_STATE_NB_SYMBOL     ((uint8_t)3)        /* Nb high state symbol definition*/
#define  SIRC_LOW_STATE_NB_SYMBOL      ((uint8_t)2)        /* Nb low state symbol definition*/
#define  SIRC_ADDRESS_BITS             ((uint8_t)5)        /* Nb of data bits definition*/
#define  SIRC_INSTRUCTION_BITS         ((uint8_t)7)        /* Nb of data bits definition*/
#define  SIRC_HIGH_STATE_CODE          ((uint8_t)0x03)     /* SIRC high level definition*/
#define  SIRC_LOW_STATE_CODE           ((uint8_t)0x01)     /* SIRC low level definition*/
#define  SIRC_MAIN_FRAME_LENGTH        ((uint8_t)12)       /* Main frame length*/
#define  SIRC_BIT_FORMAT_MASK          ((uint16_t)1)       /* Bit mask definition*/
#define  SIRC_COUNTER_LIMIT            ((uint16_t)75)      /* main frame + idle time */
#define  SIRC_IS_RUNNING               ((uint8_t)4)        /* SIRC Protocol number */
#define  SIRC_HEADERS                  ((uint16_t)0xFF)    /* SIRC Start pulse */
#define  SIRC_HEADERS_LENGTH           ((uint8_t)5)        /* Length of the headers */
#define  SIRC_CODED_FRAME_TABLE_LENGTH ((uint8_t)50)        /* Coded frame table number of uint32_t word  */

#define IR_ENC_HPERIOD_SIRC     ((uint32_t)850)        /*!< SIRC Encoder modulation frequency base period 38Khz */
#define IR_ENC_LPERIOD_SIRC     ((uint32_t)13200)       /*!< SIRC Encoder pulse base period 400 µs*/

/* Private_Function_Protoypes -----------------------------------------------*/
//static void SIRC_Encode_DeInit(void);

/* Public_Variables ----------------------------------------------------------*/
volatile uint32_t aSIRCFramePWForm[IRATL_PWFORM_MAX_SIZE_U32] = {0};
static uint16_t PW_Form_head_position = IRATL_IR_FRAME_START_LENGTH_IN_BIT;

TIM_HandleTypeDef    TimHandleHF;
TIM_HandleTypeDef    TimHandleLF;
// static TIM_HandleTypeDef    TimHandleLED;
// static TIM_HandleTypeDef    TimHandleDEC;


/* Private_Variables ---------------------------------------------------------*/
__IO uint16_t SIRCFrameBinaryFormat = 0;
uint8_t SIRCSendOpReadyFlag = RESET;
uint8_t SIRCSendOpCompleteFlag = SET;
uint16_t SIRCBitsSentCounter = 0;
uint16_t SIRCCodedFrameLastWordLength = 0;
uint8_t SIRCNbWord = 0;


/* Exported_Functions --------------------------------------------------------*/
/**
  * @brief DeInit Hardware (IPs used) for SIRC generation
  * @param None
  * @retval None
  */
//static void SIRC_Encode_DeInit(void)
//{
//  HAL_TIM_OC_DeInit(&TimHandleLF);
//  HAL_TIM_OC_DeInit(&TimHandleHF);
//  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
//
//  RFDemoStatus = NONE;
//}

/**
  * @brief Init Hardware (IPs used) for SIRC generation
  * @param None
  * @retval None
  */
void SIRC_Encode_Init(void)
{
  TIM_OC_InitTypeDef ch_config;

  RFDemoStatus = SIRC_ENC;

  /* TIM16 clock enable */
  __HAL_RCC_TIM16_CLK_ENABLE();

  /* TIM17 clock enable */
  __HAL_RCC_TIM17_CLK_ENABLE();

  TimHandleLF.Instance = TIM16;
  TimHandleHF.Instance = TIM17;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* DeInit TIM17 */
  HAL_TIM_OC_DeInit(&TimHandleHF);

  /* Elementary period 888us */
  /* Time base configuration for timer 2 */
  TimHandleHF.Init.Period = IR_ENC_HPERIOD_SIRC;
  TimHandleHF.Init.Prescaler = 0x00;
  TimHandleHF.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TimHandleHF.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_Base_Init(&TimHandleHF);
  if (HAL_TIM_OC_Init(&TimHandleHF) != HAL_OK)
  {
    /* Initialization Error */
    while (1)
    {}
  }

  /* Output Compare Timing Mode configuration: Channel 1N */
  ch_config.OCMode = TIM_OCMODE_PWM1;
  ch_config.Pulse = IR_ENC_HPERIOD_SIRC / 2; /*Set duty cycle to 50%  */
  ch_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  ch_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  ch_config.OCFastMode = TIM_OCFAST_ENABLE;
  ch_config.OCIdleState = TIM_OCIDLESTATE_RESET;
  ch_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&TimHandleHF, &ch_config, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Configuration Error */
    while (1)
    {}
  }

  /* Setting also the opposing polarity */
  TIM_SET_CAPTUREPOLARITY(&TimHandleHF, TIM_CHANNEL_1, TIM_CCxN_ENABLE | TIM_CCx_ENABLE );

  /* Start the modulation frequency */
  HAL_TIM_OC_Start(&TimHandleHF, TIM_CHANNEL_1);

  /* The TIM16 section */
  HAL_TIM_OC_DeInit(&TimHandleLF);

  /* Time Base = 1,6666 Khz(600us) */
  /* Time Base configuration for timer 16 */
  TimHandleLF.Init.Prescaler = 0;
  TimHandleLF.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandleLF.Init.Period = IR_ENC_LPERIOD_SIRC;
  TimHandleLF.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TimHandleLF.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&TimHandleLF);
  if (HAL_TIM_OC_Init(&TimHandleLF) != HAL_OK)
  {
    /* Initialization Error */
    while (1)
    {}
  }

  /* Channel 1 Configuration in Timing mode */
  ch_config.OCMode = TIM_OCMODE_TIMING;
  ch_config.Pulse = IR_ENC_LPERIOD_SIRC;
  ch_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  ch_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  ch_config.OCFastMode = TIM_OCFAST_ENABLE;
  ch_config.OCIdleState = TIM_OCIDLESTATE_RESET;
  ch_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&TimHandleLF, &ch_config, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Configuration Error */
    while (1)
    {}
  }

  /* Setting also the opposing polarity */
  TIM_SET_CAPTUREPOLARITY(&TimHandleLF, TIM_CHANNEL_1, TIM_CCxN_ENABLE | TIM_CCx_ENABLE );

  /* Enable the TIM16 Interrupt */
  HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

  /* TIM16 Main Output Enable */
  __HAL_TIM_MOE_ENABLE(&TimHandleLF);

  /* TIM Disable */
  __HAL_TIM_DISABLE(&TimHandleLF);
}

// /**
//   * @brief Generate and Send the SIRC frame.
//   * @param SIRC_Address : the SIRC Device destination
//   * @param SIRC_Instruction : the SIRC command instruction
//   * @retval  None
//   */
// void SIRC_Encode_SendFrame(uint8_t SIRC_Address, uint8_t SIRC_Instruction)
// {
//   /* Check the parameters */
//   assert_param(IS_SIRC_ADDRESS_IN_RANGE(SIRC_Address));
//   assert_param(IS_SIRC_INSTRUCTION_IN_RANGE(SIRC_Instruction));

//   /* Power ON LED */
//   HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);

//   /* Generate a binary format of the message */
//   SIRCFrameBinaryFormat = SIRC_BinFrameGeneration(SIRC_Address, SIRC_Instruction);

//   /* Transform address and data from MSB first to LSB first */
//   SIRCFrameBinaryFormat = SIRC_MSBToLSB_Data(SIRCFrameBinaryFormat, SIRC_MAIN_FRAME_LENGTH);

//   /* Convert the frame binary format to a PulseWidthModulation format of the message */
//   SIRC_PulseWidthModulationConvert(SIRCFrameBinaryFormat, SIRC_MAIN_FRAME_LENGTH);

//   /* Add the headers to SIRC_FramePulseWidthFormat Table */
//   SIRC_AddHeaders(SIRC_HEADERS);

//   /* Set the Send operation Ready flag to indicate that the frame is ready to be sent */
//   SIRCSendOpReadyFlag = SET;

//   /* Reset the counter to ensure accurate timing */
//   __HAL_TIM_SET_COUNTER( &TimHandleLF, 0);

//   /* TIM IT Enable */
//   HAL_TIM_Base_Start_IT(&TimHandleLF);
// }

/**
  * @brief Send by hardware PulseWidthModulation Format SIRC Frame.
  * @retval none
  */
void SIRC_Encode_SignalGenerate(void)
{
  uint32_t tablecounter = 0;
  uint32_t bitmsg = 0;

  if ((SIRCSendOpReadyFlag != RESET) && (SIRCBitsSentCounter <= ( PW_Form_head_position )))
  {
    /*Reset send operation complete flag*/
    SIRCSendOpCompleteFlag = RESET;

    /*Read message bits*/
    uint8_t index = IRATL_PWFORM_MAX_SIZE_U32 - 1 - (SIRCBitsSentCounter / 32);
    uint8_t bit_pos = SIRCBitsSentCounter % 32;

    uint32_t mask = 0b1 << bit_pos;

    bitmsg = (aSIRCFramePWForm[index] & mask);

    /* Generate signal */
    if (bitmsg != RESET)
    {
      TIM_ForcedOC1Config(TIM_FORCED_ACTIVE);
    }
    else if (bitmsg == RESET)
    {
      TIM_ForcedOC1Config(TIM_FORCED_INACTIVE);
    }
    // else if (SIRCBitsSentCounter <= ( SIRCNbWord*32 + SIRCCodedFrameLastWordLength ))
    // {
    //   TIM_ForcedOC1Config(TIM_FORCED_INACTIVE);
    // }

    SIRCBitsSentCounter++;
  }
  else /* Sending complete */
  {
    /* Reset flags   */
    SIRCSendOpCompleteFlag = SET;
    SIRCSendOpReadyFlag = RESET;
    /* TIM IT Disable */
    HAL_TIM_Base_Stop_IT(&TimHandleLF);
    /*Reset counters */
    SIRCNbWord = 0;
    SIRCBitsSentCounter = 0;
    /*Reset counters */
    SIRCCodedFrameLastWordLength = 0;
    SIRCFrameBinaryFormat = 0;

    /*Reset frame temporary variables*/
    for (tablecounter = 0; tablecounter < SIRCNbWord; tablecounter++)
    {
      aSIRCFramePWForm[tablecounter] = 0x0;
    }
    TIM_ForcedOC1Config(TIM_FORCED_INACTIVE);

    /* TIM Disable */
    __HAL_TIM_DISABLE(&TimHandleLF);

    /* Power OFF LED */
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);
  }
}

/* Private_Functions ---------------------------------------------------------*/
void IRATL_PWForm_position_pulse()
{
     /* Make the assumption all bit were set to 0 prior, only seting bit to 1 */
    /* Find byte position */
    uint8_t index = IRATL_PWFORM_MAX_SIZE_U32 - 1 - (PW_Form_head_position / 32);
    uint8_t bit_pos = PW_Form_head_position % 32;

    uint32_t mask = 0b1 << bit_pos;

    /* Set bit to one */
    aSIRCFramePWForm[index] = (aSIRCFramePWForm[index] | mask);
}

void IRATL_PWForm_append_bit_zero()
{
    /* Make the assumption all bit were set to 0 prior, only seting bit to 1 */
    IRATL_PWForm_position_pulse();
    /* Increment head position */
    PW_Form_head_position +=  IRATL_IR_FRAME_BIT_0_TRAILING_ZERO_COUNT + 1;

    SIRCCodedFrameLastWordLength = PW_Form_head_position;
}

void IRATL_PwForm_append_bit_one()
{
    /* Make the assumption all bit were set to 0 prior, only seting bit to 1 */
    IRATL_PWForm_position_pulse();
    /* Increment head position */
    PW_Form_head_position +=  IRATL_IR_FRAME_BIT_1_TRAILING_ZERO_COUNT + 1;

    SIRCCodedFrameLastWordLength = PW_Form_head_position;
}

void IRATL_transmit_frame(uint8_t* raw_message, uint8_t nb_byte)
{
    if(SIRCSendOpCompleteFlag == RESET)
    {
      /* Avoid simultaneous transmission */
      return;
    }

    
    /* Power ON LED */
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);

    /* Reset PulseWidth Form */
    memset(aSIRCFramePWForm, 0, sizeof(aSIRCFramePWForm));

    /* Append start frame at the end of the pwForm */
    aSIRCFramePWForm[sizeof(aSIRCFramePWForm)/4 - 1] = (IRATL_IR_FRAME_START_OF_IR_FRAME_WITH_FIRST_BIT_PULSE);    
    PW_Form_head_position = IRATL_IR_FRAME_START_LENGTH_IN_BIT;

    for (uint8_t current_byte = 0; current_byte < nb_byte; current_byte++)
    {
        uint8_t temp_byte = raw_message[current_byte];
        for (uint8_t current_bit = 0; current_bit < 8; current_bit++)
        {
            if(temp_byte & 0b1)
            {
                //printf("1");
                IRATL_PwForm_append_bit_one();
            }
            else
            {
                //printf("0");
                IRATL_PWForm_append_bit_zero();
            }

            temp_byte = temp_byte >> 1;
        }
        //printf("_|_");
    }

    /* Add a final pulse */
    IRATL_PWForm_position_pulse();

    /* Start transmission */
    /* Set the Send operation Ready flag to indicate that the frame is ready to be sent */
    SIRCSendOpReadyFlag = SET;

    /* Reset the counter to ensure accurate timing */
    __HAL_TIM_SET_COUNTER( &TimHandleLF, 0);

    /* TIM IT Enable */
    HAL_TIM_Base_Start_IT(&TimHandleLF);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
