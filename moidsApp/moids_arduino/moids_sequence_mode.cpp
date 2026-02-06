#include "moids_sequence_mode.h"
#include "moids_constants.h"
#include "moids_pins.h"
#include "Timer2_125usec.h"
#include "workaround.h"
#include <digitalWriteFast.h>
#include "Moids.h"
#include "debug.h"

extern Moids moids[MOIDS_PER_UNIT];

bool pulseMode = true;
bool showaMode = false;
bool moidsMode = false;

int showaProbability = 100;
int currentSequence  = randomPulse;

void setNextSequenceData()
{
    while (0 == sequence_length) {
        sequence_length_index++;
        currentSequence++;
        sequence_length = pgm_read_word_near(&sequence_length_table[currentSequence]);
    }
    DEBUG_PRINT("Current Sequence index:");
    DEBUG_PRINTLN(currentSequence);
    sequence_length = pgm_read_word_near(&sequence_length_table[currentSequence]);

    DEBUG_PRINT("Next Sequence:");
    DEBUG_PRINT(currentSequence);
    DEBUG_PRINT(" Length:");
    DEBUG_PRINTLN(sequence_length);

    switch (currentSequence) {
    case randomPulse:
        choosePulseSequence(0, 5);
        break;
    case oneThirdIs125:
        choosePulseSequence(33, 5);
        break;
    case twoThirdIs125:
        choosePulseSequence(66, 5);
        break;
    case fourFifthIs125:
        choosePulseSequence(80, 5);
        break;
    case all125:
        choosePulseSequence(100, 0);
        break;
    default:
        break;
    }

    switch (currentSequence) {
    case oneFifthIsShowa:
        chooseShowaSequence(20);
        break;
    case oneThirdIsShowa:
        chooseShowaSequence(33);
        break;
    case twoThirdIsShowa:
        chooseShowaSequence(66);
        break;
    case allShowa:
        chooseShowaSequence(100);
        break;
    default:
        break;
    }

    switch (currentSequence) {
    case showa250:
        chooseDelayedShowa(250);
        break;
    case showa375:
        chooseDelayedShowa(375);
        break;
    case showa500:
        chooseDelayedShowa(500);
        break;
    case showa625:
        chooseDelayedShowa(625);
        break;
    case showa750:
        chooseDelayedShowa(750);
        break;
    case showa875:
        chooseDelayedShowa(875);
        break;
    case showa1000:
        chooseDelayedShowa(1000);
        break;
    case showa1125:
        chooseDelayedShowa(1125);
        break;
    case showa1250:
        chooseDelayedShowa(1250);
        break;
    case showa2500:
        chooseDelayedShowa(2500);
        break;
    case showa5000:
        chooseDelayedShowa(5000);
        break;
    case showa10000:
        chooseDelayedShowa(10000);
        break;
    case showa15000:
        chooseDelayedShowa(15000);
        break;
    case showa25000:
        chooseDelayedShowa(25000);
        break;
    default:
        break;
    }

    switch (currentSequence) {
    case showaDecay93:
        chooseShowaDecay(93);
        break;
    case showaDecay86:
        chooseShowaDecay(86);
        break;
    case showaDecay79:
        chooseShowaDecay(79);
        break;
    case showaDecay72:
        chooseShowaDecay(72);
        break;
    case showaDecay65:
        chooseShowaDecay(65);
        break;
    case showaDecay58:
        chooseShowaDecay(58);
        break;
    case showaDecay51:
        chooseShowaDecay(51);
        break;
    case showaDecay44:
        chooseShowaDecay(44);
        break;
    case showaDecay37:
        chooseShowaDecay(37);
        break;
    case showaDecay30:
        chooseShowaDecay(30);
        break;
    case showaDecay23:
        chooseShowaDecay(23);
        break;
    case showaDecay16:
        chooseShowaDecay(16);
        break;
    case showaDecay9:
        chooseShowaDecay(9);
        break;
    default:
        break;
    }

    switch (currentSequence) {
    case moids7:
        DEBUG_PRINTLN("Choosing moids7");
        chooseMoidsThreshold(5);
        break;
    case moids6:
        chooseMoidsThreshold(20);
        break;
    case moids5:
        chooseMoidsThreshold(20);
        break;
    case moids4:
        chooseMoidsThreshold(20);
        break;
    case moids_dead5:
        chooseMoidsThreshold(50);
        break;
    case moids_dead6:
        chooseMoidsThreshold(60);
        break;
    case moids_dead7:
        chooseMoidsThreshold(70);
        break;
    case moids_dead8:
        chooseMoidsThreshold(80);
        break;
    case moids_dead9:
        chooseMoidsThreshold(90);
        break;
    case moids_dead10:
        chooseMoidsThreshold(100);
        break;
    case moids_dead:
        chooseMoidsDead();
        break;
    default:
        break;
    }

    // 2026 add precise mode transition
    switch (currentSequence) {
    case moids_transition_thres_0:
        chooseMoidsThreshold(0);
        break;
    case moids_transition_thres_1:
        chooseMoidsThreshold(1);
        break;
    case moids_transition_thres_2:
        chooseMoidsThreshold(2);
        break;
    case moids_transition_thres_3:
        chooseMoidsThreshold(3);
        break;
    case moids_transition_thres_4:
        chooseMoidsThreshold(4);
        break;
    case moids_transition_thres_5:
        chooseMoidsThreshold(5);
        break;  
    case moids_transition_thres_6:
        chooseMoidsThreshold(6);
        break;
    case moids_transition_thres_7:
        chooseMoidsThreshold(7);
        break;
    case moids_transition_thres_8:
        chooseMoidsThreshold(8);
        break;
    case moids_transition_thres_9:
        chooseMoidsThreshold(9);
        break;
    default:
        break;
    }
}

