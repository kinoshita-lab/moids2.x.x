#include <util/delay.h>
#include "MsTimer2.h"// <- N.B. modified!
#include "Moids.h"

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif




/* ========================================================================
  CONSTANTS AND PIN ASSIGNS
========================================================================= */
static const int MOIDS_PER_UNIT = 3;
static const int INPUT_MIC_PINS[MOIDS_PER_UNIT]    = {1, 0, 2};
static const int OUTPUT_LED_PINS[MOIDS_PER_UNIT]   = {6, 10, 9};
static const int OUTPUT_RELAY_PINS[MOIDS_PER_UNIT] = {8, 7, 4};


static const int LED_BRIGHTNESS_OFF = 1;
static const int LED_BRIGHTNESS_ON = 4;
// The Moids!
Moids moids[MOIDS_PER_UNIT];

/* ========================================================================
  Tables
========================================================================= */
volatile int pulse_high_table[] =
{
    125, 125, 250, 125, 250,
    125, 250, 250, 375, 500,
    625, 500, 250, 375, 250,
    375, 500, 250, 375, 500,
    625, 750, 500, 625, 750,
    875, 500, 875, 1000, 500,
    625, 625, 875, 1000, 875,
    1000, 1250, 875, 1000, 1125,
    1250, 1325, 1450, 0,   0,
    0, 0, 0, 0, 0,
};
volatile int* pulse_high = &pulse_high_table[0];

volatile int pulse_low_table[] =
{
    125, 250, 250, 375, 375,
    500, 875, 1125, 1250, 1250,
    1375, 500, 625, 625, 750,
    750, 750, 875, 875, 875,
    875, 875, 1000, 1000, 1000,
    1000, 1125, 1125, 1125, 1250,
    1250, 1325, 1325, 1325, 1450,
    1325, 1325, 1250, 1250, 1250,
    1325, 1450, 1450, 0, 0,
    0, 0, 0, 0, 0,
};
volatile int* pulse_low = &pulse_low_table[0];

volatile int showa_delay_table[] =
{
    125,
};
volatile int* showa_delay = &showa_delay_table[0];

volatile int moids_on_table[] =
{
    80,
};
volatile int* moids_on = &moids_on_table[0];

volatile int moids_wait_table[] =
{
    375,
};
volatile int* moids_wait = &moids_wait_table[0];

// sec
// for debug
volatile int sequence_length_table[] =
{
    0, 0, 0, 0, 30, // �ҁ[�@random, 1/3, 2/3, 4/5, 100% �{����1��

    5, 5, 10, 20,    // �S��������킵���125�ɕς��@�{����20�b���H

    2, 2, 2, 2, 2, // ����킵���̃f�B���C�������� 250, 375, 500, 625, 750,
    2, 2, 2, 2, 2, // ����킵���̃f�B���C�������� 875, 100, 1125, 1250, 2500,
    2, 2, 2, 2,    // ����킵���̃f�B���C�������� 5000, 10000, 15000, 25000,
    1, 1, 1, 1, 1, // ����킵���̐��������Ă��� 93, 86, 79, 72, 65, 
    0, 1, 0, 1, 0, // ����킵���̐��������Ă��� 58, 510, 44, 37, 30,
    1, 0, 1,       // ����킵���̐��������Ă��� 23, 16, 9
    3, 3,3 , 60*14,    // Moids�̊��x���オ���Ă��� 7, 6, 5, 4, ��14��
    5, 4, 3, 2, 1, 1, // ���x���������Ă���
    600, //��
};

 /*
volatile int sequence_length_table[] =
{
    0,  0,  0,  0, 0, // �ҁ[�@random, 1/3, 2/3, 4/5, 100% �{����1��

    0, 0, 0, 0,    // �S��������킵���125�ɕς��@�{����20�b���H

    0, 0, 0, 0, 0, // ����킵���̃f�B���C�������� 250, 375, 500, 625, 750,
    0, 0, 0, 0, 0, // ����킵���̃f�B���C�������� 875, 1000, 1125, 1250, 2500,
    0, 0, 0, 0,    // ����킵���̃f�B���C�������� 5000, 10000, 15000, 25000,

    0, 0, 0, 0, 0, // ����킵���̐��������Ă��� 93, 86, 79, 72, 65, 
    0, 0, 0, 0, 0, // ����킵���̐��������Ă��� 58, 510, 44, 37, 30,
    0, 0, 0,       // ����킵���̐��������Ă��� 23, 16, 9
    0, 0, 0, 9000,    // Moids�̊��x���オ���Ă��� 7, 6, 5, 4, ��10��
    0, 0, 0, 0, 0, 0, // ���x���������Ă���
    600, //��
    
};
 */
