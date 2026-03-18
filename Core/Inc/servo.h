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

void init_servos(TIM_HandleTypeDef *tim);

void open_servo(litter_type type);
void close_servo(litter_type type);
void close_all_servos();


#endif /* INC_SERVO_H_ */
