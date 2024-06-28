/* Copyright © 2023 Georgy E. All rights reserved. */

#include "system.h"

#include "main.h"
#include "hal_defs.h"


uint16_t SYSTEM_ADC_VOLTAGE = 0;


void system_clock_hsi_config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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

	reset_error(RCC_ERROR);
}

void system_pre_load(void)
{
	if (!MCUcheck()) {
		set_error(MCU_ERROR);
		while (1) {}
	}

	SET_BIT(RCC->CR, RCC_CR_HSEON_Pos);

	unsigned counter = 0;
	while (1) {
		if (READ_BIT(RCC->CR, RCC_CR_HSERDY_Pos)) {
			CLEAR_BIT(RCC->CR, RCC_CR_HSEON_Pos);
			break;
		}

		if (counter > 0x100) {
			set_status(RCC_FAULT);
			set_error(RCC_ERROR);
			break;
		}

		counter++;
	}

	uint32_t backupregister = (uint32_t)BKP_BASE;
	backupregister += (RTC_BKP_DR1 * 4U);

	SOUL_STATUS status = (SOUL_STATUS)((*(__IO uint32_t *)(backupregister)) & BKP_DR1_D);
	set_last_error(status);
	switch (status) {
	case RCC_ERROR:
        break;
    case MEMORY_ERROR:
    	set_error(MEMORY_ERROR);
        break;
    case POWER_ERROR:
        break;
    case STACK_ERROR:
    	set_error(STACK_ERROR);
        break;
    case LOAD_ERROR:
        break;
    case RAM_ERROR:
        break;
    case USB_ERROR:
        break;
    case SETTINGS_LOAD_ERROR:
    	set_error(SETTINGS_LOAD_ERROR);
        break;
    case APP_MODE_ERROR:
        break;
    case VALVE_ERROR:
        break;
    case NON_MASKABLE_INTERRUPT:
        break;
    case HARD_FAULT:
        break;
    case MEM_MANAGE:
        break;
    case BUS_FAULT:
        break;
    case USAGE_FAULT:
        break;
    case ASSERT_ERROR:
        break;
    case ERROR_HANDLER_CALLED:
    	break;
    case INTERNAL_ERROR:
        break;
    default:
		break;
	}
}

void system_post_load(void)
{
	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
	HAL_PWR_DisableBkUpAccess();

	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&SYSTEM_ADC_VOLTAGE, 1);
	unsigned counter = 0;
	while (1) {
		uint16_t voltage = 0;
		if (SYSTEM_ADC_VOLTAGE) {
			voltage = STM_ADC_MAX * STM_REF_VOLTAGEx10 / SYSTEM_ADC_VOLTAGE;
		}

		if (STM_MIN_VOLTAGEx10 <= voltage && voltage <= STM_MAX_VOLTAGEx10) {
			break;
		}

		if (counter > 0x1000) {
			set_error(POWER_ERROR);
			break;
		}

		counter++;
	}

	if (has_errors()) {
		system_error_handler(
			(get_first_error() == INTERNAL_ERROR) ?
				LOAD_ERROR :
				(SOUL_STATUS)get_first_error()
		);
	}
}

void system_error_handler(SOUL_STATUS error)
{
	static bool called = false;
	if (called) {
		return;
	}
	called = true;

	set_error(error);

	if (!has_errors()) {
		error = INTERNAL_ERROR;
	}

	HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET);
	reset_status(AUTO_NEED_VALVE_DOWN);
	reset_status(AUTO_NEED_VALVE_UP);

	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, error);
	HAL_PWR_DisableBkUpAccess();

	uint32_t counter = 0x100;
	while(--counter) {}
	NVIC_SystemReset();
}
