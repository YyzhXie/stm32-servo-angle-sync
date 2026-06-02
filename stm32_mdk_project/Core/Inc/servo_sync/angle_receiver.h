#ifndef SERVO_SYNC_ANGLE_RECEIVER_H
#define SERVO_SYNC_ANGLE_RECEIVER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t current_angle_deg10;
    uint16_t safe_angle_deg10;
    uint32_t timeout_ms;
    uint32_t last_rx_ms;
    bool has_frame;
    bool timed_out;
} angle_receiver_t;

void angle_receiver_init(angle_receiver_t *receiver,
                         uint32_t timeout_ms,
                         uint16_t safe_angle_deg10);

void angle_receiver_accept(angle_receiver_t *receiver,
                           uint16_t angle_deg10,
                           uint32_t now_ms);

uint16_t angle_receiver_poll(angle_receiver_t *receiver, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
