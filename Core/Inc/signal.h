/*
 * signal.h
 *
 *  Created on: Mar 11, 2026
 *      Author: shuja
 */

#ifndef INC_SIGNAL_H_
#define INC_SIGNAL_H_

#include "main.h"

extern const char *litter_names[];

typedef enum
{
  METAL,
  PLASTIC,
  PAPER
} litter_type;

typedef struct {
  int speed;
  int duration;
  litter_type litter;
} start_signal;

start_signal wait_for_start(void);

#endif /* INC_SIGNAL_H_ */