void chooseMoidsDead()
{
    moidsMode = false;
    pulseMode = false;
    showaMode = false;

    for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
        analogWrite(OUTPUT_LED_PINS[i], 0);
        digitalWriteFast(OUTPUT_RELAY_PINS[i], 0);
    }
}

extern void timerTick();

void chooseMoidsThreshold(const int thres)
{
    DEBUG_PRINTLN("Choosing Moids Threshold: " + String(thres));
    moidsMode = true;
    pulseMode = false;
    showaMode = false;

    Timer2_125usec::stop();
    Timer2_125usec::set(1, timerTick);
    Timer2_125usec::start();

    for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
        moids[i].setWaitAfterSoundDetect(moids_wait);
        moids[i].setMicThreshold(thres);
    }
}

void chooseShowaDecay(int probability) { showaProbability = probability; }

void chooseDelayedShowa(int delayTime)
{
    showaMode   = true;
    pulseMode   = false;
    showa_delay = delayTime;
}

void choosePulseSequence(int prob_to_zero, int max_seq_number)
{
    pulseMode       = true;
    int randomValue = random(101);
    if (randomValue <= prob_to_zero) {
        pulse_high = pgm_read_word_near(&pulse_high_table[0]);
        pulse_low  = pgm_read_word_near(&pulse_low_table[0]);
        return;
    }

    randomValue = random(max_seq_number + 1);
    pulse_high  = pgm_read_word_near(&pulse_high_table[randomValue]);
    pulse_low   = pgm_read_word_near(&pulse_low_table[randomValue]);
}

void chooseShowaSequence(int prob_to_showa)
{
    int randomValue = random(101);
    if (randomValue <= prob_to_showa) {
        showaMode   = true;
        pulseMode   = false;
        showa_delay = pgm_read_word_near(&showa_delay_table[0]);
        return;
    }
    showaMode = false;
    pulseMode = true;

    pulse_high = pgm_read_word_near(&pulse_high_table[0]);
    pulse_low  = pgm_read_word_near(&pulse_low_table[0]);
}

void makePulse()
{
    onAll();
    delayMicroseconds(pulse_high + EXTRA_DELAY_US);
    offAll();
    delayMicroseconds(pulse_low + EXTRA_DELAY_US);
}

void makeShowa()
{
    int showaOnOffRandom = random(101);
    if (showaOnOffRandom > showaProbability) {
        delayMicroseconds(showa_delay + EXTRA_DELAY_US);
        return;
    }

    int randomValue = random(3);
    analogWrite(OUTPUT_LED_PINS[randomValue], LED_BRIGHTNESS_ON);
    digitalWriteFast(OUTPUT_RELAY_PINS[randomValue], HIGH);

    delayMicroseconds(showa_delay + EXTRA_DELAY_US);
    analogWrite(OUTPUT_LED_PINS[randomValue], LED_BRIGHTNESS_OFF);
    digitalWriteFast(OUTPUT_RELAY_PINS[randomValue], LOW);
}

void onAll()
{
    for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
        digitalWriteFast(OUTPUT_RELAY_PINS[i], HIGH);
        analogWrite(OUTPUT_LED_PINS[i], LED_BRIGHTNESS_ON);
    }
}

void offAll()
{
    for (int i = 0; i < MOIDS_PER_UNIT; ++i) {
        digitalWriteFast(OUTPUT_RELAY_PINS[i], LOW);
        analogWrite(OUTPUT_LED_PINS[i], LED_BRIGHTNESS_OFF);
    }
}
