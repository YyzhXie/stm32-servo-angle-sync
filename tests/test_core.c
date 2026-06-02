#include "servo_sync/angle_frame.h"
#include "servo_sync/angle_math.h"
#include "servo_sync/angle_receiver.h"
#include "servo_sync/moving_average.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__,          \
                    __LINE__, #expr);                                         \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static void test_angle_mapping(void)
{
    CHECK(angle_from_adc(0u) == 0u);
    CHECK(angle_from_adc(4095u) == 1800u);
    CHECK(angle_from_adc(2048u) == 900u);
    CHECK(angle_from_adc(5000u) == 1800u);

    CHECK(servo_pulse_us_from_angle(0u) == 500u);
    CHECK(servo_pulse_us_from_angle(900u) == 1500u);
    CHECK(servo_pulse_us_from_angle(1800u) == 2500u);
    CHECK(servo_pulse_us_from_angle(5000u) == 2500u);

    CHECK(servo_duty_permyriad_from_angle(0u) == 250u);
    CHECK(servo_duty_permyriad_from_angle(1800u) == 1250u);
}

static void test_moving_average(void)
{
    moving_average_t filter;
    moving_average_init(&filter, 4u);

    CHECK(moving_average_push(&filter, 100u) == 100u);
    CHECK(moving_average_push(&filter, 200u) == 150u);
    CHECK(moving_average_push(&filter, 300u) == 200u);
    CHECK(moving_average_push(&filter, 400u) == 250u);
    CHECK(moving_average_push(&filter, 500u) == 350u);
}

static void test_uart_frame(void)
{
    uint8_t encoded[ANGLE_FRAME_SIZE];
    angle_frame_t decoded;
    angle_frame_decoder_t decoder;

    CHECK(angle_frame_encode(7u, 1234u, encoded) == ANGLE_FRAME_SIZE);
    CHECK(encoded[0] == ANGLE_FRAME_HEAD0);
    CHECK(encoded[1] == ANGLE_FRAME_HEAD1);

    angle_frame_decoder_init(&decoder);
    for (size_t i = 0u; i < ANGLE_FRAME_SIZE - 1u; ++i) {
        CHECK(angle_frame_decoder_push(&decoder, encoded[i], &decoded) ==
              ANGLE_FRAME_WAITING);
    }
    CHECK(angle_frame_decoder_push(&decoder, encoded[ANGLE_FRAME_SIZE - 1u],
                                   &decoded) == ANGLE_FRAME_OK);
    CHECK(decoded.sequence == 7u);
    CHECK(decoded.angle_deg10 == 1234u);
    CHECK(decoder.frames_ok == 1u);

    angle_frame_decoder_init(&decoder);
    encoded[4] ^= 0x55u;
    for (size_t i = 0u; i < ANGLE_FRAME_SIZE - 1u; ++i) {
        (void)angle_frame_decoder_push(&decoder, encoded[i], &decoded);
    }
    CHECK(angle_frame_decoder_push(&decoder, encoded[ANGLE_FRAME_SIZE - 1u],
                                   &decoded) == ANGLE_FRAME_BAD_CRC);
    CHECK(decoder.crc_errors == 1u);

    CHECK(angle_frame_encode(8u, 900u, encoded) == ANGLE_FRAME_SIZE);
    for (size_t i = 0u; i < ANGLE_FRAME_SIZE - 1u; ++i) {
        (void)angle_frame_decoder_push(&decoder, encoded[i], &decoded);
    }
    CHECK(angle_frame_decoder_push(&decoder, encoded[ANGLE_FRAME_SIZE - 1u],
                                   &decoded) == ANGLE_FRAME_OK);
    CHECK(decoded.sequence == 8u);
    CHECK(decoded.angle_deg10 == 900u);
}

static void test_can_payload(void)
{
    uint8_t payload[ANGLE_CAN_DLC];
    angle_frame_t decoded;

    angle_can_encode_payload(12u, 1777u, payload);
    CHECK(angle_can_decode_payload(payload, ANGLE_CAN_DLC, &decoded) == 1);
    CHECK(decoded.sequence == 12u);
    CHECK(decoded.angle_deg10 == 1777u);

    payload[2] ^= 0x01u;
    CHECK(angle_can_decode_payload(payload, ANGLE_CAN_DLC, &decoded) == 0);
    payload[2] ^= 0x01u;
    CHECK(angle_can_decode_payload(payload, 3u, &decoded) == 0);
}

static void test_receiver_timeout(void)
{
    angle_receiver_t receiver;
    angle_receiver_init(&receiver, 500u, SERVO_SYNC_ANGLE_SAFE_DEG10);

    CHECK(angle_receiver_poll(&receiver, 0u) == SERVO_SYNC_ANGLE_SAFE_DEG10);
    CHECK(receiver.timed_out);

    angle_receiver_accept(&receiver, 1200u, 100u);
    CHECK(angle_receiver_poll(&receiver, 599u) == 1200u);
    CHECK(!receiver.timed_out);

    CHECK(angle_receiver_poll(&receiver, 601u) == SERVO_SYNC_ANGLE_SAFE_DEG10);
    CHECK(receiver.timed_out);
}

int main(void)
{
    test_angle_mapping();
    test_moving_average();
    test_uart_frame();
    test_can_payload();
    test_receiver_timeout();

    puts("core unit tests passed");
    return 0;
}
