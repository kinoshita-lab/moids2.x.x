#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#define ONE_SEC_COUNT (8000)
#define MOIDS_PER_UNIT (3)

#define LED_BRIGHTNESS_OFF (1)
#define LED_BRIGHTNESS_ON (4)

extern const uint16_t pulse_high_table[] PROGMEM;
extern const uint16_t pulse_low_table[] PROGMEM;
extern const uint16_t showa_delay_table[] PROGMEM;
extern const uint16_t moids_on_table[] PROGMEM;
extern const uint16_t moids_wait_table[] PROGMEM;
extern PROGMEM const uint16_t sequence_length_table[] ;

extern int pulse_high_index;
extern int pulse_low_index;
extern int showa_delay_index;
extern int moids_on_index;
extern int moids_wait_index;
extern int sequence_length_index;

extern uint16_t pulse_high;
extern uint16_t pulse_low;
extern uint16_t showa_delay;
extern uint16_t moids_on;
extern uint16_t moids_wait;
extern uint16_t sequence_length;
