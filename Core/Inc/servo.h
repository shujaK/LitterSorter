/*
 * servo.h
 *
 *  Created on: Mar 16, 2026
 *      Author: marno
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "signal.h"
#include "pins.h"

#define SERVO_MIN 500   // 0.5ms pulse (0 degrees)
#define SERVO_MID 1500  // 1.5ms pulse (90 degrees)
#define SERVO_MAX 2500  // 2.5ms pulse (180 degrees)

#define CLOSE SERVO_MIN
#define OPEN  SERVO_MAX

void init_servos(TIM_HandleTypeDef *tim);

void open_servo(litter_type type);
void close_servo(litter_type type);
void close_all_servos();


#endif /* INC_SERVO_H_ */
