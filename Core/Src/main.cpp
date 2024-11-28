/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "can.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "soul.h"
#include "sensor.h"
#include "bmacro.h"
#include "gsystem.h"
#include "at24cm01.h"
#include "hal_defs.h"

#include "App.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_ERRORS (0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

#ifdef DEBUG
static constexpr char MAIN_TAG[] = "MAIN";
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	system_pre_load();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  if (is_error(SYS_TICK_ERROR)) {
	  system_hsi_config();
  } else {
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
    HAL_Delay(100);

    // Buttons TIM start
    ui_init();

    system_registrate(ui_tick,         40,  true);
    system_registrate(sensor_tick,     40,  true);
    system_registrate(app_tick,        100, true);
	system_registrate(settings_update, 50,  true);
    system_registrate(ui_btn_tick,     10,  false);

    set_system_timeout(10 * SECOND_MS);
    system_start();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_hse_config(void)
{
	SystemClock_Config();
}

void system_ready_check(void) {}

void system_error_loop()
{
	static bool initialized = false;
	static uint32_t delay_ms = 300;
	static system_timer_t led_timer = {};

	if (!initialized) {
		GPIO_InitTypeDef GPIO_InitStruct = {};

		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Pin = VALVE_UP_Pin|VALVE_DOWN_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET);
		reset_status(AUTO_NEED_VALVE_DOWN);
		reset_status(AUTO_NEED_VALVE_UP);

		__HAL_RCC_GPIOC_CLK_ENABLE();

		GPIO_InitStruct.Pin = LED_CENTER_Pin|LED_DOWN_Pin|LED_MID_Pin|LED_UP_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		HAL_GPIO_WritePin(GPIOC, LED_CENTER_Pin|LED_DOWN_Pin|LED_MID_Pin|LED_UP_Pin, GPIO_PIN_SET);

		system_timer_start(&led_timer, TIM2, delay_ms);

		initialized = true;
	}


	if (!system_timer_wait(&led_timer)) {
		system_timer_start(&led_timer, TIM2, delay_ms);
		HAL_GPIO_TogglePin(GPIOC, LED_CENTER_Pin);
		HAL_GPIO_TogglePin(GPIOC, LED_DOWN_Pin);
		HAL_GPIO_TogglePin(GPIOC, LED_MID_Pin);
		HAL_GPIO_TogglePin(GPIOC, LED_UP_Pin);
	}
}

char* get_custom_status_name(SOUL_STATUS status)
{
	static char name[35] = { 0 };
	memset(name, 0, sizeof(name));

	switch (status) {
	SYSTEM_CASE_STATUS(name, NEED_SERVICE_BACK)
	SYSTEM_CASE_STATUS(name, NEED_SERVICE_SAVE)
	SYSTEM_CASE_STATUS(name, NEED_SERVICE_UPDATE)
	SYSTEM_CASE_STATUS(name, MANUAL_NEED_VALVE_UP)
	SYSTEM_CASE_STATUS(name, MANUAL_NEED_VALVE_DOWN)
	SYSTEM_CASE_STATUS(name, AUTO_NEED_VALVE_UP)
	SYSTEM_CASE_STATUS(name, AUTO_NEED_VALVE_DOWN)
	default:
		snprintf(name, sizeof(name) - 1, "%s", get_custom_status_name(status));
		break;
	}

	return name;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
#ifdef DEBUG
    b_assert(__FILE__, __LINE__, "The error handler has been called");
#endif
    SOUL_STATUS err = has_errors() ? (SOUL_STATUS)get_first_error() : ERROR_HANDLER_CALLED;
	system_error_handler(err);
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
#ifdef DEBUG
	b_assert((char*)file, line, "Wrong parameters value");
#endif
	SOUL_STATUS err = has_errors() ? (SOUL_STATUS)get_first_error() : ASSERT_ERROR;
	system_error_handler(err);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
