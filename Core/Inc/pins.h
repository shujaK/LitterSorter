/*
 * pins.h
 *
 *  Created on: Mar 12, 2026
 *      Author: shuja
 */

#ifndef INC_PINS_H_
#define INC_PINS_H_

// DC Motor
#define MOTOR_PWM_Pin                	GPIO_PIN_8
#define MOTOR_PWM_Port          	GPIOA

#define MOTOR_IN1_Pin                	GPIO_PIN_0
#define MOTOR_IN2_Pin                	GPIO_PIN_1
#define MOTOR_STANDBY_Pin            	GPIO_PIN_10

#define MOTOR_GPIO_Port          	GPIOB

// Servos
#define SERVO1_PWM_Channel		TIM_CHANNEL_2
#define SERVO2_PWM_Channel		TIM_CHANNEL_3
#define SERVO3_PWM_Channel		TIM_CHANNEL_4

#endif /* INC_PINS_H_ */
