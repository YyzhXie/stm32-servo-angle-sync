#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32f1xx_hal.h"

#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"

#define APP_UART_BAUDRATE 9600u
#define APP_UART_SEND_PERIOD_MS 50u
#define APP_CAN_SEND_PERIOD_MS 50u
#define APP_ADC_FILTER_WINDOW 8u
#define APP_RX_TIMEOUT_MS 500u

#define APP_STATUS_LED_GPIO_PORT GPIOC
#define APP_STATUS_LED_PIN GPIO_PIN_13

#define APP_SERVO_TIM_CHANNEL TIM_CHANNEL_1

/*
 * With TIM3 configured to a 1 MHz counter, CCR value equals pulse width in us.
 * This is why servo_pulse_us_from_angle() can be written directly to CCR.
 */
static inline void app_servo_write_us(TIM_HandleTypeDef *htim, uint16_t pulse_us)
{
    __HAL_TIM_SET_COMPARE(htim, APP_SERVO_TIM_CHANNEL, pulse_us);
}

static inline void app_status_led_set_error(uint8_t error)
{
    HAL_GPIO_WritePin(APP_STATUS_LED_GPIO_PORT,
                      APP_STATUS_LED_PIN,
                      error ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

#endif
