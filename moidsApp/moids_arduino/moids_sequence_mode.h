#pragma once

#include <stdint.h>
#include <stdbool.h>

enum SequenceName
{
    randomPulse,
    oneThirdIs125,
    twoThirdIs125,
    fourFifthIs125,
    all125,

    oneFifthIsShowa,
    oneThirdIsShowa,
    twoThirdIsShowa,
    allShowa,

    showa250,
    showa375,
    showa500,
    showa625,
    showa750,

    showa875,
    showa1000,
    showa1125,
    showa1250,
    showa2500,

    showa5000,
    showa10000,
    showa15000,
    showa25000,

    showaDecay93,
    showaDecay86,
    showaDecay79,
    showaDecay72,
    showaDecay65,

    showaDecay58,
    showaDecay51,
    showaDecay44,
    showaDecay37,
    showaDecay30,

    showaDecay23,
    showaDecay16,
    showaDecay9,

    moids7,
    moids6,
    moids5,
    moids4,

    moids_dead5,
    moids_dead6,
    moids_dead7,
    moids_dead8,
    moids_dead9,
    moids_dead10,

    moids_dead,
};

extern bool pulseMode;
extern bool showaMode;
extern bool moidsMode;

extern int showaProbability;
extern int currentSequence;

extern bool led_state;
extern unsigned int timerTickCounter;
extern unsigned int moidsSec;
extern bool print_sec;

void timerTick();
void setNextSequenceData();

void chooseMoidsDead();
void chooseMoidsThreshold(const int thres);
void chooseShowaDecay(int probability);
void chooseDelayedShowa(int delayTime);
void choosePulseSequence(int prob_to_zero, int max_seq_number);
void chooseShowaSequence(int prob_to_showa);

void makePulse();
void makeShowa();
void onAll();
void offAll();
