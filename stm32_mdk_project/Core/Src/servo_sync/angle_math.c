#include "servo_sync/angle_math.h"

uint16_t angle_clamp_deg10(uint16_t angle_deg10)
{
    if (angle_deg10 > SERVO_SYNC_ANGLE_MAX_DEG10) {
        return SERVO_SYNC_ANGLE_MAX_DEG10;
    }
    return angle_deg10;
}

uint16_t angle_from_adc(uint16_t raw_adc)
{
    if (raw_adc > SERVO_SYNC_ADC_MAX) {
        raw_adc = SERVO_SYNC_ADC_MAX;
    }

    /* Rounded integer mapping: 0..4095 ADC -> 0.0..180.0 degrees. */
    const uint32_t numerator =
        (uint32_t)raw_adc * SERVO_SYNC_ANGLE_MAX_DEG10 + (SERVO_SYNC_ADC_MAX / 2u);
    return (uint16_t)(numerator / SERVO_SYNC_ADC_MAX);
}

uint16_t servo_pulse_us_from_angle(uint16_t angle_deg10)
{
    const uint16_t clamped = angle_clamp_deg10(angle_deg10);
    const uint32_t span = SERVO_SYNC_PWM_MAX_US - SERVO_SYNC_PWM_MIN_US;

    /* Rounded linear mapping: 0.0..180.0 degrees -> 500..2500 us. */
    const uint32_t numerator =
        (uint32_t)clamped * span + (SERVO_SYNC_ANGLE_MAX_DEG10 / 2u);
    return (uint16_t)(SERVO_SYNC_PWM_MIN_US + numerator / SERVO_SYNC_ANGLE_MAX_DEG10);
}

uint16_t servo_duty_permyriad_from_angle(uint16_t angle_deg10)
{
    const uint16_t pulse_us = servo_pulse_us_from_angle(angle_deg10);
    return (uint16_t)(((uint32_t)pulse_us * 10000u) / SERVO_SYNC_PWM_PERIOD_US);
}
