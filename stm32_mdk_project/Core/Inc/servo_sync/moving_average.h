#ifndef SERVO_SYNC_MOVING_AVERAGE_H
#define SERVO_SYNC_MOVING_AVERAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOVING_AVERAGE_MAX_WINDOW 16u

typedef struct {
    uint16_t samples[MOVING_AVERAGE_MAX_WINDOW];
    uint32_t sum;
    uint8_t window;
    uint8_t count;
    uint8_t index;
} moving_average_t;

void moving_average_init(moving_average_t *filter, uint8_t window);
uint16_t moving_average_push(moving_average_t *filter, uint16_t sample);
uint16_t moving_average_value(const moving_average_t *filter);

#ifdef __cplusplus
}
#endif

#endif
