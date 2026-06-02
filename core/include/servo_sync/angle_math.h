#ifndef SERVO_SYNC_ANGLE_MATH_H
#define SERVO_SYNC_ANGLE_MATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SERVO_SYNC_ADC_MAX            4095u
#define SERVO_SYNC_ANGLE_MIN_DEG10    0u
#define SERVO_SYNC_ANGLE_MAX_DEG10    1800u
#define SERVO_SYNC_ANGLE_SAFE_DEG10   900u
#define SERVO_SYNC_PWM_MIN_US         500u
#define SERVO_SYNC_PWM_MAX_US         2500u
#define SERVO_SYNC_PWM_PERIOD_US      20000u

uint16_t angle_clamp_deg10(uint16_t angle_deg10);
uint16_t angle_from_adc(uint16_t raw_adc);
uint16_t servo_pulse_us_from_angle(uint16_t angle_deg10);
uint16_t servo_duty_permyriad_from_angle(uint16_t angle_deg10);

#ifdef __cplusplus
}
#endif

#endif
