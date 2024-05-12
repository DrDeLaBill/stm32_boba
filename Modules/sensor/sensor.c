/* Copyright © 2024 Georgy E. All rights reserved. */

#include "sensor.h"

#include <string.h>

#include "log.h"
#include "main.h"
#include "soul.h"
#include "utils.h"
#include "hal_defs.h"
#include "settings.h"


#define SENSOR_DATA_MAX_SIZE       (8)
#define SENSOR_FRAME_DELAY_MS      (400)
#define SENSOR_COMMAND_DELAY_MS    (15)
#define SENSOR_CAN_DELAY_MS        (100)
#define SENSOR_MAX_ERRORS          (100)
#define SENSOR_CONNECTION_DELAY_MS (3000)

#define SENSOR_FRAME_ID1           (0x02A7)
#define SENSOR_FRAME_ID2           (0x02A8)
#define SENSOR_FRAME_ID3           (0x02AB)
#define SENSOR_DISTANCE_FRAME_ID   (0x02)

#define SENSOR_MODE_NONE           (0)

#define SENSOR_SETTINGS_STD_ID     (0x7ED)
#define SENSOR_VALUE_STD_ID        (0x2A7)


const uint16_t SENSOR_FRAME_IDS[] = {
	SENSOR_FRAME_ID1,
	SENSOR_FRAME_ID2,
	SENSOR_FRAME_ID3,
};

typedef struct _sensor_t {
	int16_t             value;
	bool                available;
	util_old_timer_t    connection_timer;
} sensor_t;

typedef struct _sensor_state_t {
	void                (*fsm) (void);
	sensor_t            sensors[__arr_len(SENSOR_FRAME_IDS)];
	SENSOR_MODE         curr_mode;
	int16_t             curr_target;
	SENSOR_MODE         need_mode;
	uint16_t            need_std_id;

	unsigned            errors;
	util_old_timer_t    timer;
	util_old_timer_t    frame_timer;

	bool                received;
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

void _fsm_sensor_change_mode();
void _fsm_sencor_set_mode_surface();
void _fsm_sencor_set_mode_string();
void _fsm_sencor_set_mode_end1();
void _fsm_sencor_set_mode_end2();
void _fsm_sencor_set_mode_end3();

void _fsm_sensor_receive_frame();
void _fsm_sensor_send_frame1();
void _fsm_sensor_send_frame2();


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
	DWT->CYCCNT           = 0;

	CAN_RxHeaderTypeDef tmp_rx_header;
	uint8_t             tmp_rx_buffer[SENSOR_DATA_MAX_SIZE];
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &tmp_rx_header, tmp_rx_buffer) == HAL_OK) {
    	if (tmp_rx_header.StdId != sensor_state.need_std_id) {
    		reset_status(CAN_FAULT);
    		return;
    	}
    	memcpy((void*)&sensor_state.rx_header, (void*)&tmp_rx_header, sizeof(tmp_rx_header));
    	memcpy(sensor_state.rx_buffer, tmp_rx_buffer, sizeof(tmp_rx_buffer));
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

void sensor_tick()
{
	if (!sensor_state.fsm) {
		sensor_state.fsm = _fsm_sensor_init;
	}
	sensor_state.fsm();
}

bool sensor_available()
{
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
		if (sensor_state.sensors[i].available) {
			return true;
		}
	}
	return false;
}

int16_t get_sensor2A7_value()
{
	return sensor_state.sensors[0].value;
}

int16_t get_sensor2A8_value()
{
	return sensor_state.sensors[1].value;
}

int16_t get_sensor2AB_value()
{
	return sensor_state.sensors[2].value;
}

int16_t get_sensor_average_value()
{
	int16_t value = 0;
	int16_t counter = 0;
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
		if (sensor_state.sensors[i].available) {
			value += sensor_state.sensors[i].value;
			counter++;
		}
	}
	return counter ? value / counter : 0;
}

void set_sensor_mode(SENSOR_MODE mode)
{
	BEDUG_ASSERT(IS_SENSOR_MODE(mode), "Unknown sensor mode");
	if (!IS_SENSOR_MODE(mode)) {
		Error_Handler();
	}

	sensor_state.need_mode = mode;
}

SENSOR_MODE get_sensor_mode()
{
	return sensor_state.curr_mode;
}


void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data)
{
	sensor_state.received = false;

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


#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC
void _fsm_sensor_init()
{
    SCB_DEMCR   |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;

	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);

	sensor_state.need_mode   = SENSOR_MODE_SURFACE;
	sensor_state.curr_mode   = SENSOR_MODE_SURFACE;
	sensor_state.curr_target = (int16_t)0xFFFF ^ settings.last_target;

	util_old_timer_start(&sensor_state.timer, 100);
	sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
	sensor_state.fsm = _fsm_sensor_start;
}

