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



//int my_putc(char c, FILE *t) { return Serial.write(c); }

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

Moids moids[MOIDS_PER_UNIT];

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

#ifdef DEBUG
  Serial.begin(115200);
  //fdevopen(&my_putc, 0);
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

  setNextSequenceData();
}

void loop() {

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
