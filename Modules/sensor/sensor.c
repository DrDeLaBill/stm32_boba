/* Copyright © 2024 Georgy E. All rights reserved. */

#include "sensor.h"

#include <string.h>

#include "log.h"
#include "soul.h"
#include "utils.h"
#include "hal_defs.h"


#define SENSOR_DATA_MAX_SIZE     (8)
#define SENSOR_FRAME_DELAY_MS    (400)
#define SENSOR_COMMAND_DELAY_MS  (5)
#define SENSOR_MAX_ERRORS        (10)

#define SENSOR_FRAME_ID          (0x02A7)
#define SENSOR_DISTANCE_FRAME_ID (0x02)


typedef struct _sensor_state_t {
	void                (*fsm) (void);
	int16_t             value;
	bool                received;
	bool                available;

	uint8_t             errors;
	util_old_timer_t    timer;
	util_old_timer_t    frame_timer;

	uint32_t            tx_mailbox;
	CAN_TxHeaderTypeDef tx_header;
	uint8_t             tx_buffer[SENSOR_DATA_MAX_SIZE];
	CAN_RxHeaderTypeDef rx_header;
	uint8_t             rx_buffer[SENSOR_DATA_MAX_SIZE];
} sensor_state_t;

typedef struct _can_frame_t {
	uint32_t std_id;
	uint32_t dlc;
	uint8_t  data[SENSOR_DATA_MAX_SIZE];
} can_frame_t;


void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data);

void _fsm_sensor_init();
void _fsm_sensor_idle();
void _fsm_sensor_start();
void _fsm_sensor_receive_frame();
void _fsm_sensor_send_frame1();
void _fsm_sensor_send_frame2();


static const char SENSOR_TAG[] = "SENS";
static const can_frame_t start_frames[] = {
	{0x0050, 0x05, {0x09,}},
	{0x0028, 0x08, {0x00, 0x9F, 0x1E, 0x0C, 0xFE, 0x01, 0x00, 0x00}},
	{0x0050, 0x05, {0x09, 0x0B,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x02,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x01,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0xCD,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x02,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x19,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x15,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x00, 0x16,}},
	{0x07EC, 0x06, {0x01, 0x0F, 0x00, 0x05, 0xF2, 0xDD,}},
	{0x07EC, 0x06, {0x01, 0x0F, 0x00, 0x17, 0x0C, 0xFE,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x23, 0x00,}},
	{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x22, 0x01,}},
	{0x07EC, 0x04, {0x01, 0x0F, 0x00, 0xFF,}},
};

extern CAN_HandleTypeDef hcan;

sensor_state_t sensor_state = {
	.fsm = _fsm_sensor_init,
};


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	sensor_state.received = false;
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &sensor_state.rx_header, sensor_state.rx_buffer) == HAL_OK) {
    	sensor_state.received = true;
    }
	reset_status(CAN_FAULT);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	(void)hcan;
	sensor_state.errors++;
	set_status(CAN_FAULT);
}


void sensor_init()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT       = 0;
	DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;

	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
}

void sensor_tick()
{
	if (!sensor_state.fsm) {
		sensor_state.fsm = _fsm_sensor_init;
	}
	sensor_state.fsm();
}

bool sensor_available()
{
	return sensor_state.available;
}

int16_t get_sensor_value()
{
	return sensor_state.value;
}


void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data)
{
	sensor_state.tx_header.RTR                = CAN_RTR_DATA;
	sensor_state.tx_header.IDE                = CAN_ID_STD;
	sensor_state.tx_header.TransmitGlobalTime = DISABLE;
	sensor_state.tx_mailbox                   = 0;
	sensor_state.tx_header.StdId              = std_id;
	sensor_state.tx_header.DLC                = dlc;
	memset(sensor_state.tx_buffer, 0 , sizeof(sensor_state.tx_buffer));
	memcpy(sensor_state.tx_buffer, data, __min(sizeof(sensor_state.tx_buffer), dlc));
	HAL_CAN_AddTxMessage(&hcan, &sensor_state.tx_header, sensor_state.tx_buffer, &sensor_state.tx_mailbox);
}


void _fsm_sensor_init()
{
	sensor_state.fsm = _fsm_sensor_start;
}

void _fsm_sensor_idle()
{
	if (!util_old_timer_wait(&(sensor_state.frame_timer))) {
		util_old_timer_start(&sensor_state.frame_timer, SENSOR_FRAME_DELAY_MS);
		sensor_state.fsm = _fsm_sensor_send_frame1;
	}
	if (sensor_state.received) {
		sensor_state.fsm = _fsm_sensor_receive_frame;
	}
	if (sensor_state.errors > SENSOR_MAX_ERRORS) {
		sensor_state.fsm = _fsm_sensor_start;
	}
}

void _fsm_sensor_start()
{
	if (util_old_timer_wait(&sensor_state.timer)) {
		return;
	}

	static unsigned counter = 0;
	if (counter >= __arr_len(start_frames)) {
		counter = 0;
		sensor_state.fsm = _fsm_sensor_idle;
		return;
	}
	_sensor_send_frame(
		start_frames[counter].std_id,
		start_frames[counter].dlc,
		start_frames[counter].data
	);
	counter++;

	util_old_timer_start(&sensor_state.timer, SENSOR_COMMAND_DELAY_MS);
	sensor_state.fsm = _fsm_sensor_start;
}

void _fsm_sensor_receive_frame()
{
    if (sensor_state.rx_header.StdId == SENSOR_FRAME_ID &&
		sensor_state.rx_buffer[0]    == SENSOR_DISTANCE_FRAME_ID
	) {
		sensor_state.value = ((int16_t)sensor_state.rx_buffer[1] << 8) | (int16_t)sensor_state.rx_buffer[2];
		printTagLog(
			SENSOR_TAG,
			"distance=%d.%d",
			sensor_state.value / 100,
			sensor_state.value % 100
		);
    }

	sensor_state.fsm = _fsm_sensor_idle;
}

void _fsm_sensor_send_frame1()
{
	uint8_t data[8] = {(uint8_t)(sensor_state.value >> 8), (uint8_t)(sensor_state.value), 0x00, 0x0C, 0xFE, 0x01, 0x00, 0x0B};
	_sensor_send_frame(0x0028, 0x08, data);

	util_old_timer_start(&sensor_state.timer, SENSOR_COMMAND_DELAY_MS);
	sensor_state.fsm = _fsm_sensor_send_frame2;
}

void _fsm_sensor_send_frame2()
{
	if (util_old_timer_wait(&sensor_state.timer)) {
		return;
	}

	uint8_t data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	_sensor_send_frame(0x03F0, 0x08, data);

	sensor_state.fsm = _fsm_sensor_idle;
}
