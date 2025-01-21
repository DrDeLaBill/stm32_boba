/* Copyright © 2024 Georgy E. All rights reserved. */

#include "sensor.h"

#include <string.h>

#include "glog.h"
#include "main.h"
#include "soul.h"
#include "gutils.h"
#include "fsm_gc.h"
#include "gsystem.h"
#include "hal_defs.h"
#include "settings.h"
#include "circle_buf_gc.h"


#define SENSOR_BUFFER_COUNT        (15)
#define SENSOR_DATA_MAX_SIZE       (8)
#define SENSOR_FRAME_DELAY_MS      (400)
#define SENSOR_COMMAND_DELAY_MS    (15)
#define SENSOR_CAN_DELAY_MS        (200)
#define SENSOR_MAX_ERRORS          (100)
#define SENSOR_CONNECTION_DELAY_MS (300)

#define SENSOR_FRAME_ANGLE_IDX     (3)

#define SENSOR_MODE_NONE           (0)

#define SENSOR_DISTANCE_COUNT      (3)

#define SENSOR_SUB_SENSORS_COUNT   (5)
#define SENSOR_SUB_MAIN_ERROR      (0x7FFF)
#define SENSOR_SUB_CHECK_MS        (1500)
#define SENSOR_SUB_ERRORS_MAX      (10)


typedef struct _sensor_dist_t {
    int16_t          value;
    gtimer_t         connection_timer;
    STRING_DIRECTION direction;
    uint8_t          error_cnt;
    int16_t          sub_main;
    int16_t          sub_values[SENSOR_SUB_SENSORS_COUNT];
} sensor_dist_t;

typedef struct _sensor_angle_t {
    int16_t          value;
    gtimer_t         connection_timer;
} sensor_angle_t;

typedef struct _sensor_state_t {
    bool                initialized;
    bool                enabled;

    sensor_dist_t       sens_2A7;
    sensor_dist_t       sens_2A8;
    sensor_dist_t       sens_2AB;
    sensor_angle_t      sens_angle;
    gtimer_t            sens_sub_check;

    SENSOR_MODE         curr_mode;
    SENSOR_MODE         need_mode;
    int16_t             curr_target;

    uint32_t            last_error;
    unsigned            errors;
    gtimer_t            timer;
    gtimer_t            frame_timer;

    uint8_t             bigski_id;
} sensor_state_t;

typedef struct _can_frame_t {
	CAN_RxHeaderTypeDef header;
	uint8_t data[SENSOR_DATA_MAX_SIZE];
} can_frame_t;


static int16_t get_sensor2A7_value();
static int16_t get_sensor2A8_value();
static int16_t get_sensor2AB_value();
static int16_t get_sensor_average();
static int16_t get_sensor_angle();

static void _can_enable_it();
static void _can_disable_it();
static void _can_enable_tick();
static void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data);
static can_frame_t get_frame(uint16_t std_id);


extern CAN_HandleTypeDef hcan1;
#define SENS_CAN         (hcan1)


static const uint8_t BIGSKI_IDS[] = {0x00, 0x02, 0x04};

sensor_state_t sensor_state = {
    .initialized       = false,
	.enabled           = false,

	.sens_2A7          = {0},
	.sens_2A8          = {0},
	.sens_2AB          = {0},
	.sens_angle        = {0},
	.sens_sub_check    = {0},

    .curr_mode         = SENSOR_MODE_SURFACE,
    .need_mode         = SENSOR_MODE_SURFACE,
	.curr_target       = 0,

	.errors            = 0,
	.timer             = {0},
	.frame_timer       = {0},

    .bigski_id         = 0,
};
static can_frame_t can_buffer[SENSOR_BUFFER_COUNT] = {0};
static circle_buf_gc_t can_circle_buffer = {0};


static void _init_s(void);
static void _idle_s(void);
static void _no_sensor_s(void);
static void _error_s(void);
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
static void no_sensor_a(void);
static void sub_check_a(void);
static void surface_a(void);
static void string_a(void);
static void send_a(void);
static void recieve_a(void);
static void error_idle_a(void);


