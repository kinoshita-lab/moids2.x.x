#include "Timer2_125usec.h"
#include "workaround.h"
#include <digitalWriteFast.h>
#include <stdint.h>
#include <util/delay.h>

#include "Moids.h"
#include "moids_constants.h"
#include "moids_pins.h"
#include "moids_sequence_mode.h"
#include "debug.h"
//#define DEBUG



#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

Moids moids[MOIDS_PER_UNIT];

bool led_state                = false;
unsigned int timerTickCounter = 0;
unsigned int moidsSec         = 0;
bool print_sec                = false;

bool should_goto_next_sequence = false;

void timerTick()
{
    timerTickCounter++;

    if (moidsMode) {
        if (timerTickCounter >= ONE_SEC_COUNT) {
            timerTickCounter = 0;
            moidsSec++;
        }

        if (moidsSec >= sequence_length) {
            moidsSec = 0;
            sequence_length++;
            currentSequence++;
            should_goto_next_sequence = true;
            return;
        }

        for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
            moids[i].tick();
        }

        return;
    }


    if (pulseMode || showaMode) {
        if (timerTickCounter >= sequence_length) {
            timerTickCounter = 0;
            sequence_length++;
            currentSequence++;
            should_goto_next_sequence = true;
        }
    }
}

void setup_timer() {
  cli();
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
  TCCR0B = (TCCR0B & 0b11111000) | 0x02;
  TIMSK0 &= ~_BV(TOIE0);
  Timer2_125usec::set(ONE_SEC_COUNT, timerTick);
  Timer2_125usec::start();
  sei();
}

void setup_adc() {
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
}


void setup() {


  moids_wait = pgm_read_word_near(&moids_wait_table[moids_wait_index]);
#ifdef DEBUG
  Serial.begin(115200);
#endif
  for (int i = 0; i < 13; i++) {
    pinMode(i, OUTPUT);
  }
  setup_adc();

  randomSeed(analogRead(0));

  for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
    pinMode(OUTPUT_LED_PINS[i], OUTPUT);
    pinMode(OUTPUT_RELAY_PINS[i], OUTPUT);
  }

  for (int i = 0; i < MOIDS_PER_UNIT; i++) {
    moids[i].setInputMicPin(INPUT_MIC_PINS[i]);
    moids[i].setOutputLEDPin(OUTPUT_LED_PINS[i]);
    moids[i].setOutputRelayPin(OUTPUT_RELAY_PINS[i]);
    moids[i].init();

    for (int j = 0; j < MOIDS_PER_UNIT; j++) {
      if (&moids[j] != &moids[i]) {
        moids[i].registerOtherMoids(&moids[j]);
      }
    }
  }
  setup_timer();

  should_goto_next_sequence = true;
}

void loop() {
  if (should_goto_next_sequence) {
    should_goto_next_sequence = false;
    setNextSequenceData();
  }
  Serial.flush();
  if (pulseMode) {
    makePulse();
    return;
  } else if (showaMode) {
    makeShowa();
    return;
  } else if (moidsMode) {
    for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
      moids[i].loop();
    }
    return;
  }
}
