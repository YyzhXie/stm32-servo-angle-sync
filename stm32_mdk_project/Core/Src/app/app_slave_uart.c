#include "common/board_config.h"

#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/angle_receiver.h"

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim3;

static angle_frame_decoder_t s_decoder;
static angle_receiver_t s_receiver;
static uint8_t s_rx_byte;

static void servo_apply_angle(uint16_t angle_deg10)
{
    /* The timer counter runs at 1 MHz, so pulse_us maps directly to CCR. */
    app_servo_write_us(&htim3, servo_pulse_us_from_angle(angle_deg10));
}

void app_slave_uart_init(void)
{
    angle_frame_decoder_init(&s_decoder);
    angle_receiver_init(&s_receiver, APP_RX_TIMEOUT_MS, SERVO_SYNC_ANGLE_SAFE_DEG10);

    HAL_TIM_PWM_Start(&htim3, APP_SERVO_TIM_CHANNEL);
    servo_apply_angle(SERVO_SYNC_ANGLE_SAFE_DEG10);
    app_status_led_set_error(0u);

    /*
     * Reception is byte-by-byte so the frame decoder can resynchronize quickly
     * after wiring noise, reset timing, or a partially received frame.
     */
    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1u);
}

void app_slave_uart_task(uint32_t now_ms)
{
    /* Polling only handles timeout safety; byte reception itself is interrupt-driven. */
    const uint16_t angle = angle_receiver_poll(&s_receiver, now_ms);
    servo_apply_angle(angle);
    app_status_led_set_error(s_receiver.timed_out ? 1u : 0u);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) {
        angle_frame_t frame;
        if (angle_frame_decoder_push(&s_decoder, s_rx_byte, &frame) ==
            ANGLE_FRAME_OK) {
            /* Only CRC-checked frames update the receiver and servo command. */
            angle_receiver_accept(&s_receiver, frame.angle_deg10, HAL_GetTick());
            servo_apply_angle(frame.angle_deg10);
        }
        /* Re-arm the interrupt for the next byte before returning to the loop. */
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1u);
    }
}
