#ifndef SERVO_SYNC_ANGLE_FRAME_H
#define SERVO_SYNC_ANGLE_FRAME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANGLE_FRAME_HEAD0 0xA5u
#define ANGLE_FRAME_HEAD1 0x5Au
#define ANGLE_FRAME_VERSION 0x01u
#define ANGLE_FRAME_SIZE 7u

#define ANGLE_CAN_STD_ID 0x321u
#define ANGLE_CAN_DLC 4u

/*
 * Angle transport unit shared by UART and CAN.
 * angle_deg10 is clamped to 0..1800 before transmission.
 */
typedef struct {
    uint8_t sequence;
    uint16_t angle_deg10;
} angle_frame_t;

typedef enum {
    ANGLE_FRAME_WAITING = 0,
    ANGLE_FRAME_OK = 1,
    ANGLE_FRAME_BAD_CRC = -1,
    ANGLE_FRAME_BAD_HEADER = -2
} angle_frame_status_t;

typedef struct {
    uint8_t buffer[ANGLE_FRAME_SIZE];
    uint8_t index;
    uint32_t frames_ok;
    uint32_t crc_errors;
    uint32_t sync_errors;
} angle_frame_decoder_t;

/* CRC-8/ATM polynomial 0x07, used to reject corrupted UART/CAN angle data. */
uint8_t angle_crc8(const uint8_t *data, size_t length);

/* Encode the required UART frame: A5 5A version sequence angle_l angle_h crc. */
size_t angle_frame_encode(uint8_t sequence,
                          uint16_t angle_deg10,
                          uint8_t out[ANGLE_FRAME_SIZE]);

void angle_frame_decoder_init(angle_frame_decoder_t *decoder);

/* Push one UART byte at a time; the decoder resynchronizes after bad bytes. */
angle_frame_status_t angle_frame_decoder_push(angle_frame_decoder_t *decoder,
                                              uint8_t byte,
                                              angle_frame_t *out_frame);

/* Encode/decode the optional CAN payload carried by standard frame ID 0x321. */
void angle_can_encode_payload(uint8_t sequence,
                              uint16_t angle_deg10,
                              uint8_t out[ANGLE_CAN_DLC]);

int angle_can_decode_payload(const uint8_t data[ANGLE_CAN_DLC],
                             uint8_t dlc,
                             angle_frame_t *out_frame);

#ifdef __cplusplus
}
#endif

#endif
