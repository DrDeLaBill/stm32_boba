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

#define SENSOR_FRAME_ID1           (0x02AB)
#define SENSOR_FRAME_ID2           (0x02A7)
#define SENSOR_FRAME_ID3           (0x02A8)
#define SENSOR_DISTANCE_FRAME_ID   (0x02)

#define SENSOR_MODE_NONE           (0)

#define SENSOR_SETTINGS_STD_ID     (0x7ED)
#define SENSOR_VALUE_STD_ID        (0)


const uint16_t SENSOR_FRAME_IDS[] = {
	SENSOR_FRAME_ID1,
	SENSOR_FRAME_ID2,
	SENSOR_FRAME_ID3,
};

typedef struct _sensor_t {
	int16_t             value;
	util_old_timer_t    connection_timer;
	STRING_DIRECTION    direction;
} sensor_t;

typedef struct _sensor_state_t {
	void                (*fsm) (void);
	bool                enabled;
	sensor_t            sensors[__arr_len(SENSOR_FRAME_IDS)];
	SENSOR_MODE         curr_mode;
	SENSOR_MODE         need_mode;
	int16_t             curr_target;
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

	uint8_t             bigski_id;
} sensor_state_t;

typedef struct _can_frame_t {
	uint32_t std_id;
	uint32_t dlc;
	uint8_t  data[SENSOR_DATA_MAX_SIZE];
} can_frame_t;


void _check_stop();
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
void _fsm_sencor_set_mode_bigski1();
void _fsm_sencor_set_mode_bigski2();
void _fsm_sencor_set_mode_bigski3();

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

static const uint8_t BIGSKI_IDS[] = {0x02, 0x00, 0x04};

extern CAN_HandleTypeDef hcan;

sensor_state_t sensor_state = {
	.fsm       = _fsm_sensor_init,
	.curr_mode = SENSOR_MODE_SURFACE,
	.bigski_id = 0,
};


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	_check_stop();

	DWT->CYCCNT = 0;

	CAN_RxHeaderTypeDef tmp_rx_header = {0};
	uint8_t             tmp_rx_buffer[SENSOR_DATA_MAX_SIZE] = {0};
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &tmp_rx_header, tmp_rx_buffer) == HAL_OK) {
    	if (sensor_state.need_std_id && tmp_rx_header.StdId != sensor_state.need_std_id) {
    		reset_status(CAN_FAULT);
    		return;
    	}
    	bool is_value = false;
    	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
    	    if (tmp_rx_header.StdId != SENSOR_FRAME_IDS[i] ||
				tmp_rx_buffer[0]    != SENSOR_DISTANCE_FRAME_ID
    		) {
    	    	continue;
    		}
    		sensor_state.sensors[i].value     = ((int16_t)tmp_rx_buffer[1] << 8) | (int16_t)tmp_rx_buffer[2];
    		sensor_state.sensors[i].direction = tmp_rx_buffer[3];
    		is_value = true;
#if SENSOR_BEDUG
    		printTagLog(
    			"SNS",
    			"distance[%X]=%d.%d",
				(i == 0 ? SENSOR_FRAME_ID1 : i == 1 ? SENSOR_FRAME_ID2 : SENSOR_FRAME_ID3),
				sensor_state.sensors[i].value / 100,
    			__abs(sensor_state.sensors[i].value % 100)
    		);
#endif
    		util_old_timer_start(&sensor_state.sensors[i].connection_timer, SENSOR_CONNECTION_DELAY_MS);
    	}
    	if (!is_value) {
			memcpy((void*)&sensor_state.rx_header, (void*)&tmp_rx_header, sizeof(tmp_rx_header));
			memcpy(sensor_state.rx_buffer, tmp_rx_buffer, sizeof(tmp_rx_buffer));
			sensor_state.received = true;
    	}
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
	if (get_sensor_mode() != SENSOR_MODE_BIGSKI) {
		return sensor2A7_available();
	}

	return sensor2AB_available() || sensor2A7_available() || sensor2A8_available();
}

int16_t get_sensor2AB_value()
{
	return sensor_state.sensors[0].value;
}

int16_t get_sensor2A7_value()
{
	return sensor_state.sensors[1].value;
}

int16_t get_sensor2A8_value()
{
	return sensor_state.sensors[2].value;
}

int16_t get_sensor_average()
{
	int16_t value = 0;
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
		value += sensor_state.sensors[i].value;
	}
	return value / (int16_t)__arr_len(sensor_state.sensors);
}

int16_t get_sensor_mode_target(SENSOR_MODE mode)
{
	switch (mode) {
	case SENSOR_MODE_SURFACE:
		return settings.surface_target;
	case SENSOR_MODE_STRING:
		return settings.string_target;
	case SENSOR_MODE_BIGSKI:
		return settings.bigski_target[1];
	default:
		Error_Handler();
		return 0;
	}
}

void save_sensor_mode_target()
{
	switch (get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		settings.surface_target += get_sensor2A7_value();
		break;
	case SENSOR_MODE_STRING:
		settings.string_target += get_sensor2A7_value();
		break;
	case SENSOR_MODE_BIGSKI:
		settings.bigski_target[0] += get_sensor2AB_value();
		settings.bigski_target[1] += get_sensor2A7_value();
		settings.bigski_target[2] += get_sensor2A8_value();
		break;
	default:
		Error_Handler();
		break;
	}
}

