#include "servo.h"

static TIM_HandleTypeDef *_htim2 = NULL;

static uint32_t
pct_to_ccr (uint8_t pct)
{
  if (pct > 100)
    pct = 100;
  return ((uint32_t) pct * _htim2->Init.Period) / 100;
}

void init_servos(TIM_HandleTypeDef *tim){
    _htim2 = tim;

    //close_all_servos();

    HAL_TIM_PWM_Start (_htim2, SERVO1_PWM_Channel);
    HAL_TIM_PWM_Start (_htim2, SERVO2_PWM_Channel);
    HAL_TIM_PWM_Start (_htim2, SERVO3_PWM_Channel);
}

void open_servo(litter_type type) {
    // 1. Safety first: Close all flaps before opening a specific one
    // This prevents mechanical jams and ensures only one path is open.
    close_all_servos();
    
    // 2. Small delay (optional) 
    // HAL_Delay(50); // Gives the previous "close" command a head start

    // 3. Open the specific channel
    switch(type) {
        case METAL:
            __HAL_TIM_SET_COMPARE(_htim2, SERVO1_PWM_Channel, SERVO_MAX);
            break;
            
        case PLASTIC:
            __HAL_TIM_SET_COMPARE(_htim2, SERVO2_PWM_Channel, SERVO_MAX);
            break;
            
        case GLASS:
            __HAL_TIM_SET_COMPARE(_htim2, SERVO3_PWM_Channel, SERVO_MAX);
            break;
            
        default:
            // Fallback: stay closed if type is unknown
            close_all_servos();
            break;
    }
}

void close_servo(litter_type type){
    if(type == METAL){
        // Channel 1
        __HAL_TIM_SET_COMPARE(_htim2, SERVO1_PWM_Channel, pct_to_ccr(CLOSE));
    } else if(type == PLASTIC){
        // Channel 2
        __HAL_TIM_SET_COMPARE(_htim2, SERVO2_PWM_Channel, pct_to_ccr(CLOSE));
    } else if(type == GLASS){
        // Channel 3
        __HAL_TIM_SET_COMPARE(_htim2, SERVO3_PWM_Channel, pct_to_ccr(CLOSE));
    }
}

void close_all_servos()
   {
    __HAL_TIM_SET_COMPARE(_htim2, SERVO1_PWM_Channel, SERVO_MIN);
    
    __HAL_TIM_SET_COMPARE(_htim2, SERVO2_PWM_Channel, SERVO_MIN);
    
    __HAL_TIM_SET_COMPARE(_htim2, SERVO3_PWM_Channel, SERVO_MIN);
        
}