volatile int* sequence_length = &sequence_length_table[0];

bool pulseMode = true;
bool showaMode = false;
bool moidsMode = false;

int showaProbability = 100;

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

    //2�b��7��������
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

    //interactive Mode 7, 6, 5, 4��5�b���� 1on/off��25msec
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
int currentSequence = randomPulse;

void timerTick()
{
    static unsigned int sec = 0;
    static unsigned int moidsSec = 0;
    sec++;

    if (moidsMode)
    {
	if (sec >= 4167*2)
	{
	    sec = 0;
	    moidsSec++;
	}
	if (moidsSec >= *sequence_length)
	{
	    moidsSec = 0;
	    sequence_length++;
	    currentSequence++;
	    setNextSequenceData();
	    return;
	}
	
	for (int i = 0; i < MOIDS_PER_UNIT; ++i)
	{
	    moids[i].tick();
	}
	return;
    }

    if (pulseMode
	|| showaMode)
    {
	if (sec >= *sequence_length)
	{   
	    sec = 0;
	    sequence_length++;
	    currentSequence++;
	    setNextSequenceData();
	}
    }
}

void setNextSequenceData()
{
    while (0 == *sequence_length)
    {
	sequence_length++;
	currentSequence++;
    }
    
    switch (currentSequence)
    {
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

    switch (currentSequence)
    {
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

    switch (currentSequence)
    {
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

    switch (currentSequence)
    {
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

    switch (currentSequence)
    {
    case moids7:
	chooseMoidsThreshold(1);
	break;
    case moids6:
	chooseMoidsThreshold(2);
	break;
    case moids5:
	chooseMoidsThreshold(2);
	break;
    case moids4:
	chooseMoidsThreshold(2);
	break;
    case moids_dead5:
	chooseMoidsThreshold(5);
	break;
    case moids_dead6:
	chooseMoidsThreshold(6);
	break;
    case moids_dead7:
	chooseMoidsThreshold(7);
	break;
    case moids_dead8:
	chooseMoidsThreshold(8);
	break;
    case moids_dead9:
	chooseMoidsThreshold(9);
	break;
    case moids_dead10:
	chooseMoidsThreshold(10);
	break;
    case moids_dead:
	chooseMoidsDead();
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

    for (int i = 0; i < MOIDS_PER_UNIT; ++i)
    {
        analogWrite(OUTPUT_LED_PINS[i], 0);
	digitalWrite(OUTPUT_RELAY_PINS[i], 0);
    }
}

void chooseMoidsThreshold(const int thres)
{
    moidsMode = true;
    pulseMode = false;
    showaMode = false;

    MsTimer2::stop();
    MsTimer2::set(1, timerTick);
    MsTimer2::start();
    
    for (int i = 0; i < MOIDS_PER_UNIT; ++i)
    {
	moids[i].setRelayOnTime(*moids_on);
	moids[i].setWaitAfterSoundDetect(*moids_wait);
        moids[i].setMicThreshold(thres);
    }
}

void chooseShowaDecay(int probability)
{
    showaProbability = probability;
}

void chooseDelayedShowa(int delayTime)
{
    showaMode = true;
    pulseMode = false;
    *showa_delay = delayTime;
}

void choosePulseSequence(int prob_to_zero, int max_seq_number)
{
    pulseMode = true;
    int randomValue = random(101);
    if (randomValue <= prob_to_zero)
    {
	pulse_high = &pulse_high_table[0];
	pulse_low = &pulse_low_table[0];
	return;
    }
    
    randomValue = random(max_seq_number+1);
    pulse_high = &pulse_high_table[0] + randomValue;
    pulse_low = &pulse_low_table[0] + randomValue;
}

void chooseShowaSequence(int prob_to_showa)
{
    int randomValue = random(101);
    if (randomValue <=  prob_to_showa)
    {
	showaMode = true;
	pulseMode = false;
	showa_delay = &showa_delay_table[0];
	return;	
    }
    showaMode = false;
    pulseMode = true;

    pulse_high = &pulse_high_table[0];
    pulse_low = &pulse_low_table[0];
}

void setup()
{
    for (int i = 0; i < 13; i++)
    {
	pinMode(i, OUTPUT);
    }
    
    // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    cbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;

    randomSeed(analogRead(0));
    
    for (int i = 0; i < MOIDS_PER_UNIT; ++i)
    {
        pinMode(OUTPUT_LED_PINS[i], OUTPUT);
	pinMode(OUTPUT_RELAY_PINS[i], OUTPUT);
    }

    for (int i = 0; i < MOIDS_PER_UNIT; i++)
    {
	moids[i].setInputMicPin(INPUT_MIC_PINS[i]);
	moids[i].setOutputLEDPin(OUTPUT_LED_PINS[i]);
	moids[i].setOutputRelayPin(OUTPUT_RELAY_PINS[i]);
	moids[i].init();

	for (int j = 0; j < MOIDS_PER_UNIT; j++)
	{
	    if (&moids[j] != &moids[i])
	    {
		moids[i].registerOtherMoids(&moids[j]);
	    }
	}
    }
    
    MsTimer2::set(4167*2, timerTick); // 1sec
    MsTimer2::start();

    setNextSequenceData();
}


void loop()
{
    if (pulseMode)
    {
	makePulse();
	return;
    }
    else if (showaMode)
    {
      	makeShowa();
	return;
    }
    else if (moidsMode)
    {
	for (int i = 0; i < MOIDS_PER_UNIT; ++i)
	{
	    moids[i].loop();
	}
	return;
    }
}

void makePulse()
{
    onAll();
    _delay_us(*pulse_high);
    offAll();
    _delay_us(*pulse_low);
}

void makeShowa()
{
    int showaOnOffRandom = random(101);
    if (showaOnOffRandom > showaProbability)
    {
	_delay_us(*showa_delay);
	return;
    }
    
    int randomValue = random(3);
    analogWrite(OUTPUT_LED_PINS[randomValue], LED_BRIGHTNESS_ON);
    digitalWrite(OUTPUT_RELAY_PINS[randomValue], HIGH);

    _delay_us(*showa_delay);
    analogWrite(OUTPUT_LED_PINS[randomValue], LED_BRIGHTNESS_OFF);
    digitalWrite(OUTPUT_RELAY_PINS[randomValue], LOW);
}

void onAll()
{
    for (int i = 0; i < MOIDS_PER_UNIT; ++i)
    {
        digitalWrite(OUTPUT_RELAY_PINS[i], HIGH);
	analogWrite(OUTPUT_LED_PINS[i], LED_BRIGHTNESS_ON);
    }
}

void offAll()
{
    for (int i = 0; i < MOIDS_PER_UNIT; ++i)
    {
        digitalWrite(OUTPUT_RELAY_PINS[i], LOW);
	analogWrite(OUTPUT_LED_PINS[i], LED_BRIGHTNESS_OFF);
    }
}
