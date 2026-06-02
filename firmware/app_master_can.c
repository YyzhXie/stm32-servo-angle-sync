#include "common/board_config.h"

#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/moving_average.h"

extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan;

static moving_average_t s_adc_filter;
static uint32_t s_next_send_ms;
static uint8_t s_sequence;

void app_master_can_init(void)
{
    moving_average_init(&s_adc_filter, APP_ADC_FILTER_WINDOW);
    s_next_send_ms = 0u;
    s_sequence = 0u;
    app_status_led_set_error(0u);

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        app_status_led_set_error(1u);
    }
}

void app_master_can_task(uint32_t now_ms)
{
    if ((int32_t)(now_ms - s_next_send_ms) < 0) {
        return;
    }
    s_next_send_ms = now_ms + APP_CAN_SEND_PERIOD_MS;

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 5u) != HAL_OK) {
        app_status_led_set_error(1u);
        HAL_ADC_Stop(&hadc1);
        return;
    }

    const uint16_t raw_adc = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    const uint16_t filtered_adc = moving_average_push(&s_adc_filter, raw_adc);
    const uint16_t angle_deg10 = angle_from_adc(filtered_adc);

    uint8_t payload[ANGLE_CAN_DLC];
    angle_can_encode_payload(s_sequence++, angle_deg10, payload);

    CAN_TxHeaderTypeDef header;
    header.StdId = ANGLE_CAN_STD_ID;
    header.ExtId = 0u;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = ANGLE_CAN_DLC;
    header.TransmitGlobalTime = DISABLE;

    uint32_t mailbox;
    if (HAL_CAN_AddTxMessage(&hcan, &header, payload, &mailbox) == HAL_OK) {
        HAL_GPIO_TogglePin(APP_STATUS_LED_GPIO_PORT, APP_STATUS_LED_PIN);
    } else {
        app_status_led_set_error(1u);
    }
}
