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
#include "sirc_encode.h"
#include "ir_common.h"

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
#define  SIRC_HEADERS                  ((uint16_t)0x0F)    /* SIRC Start pulse */
#define  SIRC_HEADERS_LENGTH           ((uint8_t)5)        /* Length of the headers */
#define  SIRC_CODED_FRAME_TABLE_LENGTH ((uint8_t)2)        /* Coded frame table number of uint32_t word  */

#define IR_ENC_HPERIOD_SIRC     ((uint32_t)1600)        /*!< SIRC Encoder modulation frequency base period */
#define IR_ENC_LPERIOD_SIRC     ((uint32_t)38400)       /*!< SIRC Encoder pulse base period */

/* Private_Function_Protoypes -----------------------------------------------*/
static void SIRC_PulseWidthModulationConvert(uint32_t bindata, uint8_t bindatalength);
static void SIRC_Shift_Table(__IO uint32_t aTable[]);
static void SIRC_AddHeaders(uint8_t headers);
static void SIRC_AddStateFragment(uint8_t State, uint8_t freespace);
static void SIRC_AddHeadersFragment(uint8_t headers, uint8_t freespace);
static uint32_t SIRC_MSBToLSB_Data(uint32_t Data, uint8_t DataNbBits);
static uint16_t SIRC_BinFrameGeneration(uint8_t SIRC_Address, uint8_t SIRC_Instruction);
static void SIRC_Encode_DeInit(void);

/* Public_Variables ----------------------------------------------------------*/
__IO uint32_t aSIRCFramePWForm[SIRC_CODED_FRAME_TABLE_LENGTH];

TIM_HandleTypeDef    TimHandleHF;
TIM_HandleTypeDef    TimHandleLF;
// static TIM_HandleTypeDef    TimHandleLED;
// static TIM_HandleTypeDef    TimHandleDEC;


/* Private_Variables ---------------------------------------------------------*/
__IO uint16_t SIRCFrameBinaryFormat = 0;
uint8_t SIRCSendOpReadyFlag = RESET;
uint8_t SIRCSendOpCompleteFlag = SET;
uint8_t SIRCBitsSentCounter = 0;
uint8_t SIRCCodedFrameLastWordLength = 0;
uint8_t SIRCNbWord = 0;

/* Exported_Functions --------------------------------------------------------*/

// /**
//   * @brief  RCR receiver demo exec.
//   * @param  None
//   * @retval None
//   */
// void Menu_SIRC_Encode_Func(void)
// {
//   MenuKeyStroke_t pressedkey = NOKEY;
//   uint8_t index = 0;

//   while (Menu_ReadKey() != NOKEY)
//   {}
//   /* Clear the LCD */
//   BSP_LCD_Clear(LCD_COLOR_WHITE);

//   /* Draw Header */
//   BSP_LCD_SetFont(&Font24);
//   /* Set the LCD Back Color */
//   BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
//   /* Set the LCD Text Color */
//   BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
//   BSP_LCD_DisplayStringAtLine(0, (uint8_t*)" SIRC Transmitter ");

//   /* Draw footer */
//   BSP_LCD_SetFont(&Font16);
//   /* Set the LCD Back Color */
//   BSP_LCD_SetBackColor(LCD_COLOR_CYAN);
//   /* Set the LCD Text Color */
//   BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
//   BSP_LCD_DisplayStringAtLine(13, (uint8_t *)MESSAGE1);
//   /* Set the LCD Back Color */
//   BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
//   BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
//   BSP_LCD_DisplayStringAtLine(14, (uint8_t *)MESSAGE2);
//   BSP_LCD_SetFont(&Font20);

//   SIRC_Encode_Init();

//   AddressIndex = 0;
//   InstructionIndex = 0;
//   RFDemoStatus = SIRC_ENC;

//   pressedkey = Menu_ReadKey();

//   /* Set the LCD Back Color */
//   BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
//   /* Set the LCD Text Color */
//   BSP_LCD_SetTextColor(LCD_COLOR_RED);
//   /* Display the device address message */
//   BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);
//   /* Set the LCD Text Color */
//   BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
//   /* Display the device address message */
//   BSP_LCD_DisplayStringAtLine(7, (uint8_t*)aSIRCDevices[AddressIndex]);

//   /* Set the LCD Text Color */
//   BSP_LCD_SetTextColor(LCD_COLOR_RED);

