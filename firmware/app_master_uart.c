#include "common/board_config.h"

#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/moving_average.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

static moving_average_t s_adc_filter;
static uint32_t s_next_send_ms;
static uint8_t s_sequence;

void app_master_uart_init(void)
{
    moving_average_init(&s_adc_filter, APP_ADC_FILTER_WINDOW);
    s_next_send_ms = 0u;
    s_sequence = 0u;
    app_status_led_set_error(0u);
}

void app_master_uart_task(uint32_t now_ms)
{
    if ((int32_t)(now_ms - s_next_send_ms) < 0) {
        return;
    }
    s_next_send_ms = now_ms + APP_UART_SEND_PERIOD_MS;

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

    uint8_t frame[ANGLE_FRAME_SIZE];
    const size_t length = angle_frame_encode(s_sequence++, angle_deg10, frame);

    if (HAL_UART_Transmit(&huart1, frame, (uint16_t)length, 10u) == HAL_OK) {
        HAL_GPIO_TogglePin(APP_STATUS_LED_GPIO_PORT, APP_STATUS_LED_PIN);
    } else {
        app_status_led_set_error(1u);
    }
}
