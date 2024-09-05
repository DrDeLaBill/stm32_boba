/* Copyright © 2024 Georgy E. All rights reserved. */

#include "sensor.h"

#include <string.h>

#include "glog.h"
#include "main.h"
#include "soul.h"
#include "gutils.h"
#include "fsm_gc.h"
#include "hal_defs.h"
#include "settings.h"


#define SENSOR_DATA_MAX_SIZE       (8)
#define SENSOR_FRAME_DELAY_MS      (400)
#define SENSOR_COMMAND_DELAY_MS    (15)
#define SENSOR_CAN_DELAY_MS        (200)
#define SENSOR_MAX_ERRORS          (100)
#define SENSOR_CONNECTION_DELAY_MS (300)

#define SENSOR_FRAME_ID1           (0x02AB)
#define SENSOR_FRAME_ID2           (0x02A7)
#define SENSOR_FRAME_ID3           (0x02A8)
#define SENSOR_DISTANCE_FRAME_ID   (0x02)

#define SENSOR_FRAME_ANGLE         (0x02AC)

#define SENSOR_MODE_NONE           (0)

#define SENSOR_SETTINGS_STD_ID     (0x7ED)
#define SENSOR_VALUE_STD_ID        (0)


const uint16_t SENSOR_FRAME_IDS[] = {
	SENSOR_FRAME_ID1,
	SENSOR_FRAME_ID2,
	SENSOR_FRAME_ID3,
	SENSOR_FRAME_ANGLE
};

typedef struct _sensor_t {
	int16_t             value;
	util_old_timer_t    connection_timer;
	STRING_DIRECTION    direction;
} sensor_t;

typedef struct _sensor_state_t {
	bool                initialized;
	bool                enabled;
	bool                no_sensor;
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


static int16_t get_sensor2A7_value();
static int16_t get_sensor2A8_value();
static int16_t get_sensor2AB_value();
static int16_t get_sensor_average();
static int16_t get_sensor_angle();

static bool sensor_angle_available();

static void _check_stop();
static bool _recieve_distance(CAN_RxHeaderTypeDef* rx_header, uint8_t* rx_buffer);
static void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data);


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
};

static const uint8_t BIGSKI_IDS[] = {0x00, 0x02, 0x04};

extern CAN_HandleTypeDef hcan;

sensor_state_t sensor_state = {
	.initialized = false,
	.curr_mode   = SENSOR_MODE_SURFACE,
	.need_mode   = SENSOR_MODE_SURFACE,
	.bigski_id   = 0,
};


static void _init_s(void);
static void _idle_s(void);
static void _start_s(void);
static void _change_s(void);
static void _bigski1_s(void);
static void _bigski2_s(void);
static void _bigski3_s(void);
static void _angle_s(void);
static void _end1_s(void);
static void _end2_s(void);
static void _end3_s(void);
static void _send_s(void);

static void start_sensor_a(void);
static void start_change_a(void);
static void start_idle_a(void);
static void surface_a(void);
static void string_a(void);
static void send_a(void);
static void recieve_a(void);
static void error_a(void);


FSM_GC_CREATE(sens_fsm)

FSM_GC_CREATE_EVENT(success_e,  0)
FSM_GC_CREATE_EVENT(timeout_e,  0)
FSM_GC_CREATE_EVENT(recieved_e, 0)
FSM_GC_CREATE_EVENT(bigski1_e,  0)
FSM_GC_CREATE_EVENT(bigski2_e,  0)
FSM_GC_CREATE_EVENT(surface_e,  0)
FSM_GC_CREATE_EVENT(string_e,   0)
FSM_GC_CREATE_EVENT(angle_e,    0)
FSM_GC_CREATE_EVENT(send_e,     0)
FSM_GC_CREATE_EVENT(change_e,   1)
FSM_GC_CREATE_EVENT(error_e,    2)

FSM_GC_CREATE_STATE(init_s,    _init_s)
FSM_GC_CREATE_STATE(idle_s,    _idle_s)
FSM_GC_CREATE_STATE(start_s,   _start_s)
FSM_GC_CREATE_STATE(change_s,  _change_s)
FSM_GC_CREATE_STATE(bigski1_s, _bigski1_s)
FSM_GC_CREATE_STATE(bigski2_s, _bigski2_s)
FSM_GC_CREATE_STATE(bigski3_s, _bigski3_s)
FSM_GC_CREATE_STATE(angle_s,   _angle_s)
FSM_GC_CREATE_STATE(end1_s,    _end1_s)
FSM_GC_CREATE_STATE(end2_s,    _end2_s)
FSM_GC_CREATE_STATE(end3_s,    _end3_s)
FSM_GC_CREATE_STATE(send_s,    _send_s)

