#include "UsTimer2.h"

/* ========================================================================
  CONSTANTS
========================================================================= */
static const int NUM_MOIDS_PER_UNIT = 3;
static const int MOIDS_FIRE_THRESHOLD = 7;
static const int TIMER_COUNT_SOUND_INPUT = 500; //
static const int TIMER_COUNT_GENERATE_SOUND = 10; // for sound devices on/off
static const int LED_BRIGHTNESS_READING = 1;
static const int LED_BRIGHTNESS_SOUND_INPUT = 16;
static const int LED_BRIGHTNESS_SOUND_GENERATING = 255;

/* ========================================================================
  FUNCTION PROTOTYPES
========================================================================= */
void timerProc(); // for timer processing
void soundInputStateProcess(const int i); // state specific process(sound input and waiting)
void generateSoundStateProcess(const int i); // state specific process (sound generation fired)
void readAnalogInput(const int i);

/* ========================================================================
  PIN ASSIGNS
========================================================================= */
int analogInputPins[NUM_MOIDS_PER_UNIT] = {0, 1, 2};
int analogOutputLEDs[NUM_MOIDS_PER_UNIT] = {6, 9, 10 };
int soundDevices[NUM_MOIDS_PER_UNIT] = {4 , 7,  8};

/* ========================================================================
  STATE DEFINITION
======================================================================== */
enum MoidsState
{
 READING_ANALOG,
 SOUND_INPUT,
 GENERATE_SOUND,  
};
MoidsState moidsStates[NUM_MOIDS_PER_UNIT] = {READING_ANALOG}; 

/* ========================================================================
  VALUES AND FLAGS
======================================================================== */
// mic input values
int micValues[NUM_MOIDS_PER_UNIT] = {0};
int oldMicValues[NUM_MOIDS_PER_UNIT] = {0};
// timer counter
int timerCounters[NUM_MOIDS_PER_UNIT] = {0};
// avoid triggered by last sound-input
bool firstReadAfterTransition[NUM_MOIDS_PER_UNIT] = {true};

/* -----------------------------------------------------------------------
  setup
------------------------------------------------------------------------ */
void setup()
{ 
  for (int i = 0; i < NUM_MOIDS_PER_UNIT; i++)
  {
    oldMicValues[i] = analogRead(analogInputPins[i]);
    pinMode(analogOutputLEDs[i], OUTPUT);
    pinMode(soundDevices[i], OUTPUT);
  } 

  UsTimer2::set(1, timerProc);
  UsTimer2::start();
}
/* -----------------------------------------------------------------------
  loop
------------------------------------------------------------------------ */ 
void loop()
{
  for (int i = 0; i < NUM_MOIDS_PER_UNIT; i++)
  {
    switch (moidsStates[i])
    {
      case READING_ANALOG:
        readAnalogInput(i);
        break;
      case SOUND_INPUT:
        break;
      case GENERATE_SOUND:
        break;
      default:
        break;
    }
  }
}
/* -----------------------------------------------------------------------
  timerProc
------------------------------------------------------------------------ */
void timerProc()
{ 
  for (int i = 0; i < NUM_MOIDS_PER_UNIT; i++)
  {
    switch (moidsStates[i])
    {
    case SOUND_INPUT:
      soundInputStateProcess(i);
      break;
    case GENERATE_SOUND:
      generateSoundStateProcess(i);
      break;
    default:
      break;
    }
  }
}
/* -----------------------------------------------------------------------
  soundInputStateProcess
------------------------------------------------------------------------ */
void soundInputStateProcess(const int i)
{ 
   timerCounters[i]++;
    if (TIMER_COUNT_SOUND_INPUT <= timerCounters[i])
    {
      timerCounters[i] = 0;
      moidsStates[i] = GENERATE_SOUND;
      digitalWrite(soundDevices[i], HIGH);
      analogWrite(analogOutputLEDs[i], LED_BRIGHTNESS_SOUND_GENERATING);
    }
}
/* -----------------------------------------------------------------------
  generateSoundStateProcess
------------------------------------------------------------------------ */
void generateSoundStateProcess(const int i)
{
  timerCounters[i]++;
  if (TIMER_COUNT_GENERATE_SOUND <= timerCounters[i])
  {
    timerCounters[i] = 0;
    moidsStates[i] = READING_ANALOG;
    firstReadAfterTransition[i] = true;
    digitalWrite(soundDevices[i], LOW);
    analogWrite(analogOutputLEDs[i], LED_BRIGHTNESS_READING);
  }
}
/* -----------------------------------------------------------------------
  readAnalogInput
------------------------------------------------------------------------ */ 
void readAnalogInput(const int i)
{ 
  if (firstReadAfterTransition[i])
  { 
    micValues[i] = analogRead(analogInputPins[i]);
    firstReadAfterTransition[i] = false;
  }
  oldMicValues[i] = micValues[i];
  micValues[i] = analogRead(analogInputPins[i]);
  
  if (abs(micValues[i] - oldMicValues[i]) >= MOIDS_FIRE_THRESHOLD)
  {
    moidsStates[i] = SOUND_INPUT;
    analogWrite(analogOutputLEDs[i], LED_BRIGHTNESS_SOUND_INPUT);
  }
} 