FSM_GC_CREATE(sens_fsm)

FSM_GC_CREATE_EVENT(success_e,   0)
FSM_GC_CREATE_EVENT(timeout_e,   0)
FSM_GC_CREATE_EVENT(recieved_e,  0)
FSM_GC_CREATE_EVENT(sub_check_e, 0)
FSM_GC_CREATE_EVENT(bigski1_e,   0)
FSM_GC_CREATE_EVENT(bigski2_e,   0)
FSM_GC_CREATE_EVENT(surface_e,   0)
FSM_GC_CREATE_EVENT(string_e,    0)
FSM_GC_CREATE_EVENT(angle_e,     0)
FSM_GC_CREATE_EVENT(send_e,      0)
FSM_GC_CREATE_EVENT(change_e,    1)
FSM_GC_CREATE_EVENT(error_e,     2)

FSM_GC_CREATE_STATE(init_s,      _init_s)
FSM_GC_CREATE_STATE(idle_s,      _idle_s)
FSM_GC_CREATE_STATE(no_sensor_s, _no_sensor_s)
FSM_GC_CREATE_STATE(error_s,     _error_s)
FSM_GC_CREATE_STATE(change_s,    _change_s)
FSM_GC_CREATE_STATE(bigski1_s,   _bigski1_s)
FSM_GC_CREATE_STATE(bigski2_s,   _bigski2_s)
FSM_GC_CREATE_STATE(bigski3_s,   _bigski3_s)
FSM_GC_CREATE_STATE(angle_s,     _angle_s)
FSM_GC_CREATE_STATE(end1_s,      _end1_s)
FSM_GC_CREATE_STATE(end2_s,      _end2_s)
FSM_GC_CREATE_STATE(end3_s,      _end3_s)
FSM_GC_CREATE_STATE(send_s,      _send_s)

FSM_GC_CREATE_TABLE(
    sens_fsm_table,
    {&init_s,      &success_e,   &idle_s,      start_sensor_a},

    {&idle_s,      &send_e,      &send_s,      send_a},
    {&idle_s,      &change_e,    &change_s,    start_change_a},
    {&idle_s,      &recieved_e,  &idle_s,      recieve_a},
    {&idle_s,      &sub_check_e, &idle_s,      sub_check_a},
    {&idle_s,      &error_e,     &no_sensor_s, no_sensor_a},

    {&no_sensor_s, &success_e,   &change_s,    start_change_a},
    {&no_sensor_s, &sub_check_e, &no_sensor_s, sub_check_a},

    {&error_s,     &success_e,   &no_sensor_s, no_sensor_a},

    {&change_s,    &bigski1_e,   &bigski1_s,   NULL},
    {&change_s,    &surface_e,   &end1_s,      surface_a},
    {&change_s,    &string_e,    &end1_s,      string_a},
    {&change_s,    &angle_e,     &angle_s,     NULL},

    {&bigski1_s,   &bigski1_e,   &bigski2_s,   NULL},
    {&bigski1_s,   &bigski2_e,   &bigski3_s,   NULL},
    {&bigski2_s,   &timeout_e,   &idle_s,      error_idle_a},
    {&bigski2_s,   &success_e,   &bigski1_s,   NULL},
    {&bigski3_s,   &timeout_e,   &idle_s,      error_idle_a},
    {&bigski3_s,   &success_e,   &idle_s,      start_idle_a},

    {&angle_s,     &success_e,   &idle_s,      start_idle_a},

    {&end1_s,      &timeout_e,   &idle_s,      error_idle_a},
    {&end1_s,      &success_e,   &end2_s,      NULL},
    {&end2_s,      &timeout_e,   &idle_s,      error_idle_a},
    {&end2_s,      &success_e,   &end3_s,      NULL},
    {&end3_s,      &timeout_e,   &idle_s,      error_idle_a},
    {&end3_s,      &success_e,   &idle_s,      start_idle_a},

    {&send_s,      &success_e,   &idle_s,      start_idle_a},
)

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	can_frame_t tmp_buf = {0};
	if (hcan->Instance == SENS_CAN.Instance) {
		if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &tmp_buf.header, tmp_buf.data) == HAL_OK) {
			reset_status(CAN_FAULT);
			circle_buf_gc_push_back(&can_circle_buffer, (uint8_t*)&tmp_buf);
		} else {
			set_status(CAN_FAULT);
		}
	}
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    sensor_state.last_error = HAL_CAN_GetError(hcan);
    HAL_CAN_ResetError(hcan);
}