FSM_GC_CREATE_TABLE(
	sens_fsm_table,
	{&init_s,   &success_e,  &start_s,    start_sensor_a},

	{&idle_s,    &send_e,     &send_s,    send_a},
	{&idle_s,    &change_e,   &change_s,  start_change_a},
	{&idle_s,    &recieved_e, &idle_s,    recieve_a},
	{&idle_s,    &error_e,    &start_s,   start_sensor_a},

	{&start_s,   &success_e,  &idle_s,    start_idle_a},
	{&start_s,   &error_e,    &idle_s,    error_a},

	{&change_s,  &bigski1_e,  &bigski1_s, NULL},
	{&change_s,  &surface_e,  &end1_s,    surface_a},
	{&change_s,  &string_e,   &end1_s,    string_a},
	{&change_s,  &angle_e,    &angle_s,   NULL},

	{&bigski1_s, &bigski1_e,  &bigski2_s, NULL},
	{&bigski1_s, &bigski2_e,  &bigski3_s, NULL},
	{&bigski2_s, &timeout_e,  &idle_s,    error_a},
	{&bigski2_s, &success_e,  &bigski1_s, NULL},
	{&bigski3_s, &timeout_e,  &idle_s,    error_a},
	{&bigski3_s, &success_e,  &idle_s,    start_idle_a},

	{&end1_s,    &timeout_e,  &idle_s,    error_a},
	{&end1_s,    &success_e,  &end2_s,    NULL},
	{&end2_s,    &timeout_e,  &idle_s,    error_a},
	{&end2_s,    &success_e,  &end3_s,    NULL},
	{&end3_s,    &timeout_e,  &idle_s,    error_a},
	{&end3_s,    &success_e,  &idle_s,    start_idle_a},

	{&send_s,    &success_e,  &idle_s,    start_idle_a},
)

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	_check_stop();

	DWT->CYCCNT = 0;

	CAN_RxHeaderTypeDef tmp_rx_header = {0};
	uint8_t             tmp_rx_buffer[SENSOR_DATA_MAX_SIZE] = {0};
    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &tmp_rx_header, tmp_rx_buffer) == HAL_OK) {
    	bool is_value = false;
    	switch (tmp_rx_header.StdId) {
    	case SENSOR_FRAME_ANGLE:
			sensor_state.sensors[3].value = ((int16_t)tmp_rx_buffer[0] << 8) | (int16_t)tmp_rx_buffer[1];
			is_value = true;
    		break;
    	default:
    		is_value = _recieve_distance(&tmp_rx_header, tmp_rx_buffer);
    		break;
    	}
    	if (is_value ||
			!sensor_state.need_std_id ||
			(tmp_rx_header.StdId != sensor_state.need_std_id)
		) {
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
	if (!sens_fsm._initialized) {
		fsm_gc_init(&sens_fsm, sens_fsm_table, __arr_len(sens_fsm_table));
	}
	fsm_gc_proccess(&sens_fsm);
}

bool sensor_available()
{
	if (get_sensor_mode() == SENSOR_MODE_ANGLE) {
		return sensor_angle_available();
	}

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

int16_t get_sensor_angle()
{
	return 360;
}

int16_t get_sensor_value()
{
	switch (sensor_state.curr_mode) {
	case SENSOR_MODE_SURFACE:
	case SENSOR_MODE_STRING:
		return get_sensor2A7_value();
	case SENSOR_MODE_BIGSKI:
		return get_sensor_average();
	case SENSOR_MODE_ANGLE:
		return get_sensor_angle();
	default:
		Error_Handler();
		return 0;
	}
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
	case SENSOR_MODE_ANGLE:
		return settings.angle_target;
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
	case SENSOR_MODE_ANGLE:
		settings.angle_target += get_sensor_angle();
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
	case SENSOR_MODE_ANGLE:
		settings.angle_target = 0;
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

bool sensor_angle_available()
{
	return util_old_timer_wait(&sensor_state.sensors[3].connection_timer);
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

SENSOR_MODE get_sensor_target_mode()
{
	return sensor_state.need_mode;
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
	if (sensor_state.enabled == is_status(LOADING)) {
		is_status(LOADING) ?
			HAL_CAN_DeactivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE) :
			HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
		sensor_state.enabled = !is_status(LOADING);
	}
}

bool _recieve_distance(CAN_RxHeaderTypeDef* rx_header, uint8_t* rx_buffer)
{
	bool res = false;
	for (unsigned i = 0; i < __arr_len(sensor_state.sensors); i++) {
		if (rx_header->StdId != SENSOR_FRAME_IDS[i] ||
			rx_buffer[0]     != SENSOR_DISTANCE_FRAME_ID
		) {
			continue;
		}
		if (rx_header->StdId == SENSOR_FRAME_ANGLE) {
		} else {
			sensor_state.sensors[i].value     = ((int16_t)rx_buffer[1] << 8) | (int16_t)rx_buffer[2];
			sensor_state.sensors[i].direction = rx_buffer[3];
		}
		res = true;
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
		break;
	}
	return res;
}

#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC
void _init_s(void)
{
    SCB_DEMCR   |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;

	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);

	sensor_state.need_mode   = SENSOR_MODE_SURFACE;
	sensor_state.curr_mode   = SENSOR_MODE_SURFACE;
	sensor_state.curr_target = 0;
	sensor_state.bigski_id   = 0;

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _idle_s(void)
{
	sensor_state.need_std_id = SENSOR_VALUE_STD_ID;

	_check_stop();

	if (is_status(LOADING)) {
		return;
	}

	if (sensor_state.errors > SENSOR_MAX_ERRORS) {
		fsm_gc_push_event(&sens_fsm, &error_e);
	} else if (
		!sensor_state.initialized ||
		sensor_state.need_mode   != sensor_state.curr_mode ||
		sensor_state.curr_target != get_sensor_mode_target(sensor_state.need_mode) ||
		sensor_state.no_sensor   != !sensor_available()
	) {
		fsm_gc_push_event(&sens_fsm, &change_e);
	} else if (
		sensor_available() &&
		!util_old_timer_wait(&(sensor_state.frame_timer))
	) {
		fsm_gc_push_event(&sens_fsm, &send_e);
	} else if (sensor_state.received) {
		fsm_gc_push_event(&sens_fsm, &recieved_e);
	} else {
		sensor_state.need_std_id = SENSOR_VALUE_STD_ID;
	}

	sensor_state.no_sensor = !sensor_available();

	if (sensor_available()) {
		reset_status(NO_SENSOR);
		reset_status(NO_BIGSKI);
	} else {
		sensor_state.errors = SENSOR_MAX_ERRORS + 1;
		set_status(NO_SENSOR);
		set_status(NO_BIGSKI);
	}
}

void _start_s(void)
{
	static unsigned counter = 0;

	if (!util_old_timer_wait(&sensor_state.timer)) {
		counter = 0;
		fsm_gc_push_event(&sens_fsm, &error_e);
		return;
	}

	if (counter >= __arr_len(start_frames)) {
		sensor_state.no_sensor = true;
		sensor_state.errors    = 0;
		counter                = 0;
		fsm_gc_push_event(&sens_fsm, &success_e);
		return;
	}
	_sensor_send_frame(
		start_frames[counter].std_id,
		start_frames[counter].dlc,
		start_frames[counter].data
	);
	counter++;

	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.errors = 0;
}

void _change_s(void)
{
	switch (sensor_state.need_mode) {
	case SENSOR_MODE_BIGSKI:
		fsm_gc_push_event(&sens_fsm, &bigski1_e);
		break;
	case SENSOR_MODE_SURFACE:
		fsm_gc_push_event(&sens_fsm, &surface_e);
		break;
	case SENSOR_MODE_STRING:
		fsm_gc_push_event(&sens_fsm, &string_e);
		break;
	case SENSOR_MODE_ANGLE:
		fsm_gc_push_event(&sens_fsm, &angle_e);
		break;
	default:
		BEDUG_ASSERT(IS_SENSOR_MODE(sensor_state.need_mode), "Unknown sensor mode");
		Error_Handler();
		break;
	};
}

void _bigski1_s(void)
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
		sensor_state.errors = 0;

		fsm_gc_push_event(&sens_fsm, &bigski2_e);
	} else {
		_sensor_send_frame(request.std_id, request.dlc, request.data);
		util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
		sensor_state.errors = 0;

		fsm_gc_push_event(&sens_fsm, &bigski1_e);
	}
}

void _bigski2_s(void)
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response[] =
		{0x01, 0x0F, BIGSKI_IDS[sensor_state.bigski_id], 0x00, 0x05, 0x00,};

	if (memcmp(response, sensor_state.rx_buffer, __arr_len(response))) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	sensor_state.bigski_id++;
	sensor_state.errors = 0;

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _bigski3_s(void)
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response[] = {0x01, 0x0F, 0x00, 0x00, 0x12, 0x00,};

	if (memcmp(response, sensor_state.rx_buffer, __arr_len(response))) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	sensor_state.errors      = 0;
	sensor_state.bigski_id   = 0;
	sensor_state.curr_mode   = sensor_state.need_mode;
	sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _angle_s(void)
{
	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end1_s(void)
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response1[] =
		{0x01, 0x0F, 0x00, 0x00, 0x19, 0x00,};

	if (memcmp(response1, sensor_state.rx_buffer, __arr_len(response1))) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	int16_t target = -get_sensor_mode_target(sensor_state.need_mode);
	can_frame_t request2 =
		{0x07EC, 0x06, {0x01, 0x0F, 0x00, 0x05, (uint8_t)(target >> 8), (uint8_t)(target)}};

	_sensor_send_frame(request2.std_id, request2.dlc, request2.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.errors = 0;

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end2_s(void)
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response2[] =
		{0x01, 0x0F, 0x00, 0x00, 0x05, 0x00};

	if (memcmp(response2, sensor_state.rx_buffer, __arr_len(response2))) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	can_frame_t request3 =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x03, 0x06}};

	_sensor_send_frame(request3.std_id, request3.dlc, request3.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.errors = 0;

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end3_s(void)
{
	bool recieved = false;

	if (sensor_state.received && sensor_state.rx_header.StdId == SENSOR_SETTINGS_STD_ID) {
		recieved = true;
	} else {
		sensor_state.received = false;
	}

	if (!util_old_timer_wait(&sensor_state.timer)) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	if (!recieved) {
		return;
	}

	uint8_t response3[] =
		{0x01, 0x0F, 0x00, 0x00, 0x03, 0x00};

	if (memcmp(response3, sensor_state.rx_buffer, __arr_len(response3))) {
		fsm_gc_push_event(&sens_fsm, &timeout_e);
		return;
	}

	sensor_state.initialized = true;
	sensor_state.need_std_id = SENSOR_VALUE_STD_ID;
	sensor_state.curr_mode   = sensor_state.need_mode;
	sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);
	sensor_state.errors      = 0;

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void _send_s(void)
{
	if (util_old_timer_wait(&sensor_state.timer)) {
		return;
	}

	uint8_t data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	_sensor_send_frame(0x03F0, 0x08, data);

	fsm_gc_push_event(&sens_fsm, &success_e);
}

void surface_a(void)
{
	can_frame_t surface_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x02,}};

	_sensor_send_frame(surface_request.std_id, surface_request.dlc, surface_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
}

void start_sensor_a(void)
{
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
	sensor_state.errors      = 0;
}

void start_change_a(void)
{
	sensor_state.need_std_id = SENSOR_SETTINGS_STD_ID;
}

void start_idle_a(void) {}

void string_a(void)
{
	can_frame_t string_request =
		{0x07EC, 0x05, {0x01, 0x0F, 0x00, 0x19, 0x01,}};

	_sensor_send_frame(string_request.std_id, string_request.dlc, string_request.data);
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
}

void send_a(void)
{
	sensor_state.need_std_id = SENSOR_VALUE_STD_ID;

	int16_t value = get_sensor_value();
	uint8_t data[8] = {
		(uint8_t)(value >> 8),
		(uint8_t)(value),
		0x00,
		0x0C,
		0xFE,
		0x01,
		0x00,
		0x0B
	};
	_sensor_send_frame(0x0028, 0x08, data);

	util_old_timer_start(&sensor_state.timer, SENSOR_COMMAND_DELAY_MS);
}

void recieve_a(void)
{
	util_old_timer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
	sensor_state.need_std_id = SENSOR_VALUE_STD_ID;

	sensor_state.errors   = 0;
	sensor_state.received = false;
}

void error_a(void)
{
	sensor_state.errors++;
}


