/*
 * signal.c
 *
 *  Created on: Mar 11, 2026
 *      Author: shuja
 */


#include "signal.h"
#include <string.h>

extern volatile uint8_t cdc_rx_buffer[64];
extern volatile uint32_t cdc_rx_len;
extern volatile uint8_t cdc_rx_ready;

const char* litter_names[] = { "METAL", "PLASTIC", "PAPER" };

start_signal
wait_for_start(void)
{
  start_signal signal;

  // wait
  while (!cdc_rx_ready || cdc_rx_len < sizeof(start_signal)) {
  }

  // copy received data to struct
  memcpy(&signal, (void*)cdc_rx_buffer, sizeof(start_signal));

  // clear the ready flag for next time
  cdc_rx_ready = 0;
  cdc_rx_len = 0;

  return signal;
}
