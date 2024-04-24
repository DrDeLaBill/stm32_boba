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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

int _write(int file, uint8_t *ptr, int len);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DISP_RST_Pin GPIO_PIN_4
#define DISP_RST_GPIO_Port GPIOA
#define LED_CENTER_Pin GPIO_PIN_4
#define LED_CENTER_GPIO_Port GPIOC
#define LED_UP_Pin GPIO_PIN_5
#define LED_UP_GPIO_Port GPIOC
#define VALVE_DOWN_Pin GPIO_PIN_12
#define VALVE_DOWN_GPIO_Port GPIOB
#define VALVE_UP_Pin GPIO_PIN_13
#define VALVE_UP_GPIO_Port GPIOB
#define RESERVED2_Pin GPIO_PIN_14
#define RESERVED2_GPIO_Port GPIOB
#define RESERVED3_Pin GPIO_PIN_15
#define RESERVED3_GPIO_Port GPIOB
#define LED_DOWN_Pin GPIO_PIN_6
#define LED_DOWN_GPIO_Port GPIOC
#define LED_MIDDLE_Pin GPIO_PIN_7
#define LED_MIDDLE_GPIO_Port GPIOC
#define BTN_UP_Pin GPIO_PIN_9
#define BTN_UP_GPIO_Port GPIOC
#define BTN_UP_EXTI_IRQn EXTI9_5_IRQn
#define DISP_CS_Pin GPIO_PIN_8
#define DISP_CS_GPIO_Port GPIOA
#define DISP_RS_Pin GPIO_PIN_9
#define DISP_RS_GPIO_Port GPIOA
#define DISP_BL_Pin GPIO_PIN_10
#define DISP_BL_GPIO_Port GPIOA
#define BTN_MODE_Pin GPIO_PIN_10
#define BTN_MODE_GPIO_Port GPIOC
#define BTN_MODE_EXTI_IRQn EXTI15_10_IRQn
#define BTN_DOWN_Pin GPIO_PIN_11
#define BTN_DOWN_GPIO_Port GPIOC
#define BTN_DOWN_EXTI_IRQn EXTI15_10_IRQn
#define BTN_ENTER_Pin GPIO_PIN_12
#define BTN_ENTER_GPIO_Port GPIOC
#define BTN_ENTER_EXTI_IRQn EXTI15_10_IRQn
#define ALARM_Pin GPIO_PIN_8
#define ALARM_GPIO_Port GPIOB
#define RESERVED1_Pin GPIO_PIN_9
#define RESERVED1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// General settings
#define GENERAL_TIMEOUT_MS       ((uint32_t)100)

// EEPROM
extern I2C_HandleTypeDef         hi2c2;
#define EEPROM_I2C               (hi2c2)

// BEDUG UART
extern UART_HandleTypeDef        huart1;
#define BEDUG_UART               (huart1)

// Display
extern SPI_HandleTypeDef         hspi1;
#define DISPLAY_SPI              (hspi1)


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
