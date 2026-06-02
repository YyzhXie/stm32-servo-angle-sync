#include "servo_sync/angle_math.h"
#include "servo_sync/angle_receiver.h"

void angle_receiver_init(angle_receiver_t *receiver,
                         uint32_t timeout_ms,
                         uint16_t safe_angle_deg10)
{
    receiver->current_angle_deg10 = angle_clamp_deg10(safe_angle_deg10);
    receiver->safe_angle_deg10 = angle_clamp_deg10(safe_angle_deg10);
    receiver->timeout_ms = timeout_ms;
    receiver->last_rx_ms = 0u;
    receiver->has_frame = false;
    receiver->timed_out = false;
}

void angle_receiver_accept(angle_receiver_t *receiver,
                           uint16_t angle_deg10,
                           uint32_t now_ms)
{
    receiver->current_angle_deg10 = angle_clamp_deg10(angle_deg10);
    receiver->last_rx_ms = now_ms;
    receiver->has_frame = true;
    receiver->timed_out = false;
}

uint16_t angle_receiver_poll(angle_receiver_t *receiver, uint32_t now_ms)
{
    if (!receiver->has_frame) {
        /* Before the first valid frame, keep the servo at the safe center. */
        receiver->timed_out = true;
        receiver->current_angle_deg10 = receiver->safe_angle_deg10;
        return receiver->current_angle_deg10;
    }

    if ((uint32_t)(now_ms - receiver->last_rx_ms) > receiver->timeout_ms) {
        /* Do not keep driving the last command after communication is lost. */
        receiver->timed_out = true;
        receiver->current_angle_deg10 = receiver->safe_angle_deg10;
    }

    return receiver->current_angle_deg10;
}
