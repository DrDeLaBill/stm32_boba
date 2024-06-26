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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "soul.h"
#include "sensor.h"
#include "bmacro.h"
#include "system.h"
#include "at24cm01.h"
#include "hal_defs.h"

#include "App.h"
#include "SoulGuard.h"
#include "StorageAT.h"
#include "StorageDriver.h"
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

static constexpr char MAIN_TAG[] = "MAIN";

StorageDriver storageDriver;
StorageAT* storage;

UI ui;
App app;

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
  if (is_error(RCC_ERROR)) {
	  system_clock_hsi_config();
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
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
    HAL_Delay(100);

	gprint("\n\n\n");
	printTagLog(MAIN_TAG, "The device is loading");

	SystemInfo();

	SoulGuard<
		RestartWatchdog,
#if !TEST_ERRORS
		PowerWatchdog,
		StackWatchdog,
		MemoryWatchdog,
#endif
		SettingsWatchdog
	> soulGuard;

	set_status(LOADING);

    storage = new StorageAT(
		eeprom_get_size() / STORAGE_PAGE_SIZE,
		&storageDriver
	);

	while (has_errors() || is_status(LOADING)) {
		soulGuard.defend();
		ui.tick();
	}

    system_post_load();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    // Buttons TIM start
    HAL_TIM_Base_Start_IT(&BTN_TIM);

    // App TIM start
    HAL_TIM_Base_Start_IT(&APP_TIM);

    printTagLog(MAIN_TAG, "The device has been loaded");

#if TEST_ERRORS
    utl::Timer timer(1000);
    SOUL_STATUS error = ERRORS_START;
#endif

    bool foundError = false;
    utl::Timer errTimer(30 * SECOND_MS);

	set_status(WORKING);
	while (1)
	{
		utl::CodeStopwatch stopwatch(MAIN_TAG, 3 * GENERAL_TIMEOUT_MS);

#if TEST_ERRORS
		if (!timer.wait()) {
			timer.start();
			reset_error(error);
			static unsigned* ptr = (unsigned*)&error;
			(*ptr) += 1;
			if (error == ERRORS_END) {
				error = (SOUL_STATUS)((unsigned)ERRORS_START + 1);
			}
			set_error(error);
		}
#endif

		soulGuard.defend();

		ui.tick();

		if (foundError && !errTimer.wait()) {
			system_error_handler((SOUL_STATUS)get_first_error());
		}

		if (has_errors() || is_status(LOADING)) {
			if (!foundError) {
				foundError = true;
				errTimer.start();
			}
			continue;
		}
		foundError = false;

		sensor_tick();
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

int _write(int, uint8_t *ptr, int len) {
	(void)ptr;
	(void)len;
#ifdef DEBUG
    HAL_UART_Transmit(&BEDUG_UART, (uint8_t *)ptr, static_cast<uint16_t>(len), GENERAL_TIMEOUT_MS);
    for (int DataIdx = 0; DataIdx < len; DataIdx++) {
        ITM_SendChar(*ptr++);
    }
    return len;
#endif
    return 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == BTN_TIM.Instance) {
    	ui.buttonsTick();
    } else if (htim->Instance == APP_TIM.Instance) {
    	app.proccess();
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
    b_assert(__FILE__, __LINE__, "The error handler has been called");
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
	b_assert((char*)file, line, "Wrong parameters value");
	SOUL_STATUS err = has_errors() ? (SOUL_STATUS)get_first_error() : ASSERT_ERROR;
	system_error_handler(err);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