void sensor_tick()
{
    if (!sens_fsm._initialized) {
    	circle_buf_gc_init(
			&can_circle_buffer,
			(uint8_t*)&can_buffer,
			sizeof(can_frame_t),
			__arr_len(can_buffer)
		);
        fsm_gc_init(&sens_fsm, sens_fsm_table, __arr_len(sens_fsm_table));
    }
    fsm_gc_process(&sens_fsm);
}

bool sensor_available()
{
    switch (get_sensor_target_mode()) {
    case SENSOR_MODE_ANGLE:
        return sensor_angle_available();
    case SENSOR_MODE_BIGSKI:
        return sensor_2A7_available();
    case SENSOR_MODE_SURFACE:
    case SENSOR_MODE_STRING:
        return sensor_2AB_available() || sensor_2A7_available() || sensor_2A8_available();
    default:
        Error_Handler();
        return false;
    }
}

int16_t get_sensor2AB_value()
{
    return sensor_state.sens_2AB.value;
}

int16_t get_sensor2A7_value()
{
    return sensor_state.sens_2A7.value;
}

int16_t get_sensor2A8_value()
{
    return sensor_state.sens_2A8.value;
}

int16_t get_sensor_average()
{
    return (get_sensor2AB_value() + get_sensor2A7_value() + get_sensor2A8_value())
           / SENSOR_DISTANCE_COUNT;
}

int16_t get_sensor_angle()
{
    return sensor_state.sens_angle.value;
}

int16_t get_sensor_value()
{
    return get_sensor_mode_value(sensor_state.curr_mode);
}

