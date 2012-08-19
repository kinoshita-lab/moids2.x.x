#pragma once

/** A Moid Class Definition
 */
class Moids
{
public:
	// ctor
	Moids();

	void init();

	void setInputMicPin(const int pin);
	void setOutputLEDPin(const int pin);
	void setOutputRelayPin(const int pin);
	void setMicThreshold(const int thres);
	void setRelayOnTime(const int time);
	void setWaitAfterSoundDetect(const int time);
	// timer callback
	void tick();

	// called from main loop
	void loop();

	void registerOtherMoids(Moids* moids);

	void broadCastGenerateSoundState(bool start);
	void receiveOtherMoidsMessageSoundState(bool start, int relayPin);

private:

	int m_micOffset;
	int m_micThreshold;

	unsigned long m_relayOnTime;
	unsigned long m_relayOnTimeCounter;

	unsigned long m_relayOffTime;
	unsigned long m_relayOffCounter;

	bool m_oscillation_high;
	unsigned long m_oscillationCount;
	unsigned long m_oscillatorCountMax;

	unsigned long m_waitAfterDetect;
	unsigned long m_waitAfterDetectCounter;

	unsigned long m_nopWait;
	unsigned long m_nopWaitCounter;

	// enum for interactive functions
	enum MoidsState
	{
		ReadAnalog = 0, SoundInput, GenerateSound, Nop, NumberOfStates,
	};

	int m_state;

	// state functions
	void changeState(const int state);
	void tickReadAnalogState();
	void tickSoundInputState();
	void tickGenerateSoundState();
	void tickNopState();

	// function pointer for state
	void (Moids::*m_stateFunction)();

	void determineSound();

	void oscillate();

	void toNop();

	volatile unsigned long m_timerCounter;

	// pins
	volatile int m_inputMicPin;
	volatile int m_outputLEDPin;
	volatile int m_outputRelayPin;

	// mic input workaround
	void readAnalogInput();
	bool checkInput();

	static const int MIC_INPUT_ARRAY_LENGTH = 2;
	volatile int m_micInput[2];
	bool m_firstTimeAfterStateTransition;
	volatile int m_dontReadCounter;
	void makeOffset();
	volatile bool m_detect1stTime;

	static const unsigned long COUNTER_PER_1MSEC = 8;
	static const int LED_BRIGHTNESS_WAITING = 1;
	static const int LED_BRIGHTNESS_INPUT_DETECTED = 2;
	static const int LED_BRIGHTNESS_SOUND_GENERATING = 4;
	static const int MOIDS_INPUT_TOO_BIG;

	static const int NUM_OTHER_MOIDS = 2;
	volatile int m_numOtherMoids;

	Moids* m_otherMoids[NUM_OTHER_MOIDS];

	static const int sound_table_on[];
	static const int sound_table_off[];
	static const int sound_durations[];
	static const int sound_table_length;



	bool m_needOscillation;
};