//   while (pressedkey != KEY)
//   {
//     pressedkey = Menu_ReadKey();
//     /* To switch between device address and command */
//     if ((pressedkey == DOWN) || (pressedkey == UP))
//     {
//       /* Set the LCD Text Color */
//       BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

//       if (index == 0)
//       {
//         index = 1;
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(7, (uint8_t*)aSIRCDevices[AddressIndex]);
//       }
//       else
//       {
//         index = 0;
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(7, (uint8_t*)aSIRCDevices[AddressIndex]);
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);
//       }
//     }
//     if (index == 0)
//     {
//       /* Commands index decrement */
//       if (pressedkey == LEFT)
//       {
//         if (InstructionIndex == 0)
//         {
//           InstructionIndex = 127;
//         }
//         else
//         {
//           InstructionIndex--;
//         }
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);
//       }
//       /* Commands index increment */
//       if (pressedkey == RIGHT)
//       {
//         if (InstructionIndex == 127)
//         {
//           InstructionIndex = 0;
//         }
//         else
//         {
//           InstructionIndex++;
//         }
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);
//       }
//     }
//     else
//     {
//       /* Decrement the address device index */
//       if (pressedkey == LEFT)
//       {
//         if (AddressIndex == 0)
//         {
//           AddressIndex = 15;
//         }
//         else
//         {
//           AddressIndex--;
//         }
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         /* Display the device address message */
//         BSP_LCD_DisplayStringAtLine(7, (uint8_t*)aSIRCDevices[AddressIndex]);
//       }
//       /* Increment the address device index increment */
//       if (pressedkey == RIGHT)
//       {
//         if (AddressIndex == 15)
//         {
//           AddressIndex = 0;
//         }
//         else
//         {
//           AddressIndex++;
//         }
//         /* Set the LCD Text Color */
//         BSP_LCD_SetTextColor(LCD_COLOR_RED);
//         BSP_LCD_DisplayStringAtLine(7, (uint8_t*)aSIRCDevices[AddressIndex]);
//       }
//     }
//     if (pressedkey == SEL)
//     {
//       BSP_LCD_ClearStringLine(6);

//       /* Button is pressed */
//       SIRC_Encode_SendFrame(AddressIndex, InstructionIndex);

//       BSP_LCD_SetTextColor(LCD_COLOR_RED);
//       BSP_LCD_DisplayStringAtLine(6, (uint8_t*)aSIRCCommands[InstructionIndex]);

//       pressedkey = NOKEY;
//     }
//   }
//   SIRC_Encode_DeInit();

//   BSP_LCD_Clear(LCD_COLOR_WHITE);
//   BSP_LCD_SetFont(&Font24);

//   /* Enable the JoyStick interrupt */
//   HAL_NVIC_EnableIRQ(LEFT_JOY_EXTI_IRQn);

//   /* Exit the SIRC demo */
//   RFDemoStatus = NONE;

//   /* Display menu */
//   Menu_DisplayMenu();
// }

/**
  * @brief DeInit Hardware (IPs used) for SIRC generation
  * @param None
  * @retval None
  */
static void SIRC_Encode_DeInit(void)
{
  HAL_TIM_OC_DeInit(&TimHandleLF);
  HAL_TIM_OC_DeInit(&TimHandleHF);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

  RFDemoStatus = NONE;
}

/**
  * @brief Init Hardware (IPs used) for SIRC generation
  * @param None
  * @retval None
  */
