/*
 * motor.h
 *
 *  Created on: Mar 13, 2026
 *      Author: shuja
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_
#include "main.h"

typedef enum {
  MOTOR_FORWARD = 0,
  MOTOR_REVERSE
} motor_direction_t;

void motor_init(TIM_HandleTypeDef *htim);

void motor_stop(void);

void motor_enable(uint8_t speed_pct, motor_direction_t direction); // 0 = CCW

void motor_set_speed(uint8_t speed_pct);

#endif /* INC_MOTOR_H_ */
