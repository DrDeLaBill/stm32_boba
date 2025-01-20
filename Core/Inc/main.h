/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "soul.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FLASH2_CS_Pin GPIO_PIN_1
#define FLASH2_CS_GPIO_Port GPIOA
#define FLASH1_CS_Pin GPIO_PIN_4
#define FLASH1_CS_GPIO_Port GPIOA
#define BTN_F1_Pin GPIO_PIN_5
#define BTN_F1_GPIO_Port GPIOC
#define ALARM_Pin GPIO_PIN_2
#define ALARM_GPIO_Port GPIOB
#define LED_UP_Pin GPIO_PIN_10
#define LED_UP_GPIO_Port GPIOB
#define LED_MID_Pin GPIO_PIN_11
#define LED_MID_GPIO_Port GPIOB
#define VALVE_UP_LIN_Pin GPIO_PIN_12
#define VALVE_UP_LIN_GPIO_Port GPIOB
#define VALVE_UP_HIN_Pin GPIO_PIN_13
#define VALVE_UP_HIN_GPIO_Port GPIOB
#define VALVE_DOWN_LIN_Pin GPIO_PIN_14
#define VALVE_DOWN_LIN_GPIO_Port GPIOB
#define VALVE_DOWN_HIN_Pin GPIO_PIN_15
#define VALVE_DOWN_HIN_GPIO_Port GPIOB
#define BTN_DOWN_Pin GPIO_PIN_6
#define BTN_DOWN_GPIO_Port GPIOC
#define BTN_UP_Pin GPIO_PIN_7
#define BTN_UP_GPIO_Port GPIOC
#define BTN_ENTER_Pin GPIO_PIN_8
#define BTN_ENTER_GPIO_Port GPIOC
#define CS_Pin GPIO_PIN_8
#define CS_GPIO_Port GPIOA
#define RESET_Pin GPIO_PIN_9
#define RESET_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_10
#define LED_GPIO_Port GPIOA
#define DC_Pin GPIO_PIN_15
#define DC_GPIO_Port GPIOA
#define BTN_MODE_Pin GPIO_PIN_4
#define BTN_MODE_GPIO_Port GPIOB
#define BTN_F2_Pin GPIO_PIN_5
#define BTN_F2_GPIO_Port GPIOB
#define BTN_F3_Pin GPIO_PIN_6
#define BTN_F3_GPIO_Port GPIOB
#define LED_CENTER_Pin GPIO_PIN_8
#define LED_CENTER_GPIO_Port GPIOB
#define LED_DOWN_Pin GPIO_PIN_9
#define LED_DOWN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// General settings
#define GENERAL_TIMEOUT_MS       ((uint32_t)100)
// Display
extern SPI_HandleTypeDef         hspi3;
#define DISPLAY_SPI              (hspi3)

// RTC
extern RTC_HandleTypeDef         hrtc;

// ADC
extern ADC_HandleTypeDef         hadc1;

extern TIM_HandleTypeDef         htim3;
#define DISPLAY_TIM              (htim3)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