void SIRC_Encode_Init(void)
{
  TIM_OC_InitTypeDef ch_config;
  GPIO_InitTypeDef gpio_init_struct;

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

  /*Configure GPIO pin : PB9 already performed by IOC generation */
//  gpio_init_struct.Pin = GPIO_PIN_9;
//  gpio_init_struct.Mode = GPIO_MODE_AF_PP;
//  gpio_init_struct.Pull = GPIO_NOPULL;
//  gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
//  gpio_init_struct.Alternate = IR_GPIO_AF_TR;
//  HAL_GPIO_Init(GPIOB, &gpio_init_struct);

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

/**
  * @brief Generate and Send the SIRC frame.
  * @param SIRC_Address : the SIRC Device destination
  * @param SIRC_Instruction : the SIRC command instruction
  * @retval  None
  */
void SIRC_Encode_SendFrame(uint8_t SIRC_Address, uint8_t SIRC_Instruction)
{
  /* Check the parameters */
  assert_param(IS_SIRC_ADDRESS_IN_RANGE(SIRC_Address));
  assert_param(IS_SIRC_INSTRUCTION_IN_RANGE(SIRC_Instruction));

  /* Power ON LED */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);

  /* Generate a binary format of the message */
  SIRCFrameBinaryFormat = SIRC_BinFrameGeneration(SIRC_Address, SIRC_Instruction);

  /* Transform address and data from MSB first to LSB first */
  SIRCFrameBinaryFormat = SIRC_MSBToLSB_Data(SIRCFrameBinaryFormat, SIRC_MAIN_FRAME_LENGTH);

  /* Convert the frame binary format to a PulseWidthModulation format of the message */
  SIRC_PulseWidthModulationConvert(SIRCFrameBinaryFormat, SIRC_MAIN_FRAME_LENGTH);

  /* Add the headers to SIRC_FramePulseWidthFormat Table */
  SIRC_AddHeaders(SIRC_HEADERS);

  /* Set the Send operation Ready flag to indicate that the frame is ready to be sent */
  SIRCSendOpReadyFlag = SET;

  /* Reset the counter to ensure accurate timing */
  __HAL_TIM_SET_COUNTER( &TimHandleLF, 0);

  /* TIM IT Enable */
  HAL_TIM_Base_Start_IT(&TimHandleLF);
}

/**
  * @brief Send by hardware PulseWidthModulation Format SIRC Frame.
  * @retval none
  */
