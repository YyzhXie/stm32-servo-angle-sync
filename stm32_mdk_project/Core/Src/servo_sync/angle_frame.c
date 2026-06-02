#include "servo_sync/angle_frame.h"

#include "servo_sync/angle_math.h"

uint8_t angle_crc8(const uint8_t *data, size_t length)
{
    uint8_t crc = 0x00u;

    for (size_t i = 0u; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0u; bit < 8u; ++bit) {
            if ((crc & 0x80u) != 0u) {
                crc = (uint8_t)((crc << 1u) ^ 0x07u);
            } else {
                crc = (uint8_t)(crc << 1u);
            }
        }
    }

    return crc;
}

size_t angle_frame_encode(uint8_t sequence,
                          uint16_t angle_deg10,
                          uint8_t out[ANGLE_FRAME_SIZE])
{
    const uint16_t clamped = angle_clamp_deg10(angle_deg10);

    out[0] = ANGLE_FRAME_HEAD0;
    out[1] = ANGLE_FRAME_HEAD1;
    out[2] = ANGLE_FRAME_VERSION;
    out[3] = sequence;
    out[4] = (uint8_t)(clamped & 0xFFu);
    out[5] = (uint8_t)((clamped >> 8u) & 0xFFu);
    out[6] = angle_crc8(out, ANGLE_FRAME_SIZE - 1u);

    return ANGLE_FRAME_SIZE;
}

void angle_frame_decoder_init(angle_frame_decoder_t *decoder)
{
    for (uint8_t i = 0u; i < ANGLE_FRAME_SIZE; ++i) {
        decoder->buffer[i] = 0u;
    }
    decoder->index = 0u;
    decoder->frames_ok = 0u;
    decoder->crc_errors = 0u;
    decoder->sync_errors = 0u;
}

static void decoder_restart_with_byte(angle_frame_decoder_t *decoder, uint8_t byte)
{
    decoder->index = 0u;
    if (byte == ANGLE_FRAME_HEAD0) {
        decoder->buffer[0] = byte;
        decoder->index = 1u;
    }
}

angle_frame_status_t angle_frame_decoder_push(angle_frame_decoder_t *decoder,
                                              uint8_t byte,
                                              angle_frame_t *out_frame)
{
    if (decoder->index == 0u) {
        if (byte == ANGLE_FRAME_HEAD0) {
            decoder->buffer[0] = byte;
            decoder->index = 1u;
        } else {
            decoder->sync_errors++;
        }
        return ANGLE_FRAME_WAITING;
    }

    if (decoder->index == 1u) {
        if (byte != ANGLE_FRAME_HEAD1) {
            decoder->sync_errors++;
            decoder_restart_with_byte(decoder, byte);
            return ANGLE_FRAME_BAD_HEADER;
        }
    }

    decoder->buffer[decoder->index] = byte;
    decoder->index++;

    if (decoder->index < ANGLE_FRAME_SIZE) {
        return ANGLE_FRAME_WAITING;
    }

    const uint8_t expected = angle_crc8(decoder->buffer, ANGLE_FRAME_SIZE - 1u);
    const uint8_t received = decoder->buffer[ANGLE_FRAME_SIZE - 1u];
    if (expected != received || decoder->buffer[2] != ANGLE_FRAME_VERSION) {
        decoder->crc_errors++;
        decoder_restart_with_byte(decoder, byte);
        return ANGLE_FRAME_BAD_CRC;
    }

    out_frame->sequence = decoder->buffer[3];
    out_frame->angle_deg10 =
        angle_clamp_deg10((uint16_t)decoder->buffer[4] |
                          ((uint16_t)decoder->buffer[5] << 8u));
    decoder->frames_ok++;
    decoder->index = 0u;
    return ANGLE_FRAME_OK;
}

void angle_can_encode_payload(uint8_t sequence,
                              uint16_t angle_deg10,
                              uint8_t out[ANGLE_CAN_DLC])
{
    const uint16_t clamped = angle_clamp_deg10(angle_deg10);

    out[0] = sequence;
    out[1] = (uint8_t)(clamped & 0xFFu);
    out[2] = (uint8_t)((clamped >> 8u) & 0xFFu);
    out[3] = angle_crc8(out, ANGLE_CAN_DLC - 1u);
}

int angle_can_decode_payload(const uint8_t data[ANGLE_CAN_DLC],
                             uint8_t dlc,
                             angle_frame_t *out_frame)
{
    if (dlc != ANGLE_CAN_DLC) {
        return 0;
    }

    if (angle_crc8(data, ANGLE_CAN_DLC - 1u) != data[3]) {
        return 0;
    }

    out_frame->sequence = data[0];
    out_frame->angle_deg10 =
        angle_clamp_deg10((uint16_t)data[1] | ((uint16_t)data[2] << 8u));
    return 1;
}
