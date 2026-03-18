#include "servo.h"

static TIM_HandleTypeDef *_htim2 = NULL;

#define SERVO_TIMER_INSTANCE        TIM2
#define SERVO_PWM_HZ                50U
#define SERVO_TIMER_TICK_HZ         1000000U
#define SERVO_PWM_PERIOD_TICKS      ((SERVO_TIMER_TICK_HZ / SERVO_PWM_HZ) - 1U)

#define SERVO_FRAME_US              20000U
#define SERVO_CLOSE_PULSE_US        1000U
#define SERVO_OPEN_PULSE_US         2000U

static uint32_t
tim2_input_clk_hz (void)
{
  RCC_ClkInitTypeDef clk_init =
    { 0 };
  uint32_t flash_latency = 0;

  HAL_RCC_GetClockConfig (&clk_init, &flash_latency);

  if (clk_init.APB1CLKDivider == RCC_HCLK_DIV1)
    return HAL_RCC_GetPCLK1Freq ();

  return HAL_RCC_GetPCLK1Freq () * 2U;
}

static uint32_t
pulse_us_to_ccr (uint32_t pulse_us)
{
  if (pulse_us > SERVO_FRAME_US)
    pulse_us = SERVO_FRAME_US;

  return ((_htim2->Init.Period + 1U) * pulse_us) / SERVO_FRAME_US;
}

static uint32_t
channel_from_type (litter_type type)
{
  switch (type)
    {
    case METAL:
      return SERVO1_PWM_Channel;
    case PLASTIC:
      return SERVO2_PWM_Channel;
    case PAPER:
      return SERVO3_PWM_Channel;
    default:
      return 0U;
    }
}

void
init_servos (TIM_HandleTypeDef *tim)
{
  TIM_OC_InitTypeDef sConfigOC =
    { 0 };

  if (tim == NULL)
    return;

  _htim2 = tim;

  if (_htim2->Instance != SERVO_TIMER_INSTANCE)
    {
      Error_Handler ();
      return;
    }

  _htim2->Init.Prescaler = (tim2_input_clk_hz () / SERVO_TIMER_TICK_HZ) - 1U;
  _htim2->Init.CounterMode = TIM_COUNTERMODE_UP;
  _htim2->Init.Period = SERVO_PWM_PERIOD_TICKS;
  _htim2->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  _htim2->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_PWM_Init (_htim2) != HAL_OK)
    {
      Error_Handler ();
      return;
    }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse_us_to_ccr (SERVO_CLOSE_PULSE_US);
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

  if (HAL_TIM_PWM_ConfigChannel (_htim2, &sConfigOC, SERVO1_PWM_Channel)
      != HAL_OK)
    {
      Error_Handler ();
      return;
    }

  if (HAL_TIM_PWM_ConfigChannel (_htim2, &sConfigOC, SERVO2_PWM_Channel)
      != HAL_OK)
    {
      Error_Handler ();
      return;
    }

  if (HAL_TIM_PWM_ConfigChannel (_htim2, &sConfigOC, SERVO3_PWM_Channel)
      != HAL_OK)
    {
      Error_Handler ();
      return;
    }

  __HAL_TIM_SET_COMPARE(_htim2, SERVO1_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));
  __HAL_TIM_SET_COMPARE(_htim2, SERVO2_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));
  __HAL_TIM_SET_COMPARE(_htim2, SERVO3_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));

  HAL_TIM_PWM_Start (_htim2, SERVO1_PWM_Channel);
  HAL_TIM_PWM_Start (_htim2, SERVO2_PWM_Channel);
  HAL_TIM_PWM_Start (_htim2, SERVO3_PWM_Channel);
}

void
open_servo (litter_type type)
{
  uint32_t channel;

  if (_htim2 == NULL)
    return;

  channel = channel_from_type (type);
  if (channel == 0U)
    return;

  __HAL_TIM_SET_COMPARE(_htim2, channel, pulse_us_to_ccr (SERVO_OPEN_PULSE_US));
}

void
close_servo (litter_type type)
{
  uint32_t channel;

  if (_htim2 == NULL)
    return;

  channel = channel_from_type (type);
  if (channel == 0U)
    return;

  __HAL_TIM_SET_COMPARE(_htim2, channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));

}

void
close_all_servos ()
{
  if (_htim2 == NULL)
    return;

  __HAL_TIM_SET_COMPARE(_htim2, SERVO1_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));
  __HAL_TIM_SET_COMPARE(_htim2, SERVO2_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));
  __HAL_TIM_SET_COMPARE(_htim2, SERVO3_PWM_Channel,
			pulse_us_to_ccr (SERVO_CLOSE_PULSE_US));
}