int16_t get_sensor_mode_value(SENSOR_MODE mode)
{
    switch (mode) {
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
    sensor_state.initialized = false;
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
    sensor_state.initialized = false;
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

bool sensor_2AB_available()
{
    return gtimer_wait(&sensor_state.sens_2AB.connection_timer) &&
           sensor_state.sens_2AB.error_cnt < SENSOR_SUB_ERRORS_MAX;
}

bool sensor_2A7_available()
{
    return gtimer_wait(&sensor_state.sens_2A7.connection_timer) &&
           sensor_state.sens_2A7.error_cnt < SENSOR_SUB_ERRORS_MAX;
}

bool sensor_2A8_available()
{
    return gtimer_wait(&sensor_state.sens_2A8.connection_timer) &&
		   sensor_state.sens_2A8.error_cnt < SENSOR_SUB_ERRORS_MAX;
}

bool sensor_distance_available()
{
    return sensor_2AB_available() || sensor_2A7_available() || sensor_2A8_available();
}

bool sensor_angle_available()
{
    return gtimer_wait(&sensor_state.sens_angle.connection_timer);
}

void set_sensor_mode(SENSOR_MODE mode)
{
    sensor_state.initialized = false;

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
        return sensor_state.sens_2A7.direction;
    }
    return STR_MIDDLE;
}

uint8_t get_sensor_mode_sensitive()
{
    switch(get_sensor_mode()) {
    case SENSOR_MODE_SURFACE:
        return SENSITIVITY[settings.surface_snstv];
    case SENSOR_MODE_STRING:
        return SENSITIVITY[settings.string_snstv];
    case SENSOR_MODE_BIGSKI:
        return SENSITIVITY[settings.bigski_snstv];
    case SENSOR_MODE_ANGLE:
        return SENSITIVITY[settings.angle_snstv];
    default:
        BEDUG_ASSERT(false, "Unknown mode");
        Error_Handler();
        return 0;
    }
}

uint8_t get_sensor_mode_sensitive_index()
{
    switch(get_sensor_mode()) {
    case SENSOR_MODE_SURFACE:
        return settings.surface_snstv;
    case SENSOR_MODE_STRING:
        return settings.string_snstv;
    case SENSOR_MODE_BIGSKI:
        return settings.bigski_snstv;
    case SENSOR_MODE_ANGLE:
        return settings.angle_snstv;
    default:
        BEDUG_ASSERT(false, "Unknown mode");
        Error_Handler();
        return 0;
    }
}

void set_sensor_mode_sensitive(uint8_t sensitivity_idx)
{
    if (sensitivity_idx > __arr_len(SENSITIVITY)) {
        BEDUG_ASSERT(false, "Unacceptable SENSITIVITY");
        return;
    }
    switch(get_sensor_mode()) {
    case SENSOR_MODE_SURFACE:
        settings.surface_snstv = sensitivity_idx;
        break;
    case SENSOR_MODE_STRING:
        settings.string_snstv = sensitivity_idx;
        break;
    case SENSOR_MODE_BIGSKI:
        settings.bigski_snstv = sensitivity_idx;
        break;
    case SENSOR_MODE_ANGLE:
        settings.angle_snstv = sensitivity_idx;
        break;
    default:
        BEDUG_ASSERT(false, "Unknown mode");
        Error_Handler();
        return;
    }
}

uint8_t get_sensor_mode_regulation_mm()
{
    switch(get_sensor_mode()) {
    case SENSOR_MODE_SURFACE:
        return settings.surface_reg_wind_x10;
    case SENSOR_MODE_STRING:
        return settings.string_reg_wind_x10;
    case SENSOR_MODE_BIGSKI:
        return settings.bigski_reg_wind_x10;
    case SENSOR_MODE_ANGLE:
    default:
        BEDUG_ASSERT(false, "Mode regulation get error");
        Error_Handler();
        return 0;
    }
}

void set_sensor_mode_regulation(uint8_t regulation_mm)
{
    if (regulation_mm > SETTINGS_REGULATION_MM_MAX) {
        BEDUG_ASSERT(false, "Unacceptable regulation window");
        return;
    }
    switch(get_sensor_mode()) {
    case SENSOR_MODE_SURFACE:
        settings.surface_reg_wind_x10 = regulation_mm;
        break;
    case SENSOR_MODE_STRING:
        settings.string_reg_wind_x10 = regulation_mm;
        break;
    case SENSOR_MODE_BIGSKI:
        settings.bigski_reg_wind_x10 = regulation_mm;
        break;
    case SENSOR_MODE_ANGLE:
    default:
        BEDUG_ASSERT(false, "Mode regulation set error");
        Error_Handler();
        return;
    }
}

void _sensor_send_frame(const uint32_t std_id, const uint32_t dlc, const uint8_t* data)
{
    static uint32_t            tx_mailbox = 0;
    static CAN_TxHeaderTypeDef tx_header = {0};
    static uint8_t             tx_buffer[SENSOR_DATA_MAX_SIZE] = {0};
    tx_mailbox = 0;
    memset((void*)&tx_header, 0, sizeof(tx_header));
    memset((void*)tx_buffer, 0, sizeof(tx_buffer));

    tx_header.RTR                = CAN_RTR_DATA;
    tx_header.IDE                = CAN_ID_STD;
    tx_header.TransmitGlobalTime = DISABLE;
    tx_mailbox                   = 0;
    tx_header.StdId              = std_id;
    tx_header.DLC                = dlc;
    memset(tx_buffer, 0 , sizeof(tx_buffer));
    memcpy(tx_buffer, data, __min(sizeof(tx_buffer), dlc));
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(
	   &SENS_CAN,
	   &tx_header,
	   tx_buffer,
	   &tx_mailbox
	);
    if (status != HAL_OK) {
        printTagLog(
			"SENS",
			"CAN send status=%u error=0x%08X std_id=%lu len=%lu",
			status,
			sensor_state.last_error,
			std_id,
			dlc
		);
        sensor_state.last_error = 0;
        HAL_CAN_AbortTxRequest(&SENS_CAN, tx_mailbox);
    }
}

can_frame_t get_frame(uint16_t std_id)
{
	if (sensor_state.enabled) {
		_can_disable_it();
	}
	bool found = false;
	can_frame_t frame = {0};
	for (unsigned i = 0; i < circle_buf_gc_count(&can_circle_buffer); i++) {
		can_frame_t* tmp = (can_frame_t*)circle_buf_gc_front(&can_circle_buffer);
		if (!tmp) {
			break;
		}
		frame = *tmp;
		circle_buf_gc_pop_front(&can_circle_buffer);
		if (frame.header.StdId == std_id) {
			found = true;
			break;
		}
		circle_buf_gc_push_back(&can_circle_buffer, (uint8_t*)&frame);
	}
	if (!found) {
		memset((uint8_t*)&frame, 0, sizeof(frame));
	}
	if (sensor_state.enabled) {
		_can_enable_it();
	}
	return frame;
}

void _can_enable_it()
{
	HAL_CAN_ActivateNotification(&SENS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
}

void _can_disable_it()
{
	HAL_CAN_DeactivateNotification(&SENS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);
}

void _can_enable_tick()
{
    if (sensor_state.enabled != is_system_ready()) {
        is_system_ready() ?
            _can_enable_it() :
            _can_disable_it();
        sensor_state.enabled = is_system_ready();
    }
}

#define DWT_CYCCNT  (*(volatile unsigned long *)0xE0001004)
#define DWT_CONTROL (*(volatile unsigned long *)0xE0001000)
#define SCB_DEMCR   (*(volatile unsigned long *)0xE000EDFC)
void _init_s(void)
{
    HAL_CAN_Start(&SENS_CAN);
    HAL_CAN_ActivateNotification(&SENS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);

    sensor_state.need_mode   = SENSOR_MODE_SURFACE;
    sensor_state.curr_mode   = SENSOR_MODE_SURFACE;
    sensor_state.curr_target = 0;
    sensor_state.bigski_id   = 0;

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _idle_s(void)
{
    _can_enable_tick();

    if (!is_system_ready()) {
        return;
    }

    if (sensor_state.errors > SENSOR_MAX_ERRORS) {
        fsm_gc_push_event(&sens_fsm, &error_e);
        return;
    }

    if (!sensor_state.initialized || is_status(NEED_RESET_SENSOR)) {
        fsm_gc_push_event(&sens_fsm, &change_e);
        return;
    }

    if (circle_buf_gc_count(&can_circle_buffer)) {
        fsm_gc_push_event(&sens_fsm, &recieved_e);
        return;
    }

    if (!sensor_available()) {
        fsm_gc_push_event(&sens_fsm, &error_e);
        return;
    }

    if (!gtimer_wait(&sensor_state.sens_sub_check)) {
    	fsm_gc_push_event(&sens_fsm, &sub_check_e);
    	return;
    }

    if (!gtimer_wait(&sensor_state.frame_timer)) {
        fsm_gc_push_event(&sens_fsm, &send_e);
        return;
    }
}

void _no_sensor_s(void)
{
    _can_enable_tick();

    if (!gtimer_wait(&sensor_state.sens_sub_check)) {
    	fsm_gc_push_event(&sens_fsm, &sub_check_e);
    }

    if (circle_buf_gc_count(&can_circle_buffer)) {
        fsm_gc_push_event(&sens_fsm, &success_e);
    }
}

void _error_s(void)
{
    fsm_gc_push_event(&sens_fsm, &success_e);
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
    can_frame_t request = {
		{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x06},
		{0x01, 0x0F, BIGSKI_IDS[sensor_state.bigski_id], 0x05, (uint8_t)(value >> 8), (uint8_t)value}
    };

    if (sensor_state.bigski_id >= __arr_len(BIGSKI_IDS)) {
        sensor_state.bigski_id = 0;
        can_frame_t mode = {
        	{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x05},
			{0x01, 0x0F, 0x00, 0x12, 0x00,}
        };
        _sensor_send_frame(mode.header.StdId, mode.header.DLC, mode.data);
        gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
        sensor_state.errors = 0;

        fsm_gc_push_event(&sens_fsm, &bigski2_e);
    } else {
        _sensor_send_frame(request.header.StdId, request.header.DLC, request.data);
        gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
        sensor_state.errors = 0;

        fsm_gc_push_event(&sens_fsm, &bigski1_e);
    }
}

void _bigski2_s(void)
{
    if (!gtimer_wait(&sensor_state.timer)) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t frame = get_frame(LINE_SENSOR_SETTINGS);
    if (frame.header.StdId != LINE_SENSOR_SETTINGS) {
        return;
    }

    uint8_t response[] =
        {0x01, 0x0F, BIGSKI_IDS[sensor_state.bigski_id], 0x00, 0x05, 0x00,};

    if (memcmp(response, frame.data, __arr_len(response))) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    sensor_state.bigski_id++;
    sensor_state.errors = 0;

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _bigski3_s(void)
{
    if (!gtimer_wait(&sensor_state.timer)) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t frame = get_frame(LINE_SENSOR_SETTINGS);
    if (frame.header.StdId != LINE_SENSOR_SETTINGS) {
        return;
    }

    uint8_t response[] = {0x01, 0x0F, 0x00, 0x00, 0x12, 0x00,};

    if (memcmp(response, frame.data, __arr_len(response))) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    sensor_state.errors      = 0;
    sensor_state.bigski_id   = 0;
    sensor_state.curr_mode   = sensor_state.need_mode;
    sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);

	reset_status(NEED_RESET_SENSOR);

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _angle_s(void)
{
    sensor_state.initialized = true;
    sensor_state.curr_mode   = sensor_state.need_mode;
    sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);
    sensor_state.errors      = 0;

	reset_status(NEED_RESET_SENSOR);

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end1_s(void)
{
    if (!gtimer_wait(&sensor_state.timer)) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t frame = get_frame(LINE_SENSOR_SETTINGS);
    if (frame.header.StdId != LINE_SENSOR_SETTINGS) {
        return;
    }

    uint8_t response1[] =
        {0x01, 0x0F, 0x00, 0x00, 0x19, 0x00,};

    if (memcmp(response1, frame.data, __arr_len(response1))) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    int16_t target = -get_sensor_mode_target(sensor_state.need_mode);
    can_frame_t request2 = {
		{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x06},
		{0x01, 0x0F, 0x00, 0x05, (uint8_t)(target >> 8), (uint8_t)(target)}
    };

    _sensor_send_frame(request2.header.StdId, request2.header.DLC, request2.data);
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
    sensor_state.errors = 0;

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end2_s(void)
{
    if (!gtimer_wait(&sensor_state.timer)) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t frame = get_frame(LINE_SENSOR_SETTINGS);
    if (frame.header.StdId != LINE_SENSOR_SETTINGS) {
        return;
    }

    uint8_t response2[] =
        {0x01, 0x0F, 0x00, 0x00, 0x05, 0x00};

    if (memcmp(response2, frame.data, __arr_len(response2))) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t request3 = {
		{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x05},
		{0x01, 0x0F, 0x00, 0x03, 0x06}
    };

    _sensor_send_frame(request3.header.StdId, request3.header.DLC, request3.data);
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
    sensor_state.errors = 0;

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _end3_s(void)
{
    if (!gtimer_wait(&sensor_state.timer)) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    can_frame_t frame = get_frame(LINE_SENSOR_SETTINGS);
    if (frame.header.StdId != LINE_SENSOR_SETTINGS) {
        return;
    }

    uint8_t response3[] =
        {0x01, 0x0F, 0x00, 0x00, 0x03, 0x00};

    if (memcmp(response3, frame.data, __arr_len(response3))) {
        fsm_gc_push_event(&sens_fsm, &timeout_e);
        return;
    }

    sensor_state.initialized = true;
    sensor_state.curr_mode   = sensor_state.need_mode;
    sensor_state.curr_target = get_sensor_mode_target(sensor_state.need_mode);
    sensor_state.errors      = 0;

	reset_status(NEED_RESET_SENSOR);

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void _send_s(void)
{
    if (gtimer_wait(&sensor_state.timer)) {
        return;
    }

    uint8_t line_data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t angle_data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t* data = NULL;
    CAN_STD_ID std_id = NO_STD_ID;
    switch (get_sensor_mode()) {
    case SENSOR_MODE_ANGLE:
        std_id = ANGLE_CONTROL_STATUS;
        data = angle_data;
        break;
    case SENSOR_MODE_SURFACE:
    case SENSOR_MODE_STRING:
    case SENSOR_MODE_BIGSKI:
        std_id = LINE_CONTROL_STATUS;
        data = line_data;
        break;
    default:
        set_error(INTERNAL_ERROR);
        BEDUG_ASSERT(false, "Unknown sensor mode");
        Error_Handler();
        return;
    }
    _sensor_send_frame(std_id, 0x08, data);

    fsm_gc_push_event(&sens_fsm, &success_e);
}

void surface_a(void)
{
    can_frame_t surface_request = {
		{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x05},
		{0x01, 0x0F, 0x00, 0x19, 0x02,}
    };
    _sensor_send_frame(surface_request.header.StdId, surface_request.header.DLC, surface_request.data);
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
}

void string_a(void)
{
    can_frame_t string_request = {
		{LINE_CONTROL_SETTINGS, 0, 0, 0, 0x05},
		{0x01, 0x0F, 0x00, 0x19, 0x01,}
    };
    _sensor_send_frame(string_request.header.StdId, string_request.header.DLC, string_request.data);
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
}

void start_sensor_a(void)
{
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
    gtimer_start(&sensor_state.sens_sub_check, SENSOR_SUB_CHECK_MS);

    sensor_state.errors      = 0;

    fsm_gc_clear(&sens_fsm);
}

void start_change_a(void)
{
    fsm_gc_clear(&sens_fsm);
}

void start_idle_a(void)
{
    fsm_gc_clear(&sens_fsm);
}

void error_idle_a(void)
{
    gtimer_start(&sensor_state.sens_sub_check, SENSOR_SUB_CHECK_MS);

    sensor_state.errors++;

    fsm_gc_clear(&sens_fsm);
}

void no_sensor_a(void)
{
    fsm_gc_clear(&sens_fsm);
}

void send_a(void)
{
    int16_t value = get_sensor_value();
    uint8_t line_data[8] = {
        (uint8_t)(value >> 8),
        (uint8_t)(value),
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,
        0x0B
    };
    uint8_t angle_data[8] = {
        (uint8_t)(value >> 8),
        (uint8_t)(value),
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,
        0x0A
    };

    uint8_t* data = NULL;
    CAN_STD_ID std_id;
    switch (get_sensor_mode()) {
    case SENSOR_MODE_ANGLE:
        std_id = ANGLE_CONTROL_VALUE;
        data = angle_data;
        break;
    case SENSOR_MODE_SURFACE:
    case SENSOR_MODE_STRING:
    case SENSOR_MODE_BIGSKI:
        std_id = LINE_CONTROL_VALUE;
        data = line_data;
        break;
    default:
        set_error(INTERNAL_ERROR);
        BEDUG_ASSERT(false, "Unknown sensor mode");
        Error_Handler();
        return;
    }
    _sensor_send_frame(std_id, 0x08, data);

    gtimer_start(&sensor_state.timer, SENSOR_COMMAND_DELAY_MS);
}

void recieve_a(void)
{
    gtimer_start(&sensor_state.timer, SENSOR_CAN_DELAY_MS);
    fsm_gc_clear(&sens_fsm);

    sensor_state.errors = 0;

    while (circle_buf_gc_count(&can_circle_buffer)) {
    	can_frame_t frame = *(can_frame_t*)circle_buf_gc_pop_front(&can_circle_buffer);
        sensor_dist_t* sensor = NULL;
        switch (frame.header.StdId) {
        case LINE_SENSOR_1_VALUE:
        	sensor = &sensor_state.sens_2A8;
            break;
        case LINE_SENSOR_C_VALUE:
        	sensor = &sensor_state.sens_2A7;
            break;
        case LINE_SENSOR_3_VALUE:
        	sensor = &sensor_state.sens_2AB;
            break;
        case ANGLE_SENSOR_VALUE:
            sensor_state.sens_angle.value = (
                ((int16_t)frame.data[1] << 8) |
                (int16_t)frame.data[2]
            );
            gtimer_start(&sensor_state.sens_angle.connection_timer, SENSOR_CONNECTION_DELAY_MS);
            continue;
        default:
        	continue;
        }

        switch (frame.data[0]) {
        case RELATIVE_VALUE:
        	sensor->value = (
                ((int16_t)frame.data[1] << 8) |
                (int16_t)frame.data[2]
            );
        	sensor->direction = frame.data[3];
            gtimer_start(&sensor->connection_timer, SENSOR_CONNECTION_DELAY_MS);
        	break;
        case LINE_ABSOLUTE1_VALUE:
        	sensor->sub_main = (
    			((int16_t)frame.data[1] << 8) |
    			(int16_t)frame.data[2]
    		);
        	sensor->sub_values[0] = (
    			((int16_t)frame.data[3] << 8) |
    			(int16_t)frame.data[4]
    		);
        	sensor->sub_values[1] = (
    			((int16_t)frame.data[5] << 8) |
    			(int16_t)frame.data[6]
    		);
        	break;
        case LINE_ABSOLUTE2_VALUE:
        	sensor->sub_values[2] = (
    			((int16_t)frame.data[1] << 8) |
    			(int16_t)frame.data[2]
    		);
        	sensor->sub_values[3] = (
    			((int16_t)frame.data[3] << 8) |
    			(int16_t)frame.data[4]
    		);
        	sensor->sub_values[4] = (
    			((int16_t)frame.data[5] << 8) |
    			(int16_t)frame.data[6]
    		);
        	break;
        default:
        	break;
        }
    }
}

void sub_check_a(void)
{
	sensor_dist_t* sensors[] = {
		&sensor_state.sens_2A8,
		&sensor_state.sens_2A7,
		&sensor_state.sens_2AB
	};

	uint8_t regulation = get_sensor_mode_regulation_mm();
	for (unsigned i = 0; i < __arr_len(sensors); i++) {
		int16_t min = 0x7FFF;
		int16_t max = 0xFFFF;
		for (unsigned j = 0; j < __arr_len(sensors[i]->sub_values); j++) {
			if (sensors[i]->sub_values[j] < min) {
				min = sensors[i]->sub_values[j];
			}
			if (sensors[i]->sub_values[j] > max) {
				max = sensors[i]->sub_values[j];
			}
		}
		if (sensors[i]->sub_main == SENSOR_SUB_MAIN_ERROR ||
			(regulation && __abs_dif(min, max) > regulation)
		) {
			sensors[i]->error_cnt++;
		} else {
			sensors[i]->error_cnt = 0;
		}
		if (sensors[i]->error_cnt >= SENSOR_SUB_ERRORS_MAX) {
			sensors[i]->error_cnt = SENSOR_SUB_ERRORS_MAX;
			set_status(SENSOR_REGULATE_FAULT);
		} else {
			reset_status(SENSOR_REGULATE_FAULT);
		}
	}

    gtimer_start(&sensor_state.sens_sub_check, SENSOR_SUB_CHECK_MS);
}
