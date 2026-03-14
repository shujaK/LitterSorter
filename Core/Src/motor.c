/*
 * motor.c
 *
 *  Created on: Mar 13, 2026
 *      Author: shuja
 */

#include "motor.h"
#include "pins.h"

static TIM_HandleTypeDef *_htim = NULL;

static uint32_t
pct_to_ccr (uint8_t pct)
{
  if (pct > 100)
    pct = 100;
  return ((uint32_t) pct * _htim->Init.Period) / 100;
}

void
motor_init (TIM_HandleTypeDef *htim)
{
  _htim = htim;

  HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN1_Pin | MOTOR_IN2_Pin,
		     GPIO_PIN_RESET);

  __HAL_TIM_SET_COMPARE(_htim, MOTOR_PWM_Channel, 0);
  HAL_TIM_PWM_Start (_htim, MOTOR_PWM_Channel);
}

void
motor_stop (void)
{
  __HAL_TIM_SET_COMPARE(_htim, MOTOR_PWM_Channel, 0); // coast stop
  HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN1_Pin | MOTOR_IN2_Pin,
		     GPIO_PIN_RESET);
}

void
motor_set_speed (uint8_t speed_pct)
{
  __HAL_TIM_SET_COMPARE(_htim, MOTOR_PWM_Channel, pct_to_ccr (speed_pct));
}

void
motor_enable (uint8_t speed_pct, motor_direction_t direction)
{
  if (speed_pct != 0) motor_set_speed (speed_pct); // speed_pct = 0 : unchanged
  if (direction == MOTOR_FORWARD)
    {
      HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_RESET);
    }
  else
    {
      HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin (MOTOR_GPIO_Port, MOTOR_IN2_Pin, GPIO_PIN_SET);
    }

}