void SIRC_Encode_SignalGenerate(void)
{
  uint32_t tablecounter = 0;
  uint32_t bitmsg = 0;

  if ((SIRCSendOpReadyFlag != RESET) && (SIRCBitsSentCounter <= ( SIRC_COUNTER_LIMIT )))
  {
    /*Reset send operation complete flag*/
    SIRCSendOpCompleteFlag = RESET;

    /*Read message bits*/
    if (SIRCBitsSentCounter < SIRCCodedFrameLastWordLength)
    {
      /*Read coded frame bits from the last word*/
      bitmsg = (uint8_t)((aSIRCFramePWForm[0] >> SIRCBitsSentCounter) & 1);
    }
    else
    {
      /*Read coded frame bits from the table*/
      bitmsg = (uint8_t)((aSIRCFramePWForm[((SIRCBitsSentCounter-SIRCCodedFrameLastWordLength)/32)+1] >> (SIRCBitsSentCounter - SIRCCodedFrameLastWordLength)) & 1);
    }

    /* Generate signal */
    if (bitmsg != RESET)
    {
      TIM_ForcedOC1Config(TIM_FORCED_ACTIVE);
    }
    else if (bitmsg == RESET)
    {
      TIM_ForcedOC1Config(TIM_FORCED_INACTIVE);
    }
    else if (SIRCBitsSentCounter <= ( SIRCNbWord*32 + SIRCCodedFrameLastWordLength ))
    {
      TIM_ForcedOC1Config(TIM_FORCED_INACTIVE);
    }

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

/**
  * @ Transform the frame binary form from MSB to LSB.
  * @param Data: Frame binary format to inverse
  * @param DataNbBits: size of the transmission
  * This parameter can be any of the @ref uint32_t.
  * @retval Symmetric binary frame form
  */
static uint32_t SIRC_MSBToLSB_Data(uint32_t Data , uint8_t DataNbBits)
{
  uint32_t temp = 0;     /* Temporary variable to memorize the converted message */
  uint8_t datacount = 0; /* Counter of bits converted */

  /* Shift the temporary variable to the left and add one bit from the Binary frame  */
  for (datacount = 0; datacount < (DataNbBits); datacount++)
  {
    temp = temp << 1;
    temp |= ((Data >> datacount) & 1);
  }
  return temp;
}

/**
  * @brief Generate the binary format of the SIRC frame.
  * @param SIRC_Address : Select the device address
  * @param SIRC_Instruction : Select the device instruction
  * @retval Binary format of the SIRC Frame.
  */
static uint16_t SIRC_BinFrameGeneration(uint8_t SIRC_Address, uint8_t SIRC_Instruction)
{
  /* wait until the ongoing Frame sending operation finishes */
  while (SIRCSendOpCompleteFlag == RESET)
  {}

  /* Reset SIRC_Send_Operation_Ready_f flag to mention that a send operation can be treated */
  SIRCSendOpReadyFlag = RESET;

  /* Concatenate Binary Frame Format */
  SIRCFrameBinaryFormat = (SIRC_Address << SIRC_INSTRUCTION_BITS );
  SIRCFrameBinaryFormat = SIRCFrameBinaryFormat | (SIRC_Instruction);

  return (SIRCFrameBinaryFormat);
}

/**
  * @Shift the coded frame table by one box.
  * @param aTable: coded data table
  * This parameter can be any of the @ref uint32_t.
  * @retval  None
  */
static void SIRC_Shift_Table(__IO uint32_t aTable[])
{
  uint8_t i = 0;

  /* Increment the coded frame table words number */
  SIRCNbWord++;

  /* Shift the coded frame table to the left by one box */
  for (i = 0; i < SIRCNbWord; i++)
  {
    aTable[SIRCNbWord-i] = aTable[SIRCNbWord - i - 1];
  }
  /* Clear the first the coded frame table box */
  aTable[0] = 0;

  /* Reset the last word length counter */
  SIRCCodedFrameLastWordLength = 0;
}

/**
  * @brief split state codes in to two word of the table .
  * @param state: the coded state to add to the coded frame table
  * @param freespace: the last coded frame table word free space
  * @retval  None
  */
static void SIRC_AddStateFragment(uint8_t state, uint8_t freespace)
{
  /* Shift the table to the left by one box */
  SIRC_Shift_Table(aSIRCFramePWForm);

  /*Test if the message to add is a high state code */
  if (state == SIRC_HIGH_STATE_CODE)
  {
    /* The message is a high state code */
    /* Add the first frame fragment to the First word of the table*/
    aSIRCFramePWForm[SIRCNbWord] = aSIRCFramePWForm[SIRCNbWord] << freespace ;
    aSIRCFramePWForm[SIRCNbWord] |= SIRC_HIGH_STATE_CODE >> (SIRC_HIGH_STATE_NB_SYMBOL - freespace);

    /* Add the Second frame fragment to the Second word of the table*/
    aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << (SIRC_HIGH_STATE_NB_SYMBOL - freespace) ;
    aSIRCFramePWForm[0] |= ((SIRC_HIGH_STATE_CODE << (8 - SIRC_HIGH_STATE_NB_SYMBOL + freespace)) & 0xff) >> (8 - SIRC_HIGH_STATE_NB_SYMBOL + freespace);

    /* Increment the Last word of the coded frame counter*/
    SIRCCodedFrameLastWordLength = SIRC_HIGH_STATE_NB_SYMBOL - freespace ;
  }
  else
  {
    /* The message is a low state code */
    /* Add the first frame fragment to the First word of the table */
    aSIRCFramePWForm[SIRCNbWord] = aSIRCFramePWForm[SIRCNbWord] << freespace ;
    aSIRCFramePWForm[SIRCNbWord] |= SIRC_LOW_STATE_CODE >> (SIRC_LOW_STATE_NB_SYMBOL - freespace);

    /* Add the Second frame fragment to the Second word of the table */
    aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << (SIRC_LOW_STATE_NB_SYMBOL - freespace) ;
    aSIRCFramePWForm[0] |= ((SIRC_LOW_STATE_CODE << (8 - SIRC_LOW_STATE_NB_SYMBOL + freespace)) & 0xff) >> (8 - SIRC_LOW_STATE_NB_SYMBOL + freespace);

    /* Increment the Last word of the coded frame counter */
    SIRCCodedFrameLastWordLength = SIRC_LOW_STATE_NB_SYMBOL - freespace ;
  }
}

/**
  * @brief Convert the SIRC message from binary to Pulse width modulation Format.
  *        Output is put into global variable.
  * @param binData: The SIRC message in binary format.
  * @param binDataLength: Number of bits
  * @retval None
  */
void SIRC_PulseWidthModulationConvert(uint32_t binData, uint8_t binDataLength)
{
  uint32_t dataframecount = 0;
  uint32_t bitformat = 0;
  uint32_t lastwordfreeespace = 0;

  for (dataframecount = 0; dataframecount < binDataLength; dataframecount++)
  {
    /* Calculate last coded frame word free space */
    lastwordfreeespace = 32 - SIRCCodedFrameLastWordLength;
    /* Select one bit from the binary frame */
    bitformat = (uint32_t)((uint32_t)((((uint16_t)(binData)) >> dataframecount) & SIRC_BIT_FORMAT_MASK) << dataframecount);

    /* Test the bit format state */
    if (bitformat != 0)
    {
      /* Test if the last word of the frame enough space */
      if ((lastwordfreeespace) > (SIRC_HIGH_STATE_NB_SYMBOL - 1))
      {
        /*Shift left the the last coded frame word by State number of bits */
        aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << SIRC_HIGH_STATE_NB_SYMBOL ;

        /* Add the state to the last word of the coded frame table */
        aSIRCFramePWForm[0] |= SIRC_HIGH_STATE_CODE;

        /*Increment the last coded frame word counter */
        SIRCCodedFrameLastWordLength = SIRCCodedFrameLastWordLength + SIRC_HIGH_STATE_NB_SYMBOL ;
      }
      else
      {
        /* Split state code to two words */
        SIRC_AddStateFragment(SIRC_HIGH_STATE_CODE, lastwordfreeespace);
      }
    }
    else
    {
      /*  bit format == 0 */
      /* Test if the last word of the frame enough space */
      if ((lastwordfreeespace) > 1)
      {
        /* enough space found */
        /* Shift left the the last coded frame word by State bits number */
        aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << SIRC_LOW_STATE_NB_SYMBOL ;

        /* Shift left the the last coded frame word by State bits number */
        aSIRCFramePWForm[0] |= SIRC_LOW_STATE_CODE;

        /* Increment the Last word of the coded frame counter */
        SIRCCodedFrameLastWordLength = SIRCCodedFrameLastWordLength + SIRC_LOW_STATE_NB_SYMBOL ;
      }
      else
      {
        /* Split state code to two words */
        SIRC_AddStateFragment(SIRC_LOW_STATE_CODE, lastwordfreeespace);
      }
    }
  }
}

/**
  * @brief Split coded headers in to two word of the table.
  * @param headers: the coded headers to add to the coded frame table
  * @param freespace: the last coded frame table word free space
  * This parameter can be any of the @ref uint8_t.
  * @retval  None
  */
static void SIRC_AddHeadersFragment (uint8_t headers, uint8_t freespace)
{
  /* Shift the table to the left by one box */
  SIRC_Shift_Table(aSIRCFramePWForm);

  /* Shift left the the second coded frame word by headers second fragment bits number */
  aSIRCFramePWForm[SIRCNbWord] = aSIRCFramePWForm[SIRCNbWord] << freespace;

  /* Add the first headers fragment to the Second word of the table */
  aSIRCFramePWForm[SIRCNbWord] |= SIRC_HEADERS >> (SIRC_HEADERS_LENGTH - freespace);

  /* Shift left the the last coded frame word by headers bits number */
  aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << (SIRC_HEADERS_LENGTH - freespace);

  /* Add the Second Headers fragment to the Second word of the table*/
  aSIRCFramePWForm[0] |= (((( SIRC_HEADERS) << (freespace + 3))) & 0xFF) >> ((freespace + 3));

  /* Increment the Last word of the coded frame counter */
  SIRCCodedFrameLastWordLength = SIRC_HEADERS_LENGTH - freespace;
}

/**
  * @brief Add coded headers to the coded frame table.
  * @param headers: This parameter can be any of the @ref uint8_t.
  * @retval None
  */
static void SIRC_AddHeaders(uint8_t headers)
{
  uint8_t lastwordfreespace = 0;

  /* Calculate last coded frame word free space */
  lastwordfreespace = 32 - SIRCCodedFrameLastWordLength;

  if (lastwordfreespace > 4)
  {
    /* Shift left the the last coded frame word by headers bits number */
    aSIRCFramePWForm[0] = aSIRCFramePWForm[0] << 5;

    /* Add the first headers fragment to the First word of the table */
    aSIRCFramePWForm[0] = aSIRCFramePWForm[0] | headers;

    /* Increment the Last word of the coded frame counter */
    SIRCCodedFrameLastWordLength = SIRCCodedFrameLastWordLength + SIRC_HEADERS_LENGTH;
  }
  else
  {
    SIRC_AddHeadersFragment(headers, lastwordfreespace);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
