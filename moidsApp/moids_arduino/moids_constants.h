#pragma once

static const int ONE_SEC_COUNT  = 8000;
static const int MOIDS_PER_UNIT = 3;

static const int LED_BRIGHTNESS_OFF = 1;
static const int LED_BRIGHTNESS_ON  = 4;

extern volatile int pulse_high_table[];
extern volatile int pulse_low_table[];
extern volatile int showa_delay_table[];
extern volatile int moids_on_table[];
extern volatile int moids_wait_table[];
extern volatile int sequence_length_table[];

extern volatile int* pulse_high;
extern volatile int* pulse_low;
extern volatile int* showa_delay;
extern volatile int* moids_on;
extern volatile int* moids_wait;
extern volatile int* sequence_length;
