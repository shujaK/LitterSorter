/*
 * signal.h
 *
 *  Created on: Mar 11, 2026
 *      Author: shuja
 */

#ifndef INC_SIGNAL_H_
#define INC_SIGNAL_H_

typedef enum
{
  METAL,
  PLASTIC,
  GLASS
} litter_type;

struct start_signal
{
  int speed;
  int duration;
  litter_type litter;
};

start_signal wait_for_start();

#endif /* INC_SIGNAL_H_ */
