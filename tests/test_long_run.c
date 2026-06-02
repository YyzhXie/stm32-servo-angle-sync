#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/angle_receiver.h"
#include "servo_sync/moving_average.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__,          \
                    __LINE__, #expr);                                         \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static uint16_t triangular_adc(uint32_t tick)
{
    const uint32_t period = 400u;
    const uint32_t phase = tick % period;
    const uint32_t value = (phase < period / 2u)
                               ? (phase * SERVO_SYNC_ADC_MAX) / (period / 2u)
                               : ((period - phase) * SERVO_SYNC_ADC_MAX) /
                                     (period / 2u);
    return (uint16_t)value;
}

static uint16_t add_small_noise(uint16_t value, uint32_t tick)
{
    const int noise = (int)((tick * 37u + 11u) % 17u) - 8;
    int noisy = (int)value + noise;
    if (noisy < 0) {
        noisy = 0;
    }
    if (noisy > (int)SERVO_SYNC_ADC_MAX) {
        noisy = (int)SERVO_SYNC_ADC_MAX;
    }
    return (uint16_t)noisy;
}

int main(void)
{
    moving_average_t adc_filter;
    angle_frame_decoder_t decoder;
    angle_receiver_t receiver;

    moving_average_init(&adc_filter, 8u);
    angle_frame_decoder_init(&decoder);
    angle_receiver_init(&receiver, 500u, SERVO_SYNC_ANGLE_SAFE_DEG10);

    uint8_t sequence = 0u;
    uint32_t delivered_frames = 0u;
    uint32_t intentionally_corrupt = 0u;

    for (uint32_t tick = 0u; tick < 2400u; ++tick) {
        const uint32_t now_ms = tick * 50u;
        const uint16_t raw_adc = add_small_noise(triangular_adc(tick), tick);
        const uint16_t filtered_adc = moving_average_push(&adc_filter, raw_adc);
        const uint16_t sent_angle = angle_from_adc(filtered_adc);

        uint8_t bytes[ANGLE_FRAME_SIZE];
        angle_frame_t frame;
        (void)angle_frame_encode(sequence++, sent_angle, bytes);

        if ((tick % 97u) == 35u) {
            bytes[5] ^= 0x20u;
            intentionally_corrupt++;
        }

        for (size_t i = 0u; i < ANGLE_FRAME_SIZE; ++i) {
            if (angle_frame_decoder_push(&decoder, bytes[i], &frame) ==
                ANGLE_FRAME_OK) {
                angle_receiver_accept(&receiver, frame.angle_deg10, now_ms);
                delivered_frames++;
            }
        }

        const uint16_t servo_angle = angle_receiver_poll(&receiver, now_ms);
        const uint16_t pulse = servo_pulse_us_from_angle(servo_angle);

        CHECK(pulse >= SERVO_SYNC_PWM_MIN_US);
        CHECK(pulse <= SERVO_SYNC_PWM_MAX_US);
        CHECK(!receiver.timed_out);

        /*
         * A corrupted frame should be dropped, so the slave may hold the last
         * valid angle for one 50 ms cycle. The exam allows <= 15 degrees.
         */
        const int err = (int)servo_angle - (int)sent_angle;
        CHECK(err <= 150 && err >= -150);
    }

    CHECK(delivered_frames > 2300u);
    CHECK(decoder.crc_errors == intentionally_corrupt);
    CHECK(decoder.frames_ok == delivered_frames);

    printf("long-run simulation passed: %lu frames, %lu injected crc errors\n",
           (unsigned long)delivered_frames,
           (unsigned long)intentionally_corrupt);
    return 0;
}
