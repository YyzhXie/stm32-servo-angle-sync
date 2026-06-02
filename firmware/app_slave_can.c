#include "common/board_config.h"

#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/angle_receiver.h"

extern CAN_HandleTypeDef hcan;
extern TIM_HandleTypeDef htim3;

static angle_receiver_t s_receiver;

static void servo_apply_angle(uint16_t angle_deg10)
{
    app_servo_write_us(&htim3, servo_pulse_us_from_angle(angle_deg10));
}

static void configure_can_filter(void)
{
    CAN_FilterTypeDef filter;

    filter.FilterBank = 0u;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = (uint16_t)(ANGLE_CAN_STD_ID << 5u);
    filter.FilterIdLow = 0x0000u;
    filter.FilterMaskIdHigh = 0xFFE0u;
    filter.FilterMaskIdLow = 0x0004u;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14u;

    HAL_CAN_ConfigFilter(&hcan, &filter);
}

void app_slave_can_init(void)
{
    angle_receiver_init(&s_receiver, APP_RX_TIMEOUT_MS, SERVO_SYNC_ANGLE_SAFE_DEG10);

    HAL_TIM_PWM_Start(&htim3, APP_SERVO_TIM_CHANNEL);
    servo_apply_angle(SERVO_SYNC_ANGLE_SAFE_DEG10);
    app_status_led_set_error(0u);

    configure_can_filter();
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void app_slave_can_task(uint32_t now_ms)
{
    const uint16_t angle = angle_receiver_poll(&s_receiver, now_ms);
    servo_apply_angle(angle);
    app_status_led_set_error(s_receiver.timed_out ? 1u : 0u);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_arg)
{
    if (hcan_arg == &hcan) {
        uint8_t payload[8] = {0u};
        CAN_RxHeaderTypeDef header;
        angle_frame_t frame;

        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &header, payload) == HAL_OK &&
            header.IDE == CAN_ID_STD &&
            header.StdId == ANGLE_CAN_STD_ID &&
            angle_can_decode_payload(payload, header.DLC, &frame)) {
            angle_receiver_accept(&s_receiver, frame.angle_deg10, HAL_GetTick());
            servo_apply_angle(frame.angle_deg10);
        }
    }
}
