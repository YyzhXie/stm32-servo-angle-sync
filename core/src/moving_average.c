#include "servo_sync/moving_average.h"

void moving_average_init(moving_average_t *filter, uint8_t window)
{
    if (window == 0u) {
        window = 1u;
    }
    if (window > MOVING_AVERAGE_MAX_WINDOW) {
        window = MOVING_AVERAGE_MAX_WINDOW;
    }

    for (uint8_t i = 0u; i < MOVING_AVERAGE_MAX_WINDOW; ++i) {
        filter->samples[i] = 0u;
    }
    filter->sum = 0u;
    filter->window = window;
    filter->count = 0u;
    filter->index = 0u;
}

uint16_t moving_average_push(moving_average_t *filter, uint16_t sample)
{
    if (filter->count < filter->window) {
        filter->samples[filter->index] = sample;
        filter->sum += sample;
        filter->count++;
    } else {
        filter->sum -= filter->samples[filter->index];
        filter->samples[filter->index] = sample;
        filter->sum += sample;
    }

    filter->index++;
    if (filter->index >= filter->window) {
        filter->index = 0u;
    }

    return moving_average_value(filter);
}

uint16_t moving_average_value(const moving_average_t *filter)
{
    if (filter->count == 0u) {
        return 0u;
    }
    return (uint16_t)((filter->sum + filter->count / 2u) / filter->count);
}