void _fsm_sensor_idle()
{
	if (sensor_state.errors > SENSOR_MAX_ERRORS) {
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
		sensor_state.fsm = _fsm_sensor_start;
	} else if (sensor_state.need_mode != sensor_state.curr_mode ||
		settings.last_target != sensor_state.curr_target
	) {
		sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
		sensor_state.fsm = _fsm_sensor_change_mode;
	} else if (!util_old_timer_wait(&(sensor_state.frame_timer))) {
		util_old_timer_start(&sensor_state.frame_timer, SENSOR_FRAME_DELAY_MS);
		sensor_state.need_std_id = SENSOR_VALUE_STD_ID;
		sensor_state.fsm = _fsm_sensor_send_frame1;
	} else if (sensor_state.received) {
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.need_std_id = SENSOR_VALUE_STD_ID;
		sensor_state.fsm = _fsm_sensor_receive_frame;
	} else {
		sensor_state.need_std_id = SENSOR_VALUE_STD_ID;
	}

	bool status = true;
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
		if (util_old_timer_wait(&sensor_state.sensors[i].connection_timer)) {
			status = false;
		} else {
			sensor_state.sensors[i].available = false;
		}
	}
	if (status) {
		set_status(NO_SENSOR);
	}
}

void _fsm_sensor_start()
{
	static unsigned counter = 0;

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		counter = 0;
		return;
	}

	if (counter >= __arr_len(start_frames)) {
		sensor_state.errors = 0;
		sensor_state.fsm    = _fsm_sensor_idle;
		counter = 0;
		return;
	}
	_sensor_send_frame(
		start_frames[counter].std_id,
		start_frames[counter].dlc,
		start_frames[counter].data
	);
	counter++;

	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sensor_start;
}

void _fsm_sensor_change_mode()
{
	switch (sensor_state.need_mode) {
	case SENSOR_MODE_BIGSKY: // TODO: BIGSKY
	case SENSOR_MODE_SURFACE:
		sensor_state.fsm = _fsm_sencor_set_mode_surface;
		break;
	case SENSOR_MODE_STRING:
		util_old_timer_start(&sensor_state.timer, SENSOR_COMMAND_DELAY_MS);
		sensor_state.fsm = _fsm_sencor_set_mode_string;
		break;
	default:
		BEDUG_ASSERT(IS_SENSOR_MODE(sensor_state.need_mode), "Unknown sensor mode");
		Error_Handler();
		break;
	};
}

void _fsm_sencor_set_mode_surface()
{
	static const can_frame_t surface_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x02,}};

	_sensor_send_frame(surface_request.std_id, surface_request.dlc, surface_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end1;
}

void _fsm_sencor_set_mode_string()
{
	static const can_frame_t string_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x01,}};

	_sensor_send_frame(string_request.std_id, string_request.dlc, string_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end1;
}

void _fsm_sencor_set_mode_end1()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	static const uint8_t response1[] =
		{0x01, 0x0F, 0x00, 0x00, 0x19, 0x00,};

	if (memcmp(response1, sensor_state.rx_buffer, __arr_len(response1))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	int16_t target = -settings.last_target;
	can_frame_t request2 =
		{0x07EC, 0x06, {0x01, 0x0F, 0x00, 0x05, (uint8_t)(target >> 8), (uint8_t)(target)}};

	_sensor_send_frame(request2.std_id, request2.dlc, request2.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end2;
}

void _fsm_sencor_set_mode_end2()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	static const uint8_t response2[] =
		{0x01, 0x0F, 0x00, 0x00, 0x05, 0x00};

	if (memcmp(response2, sensor_state.rx_buffer, __arr_len(response2))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	static const can_frame_t request3 =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x03, 0x06}};

	_sensor_send_frame(request3.std_id, request3.dlc, request3.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end3;
}

void _fsm_sencor_set_mode_end3()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	static const uint8_t response3[] =
		{0x01, 0x0F, 0x00, 0x00, 0x03, 0x00};

	if (memcmp(response3, sensor_state.rx_buffer, __arr_len(response3))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	sensor_state.curr_mode   = sensor_state.need_mode;
	sensor_state.curr_target = settings.last_target;
	sensor_state.fsm         = _fsm_sensor_idle;
}

void _fsm_sensor_receive_frame()
{
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
	    if (sensor_state.rx_header.StdId != SENSOR_FRAME_IDS[i] ||
			sensor_state.rx_buffer[0]    != SENSOR_DISTANCE_FRAME_ID
		) {
	    	continue;
		}
	    int16_t value = ((int16_t)sensor_state.rx_buffer[1] << 8) | (int16_t)sensor_state.rx_buffer[2];
		sensor_state.sensors[i].value = value;
		sensor_state.sensors[i].available = true;
#if SENSOR_BEDUG
		printTagLog(
			SENSOR_TAG,
			"distance=%d.%d",
			value / 100,
			__abs(value % 100)
		);
#endif
		util_old_timer_start(&sensor_state.sensors[i].connection_timer, SENSOR_CONNECTION_DELAY_MS);
		reset_status(NO_SENSOR);
	}

	sensor_state.errors   = 0;
	sensor_state.received = false;
	sensor_state.fsm      = _fsm_sensor_idle;
}

void _fsm_sensor_send_frame1()
{
	uint8_t data[8] = {
		(uint8_t)(sensor_state.sensors[0].value >> 8), // TODO: for BIGSKY (3 sensors)
		(uint8_t)(sensor_state.sensors[0].value),
		0x00,
		0x0C,
		0xFE,
		0x01,
		0x00,
		0x0B
	};
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