void reset_sensor_mode_target()
{
	switch (get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		settings.surface_target = 0;
		break;
	case SENSOR_MODE_STRING:
		settings.string_target = 0;
		break;
	case SENSOR_MODE_BIGSKI:
		settings.bigski_target[0] = 0;
		settings.bigski_target[1] = 0;
		settings.bigski_target[2] = 0;
		break;
	default:
		Error_Handler();
		break;
	}
}

bool sensor2AB_available()
{
	return util_old_timer_wait(&sensor_state.sensors[0].connection_timer);
}

bool sensor2A7_available()
{
	return util_old_timer_wait(&sensor_state.sensors[1].connection_timer);
}

bool sensor2A8_available()
{
	return util_old_timer_wait(&sensor_state.sensors[2].connection_timer);
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

STRING_DIRECTION get_sensor_direction()
{
	if (get_sensor_mode() == SENSOR_MODE_STRING) {
		return sensor_state.sensors[1].direction;
	}
	return STR_MIDDLE;
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


void _check_stop()
{
	if (sensor_state.enabled != is_status(WAIT_LOAD)) {
		is_status(WAIT_LOAD) ?
			HAL_CAN_DeactivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE) :
			HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
		sensor_state.enabled = is_status(WAIT_LOAD);
	}
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
	sensor_state.curr_target = 0;
	sensor_state.bigski_id   = 0;

	util_old_timer_start(&sensor_state.timer, 100);
	sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
	sensor_state.fsm = _fsm_sensor_start;
}

void _fsm_sensor_idle()
{
	_check_stop();

	if (is_status(WAIT_LOAD)) {
		return;
	}

	if (sensor_state.errors > SENSOR_MAX_ERRORS) {
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
		sensor_state.fsm = _fsm_sensor_start;
	} else if (
		sensor_state.need_mode   != sensor_state.curr_mode ||
		sensor_state.curr_target != get_sensor_mode_target(sensor_state.need_mode)
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

	sensor_available() ? reset_status(NO_SENSOR) : set_status(NO_SENSOR);
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
	case SENSOR_MODE_BIGSKI:
		sensor_state.fsm = _fsm_sencor_set_mode_bigski1;
		break;
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
	can_frame_t surface_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x02,}};

	_sensor_send_frame(surface_request.std_id, surface_request.dlc, surface_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end1;
}

void _fsm_sencor_set_mode_string()
{
	can_frame_t string_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x01,}};

	_sensor_send_frame(string_request.std_id, string_request.dlc, string_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.fsm = _fsm_sencor_set_mode_end1;
}

void _fsm_sencor_set_mode_bigski1()
{
	int16_t value = -settings.bigski_target[sensor_state.bigski_id];
	can_frame_t request =
			{0x07EC, 0x06, {0x01, 0x0F, BIGSKI_IDS[sensor_state.bigski_id], 0x05, (uint8_t)(value >> 8), (uint8_t)value}};

	if (sensor_state.bigski_id >= __arr_len(BIGSKI_IDS)) {
		sensor_state.bigski_id = 0;
		can_frame_t mode_request =
				{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x12, 0x00,}};
		_sensor_send_frame(mode_request.std_id, mode_request.dlc, mode_request.data);
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.fsm = _fsm_sencor_set_mode_bigski3;
	} else {
		_sensor_send_frame(request.std_id, request.dlc, request.data);
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.fsm = _fsm_sencor_set_mode_bigski2;
	}
}

void _fsm_sencor_set_mode_bigski2()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response[] =
		{0x01, 0x0F, BIGSKI_IDS[sensor_state.bigski_id], 0x00, 0x05, 0x00,};

	if (memcmp(response, sensor_state.rx_buffer, __arr_len(response))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	sensor_state.bigski_id++;
	sensor_state.fsm = _fsm_sencor_set_mode_bigski1;
}

void _fsm_sencor_set_mode_bigski3()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response[] = {0x01, 0x0F, 0x00, 0x00, 0x12, 0x00,};

	if (memcmp(response, sensor_state.rx_buffer, __arr_len(response))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	sensor_state.bigski_id   = 0;
	sensor_state.curr_mode   = sensor_state.need_mode;
	sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);
	sensor_state.fsm         = _fsm_sensor_idle;
}

void _fsm_sencor_set_mode_end1()
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response1[] =
		{0x01, 0x0F, 0x00, 0x00, 0x19, 0x00,};

	if (memcmp(response1, sensor_state.rx_buffer, __arr_len(response1))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	int16_t target = -get_sensor_mode_target(sensor_state.need_mode);
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
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response2[] =
		{0x01, 0x0F, 0x00, 0x00, 0x05, 0x00};

	if (memcmp(response2, sensor_state.rx_buffer, __arr_len(response2))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	can_frame_t request3 =
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
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response3[] =
		{0x01, 0x0F, 0x00, 0x00, 0x03, 0x00};

	if (memcmp(response3, sensor_state.rx_buffer, __arr_len(response3))) {
		sensor_state.fsm = _fsm_sensor_idle;
		sensor_state.errors++;
		return;
	}

	sensor_state.curr_mode   = sensor_state.need_mode;
	sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);
	sensor_state.fsm         = _fsm_sensor_idle;
}

void _fsm_sensor_receive_frame()
{
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
