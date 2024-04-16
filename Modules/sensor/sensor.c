/* Copyright © 2024 Georgy E. All rights reserved. */

#include "sensor.h"

#include "log.h"
#include "utils.h"
#include "hal_defs.h"



static const char SENSOR_TAG[] = "SENS";

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t TxData[8] = {0,};
uint8_t RxData[8] = {0,};
uint32_t TxMailbox = 0;

uint32_t value = 0;


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
    	printTagLog(SENSOR_TAG, "Data:");
    	printPretty("0x04X - ", RxHeader.StdId);
    	for (unsigned i = 0; i < __arr_len(RxData); i++) {
    		gprint("0x%02X ", RxData[i]);
    	}
    }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    uint32_t er = HAL_CAN_GetError(hcan);
    printTagLog(SENSOR_TAG, "ER CAN %lu %08lX", er, er);
}


void sensor_init()
{
	extern CAN_HandleTypeDef hcan;
	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
}

void sensor_tick()
{
//	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) {
//		return;
//	}
//
//	if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
//		HAL_UART_Transmit(&huart1, (uint8_t*)"ER SEND\n", 8, 100);
//	}
}

bool sensor_available()
{

}

uint32_t get_sensor_value()
{
	return value;
}
