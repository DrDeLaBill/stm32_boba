/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, FLASH2_CS_Pin|RESET_Pin|LED_Pin|DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, FLASH1_CS_Pin|CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ALARM_Pin|LED_UP_Pin|LED_MID_Pin|VALVE_UP_LIN_Pin
                          |VALVE_UP_HIN_Pin|VALVE_DOWN_LIN_Pin|VALVE_DOWN_HIN_Pin|LED_CENTER_Pin
                          |LED_DOWN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : FLASH2_CS_Pin FLASH1_CS_Pin CS_Pin RESET_Pin
                           LED_Pin DC_Pin */
  GPIO_InitStruct.Pin = FLASH2_CS_Pin|FLASH1_CS_Pin|CS_Pin|RESET_Pin
                          |LED_Pin|DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_F1_Pin BTN_DOWN_Pin BTN_UP_Pin BTN_ENTER_Pin */
  GPIO_InitStruct.Pin = BTN_F1_Pin|BTN_DOWN_Pin|BTN_UP_Pin|BTN_ENTER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ALARM_Pin LED_UP_Pin LED_MID_Pin VALVE_UP_LIN_Pin
                           VALVE_UP_HIN_Pin VALVE_DOWN_LIN_Pin VALVE_DOWN_HIN_Pin LED_CENTER_Pin
                           LED_DOWN_Pin */
  GPIO_InitStruct.Pin = ALARM_Pin|LED_UP_Pin|LED_MID_Pin|VALVE_UP_LIN_Pin
                          |VALVE_UP_HIN_Pin|VALVE_DOWN_LIN_Pin|VALVE_DOWN_HIN_Pin|LED_CENTER_Pin
                          |LED_DOWN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_MODE_Pin BTN_F2_Pin BTN_F3_Pin */
  GPIO_InitStruct.Pin = BTN_MODE_Pin|BTN_F2_Pin|BTN_F3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
