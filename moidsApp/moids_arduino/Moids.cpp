#include "Arduino.h"
#include "Moids.h"

const int Moids::sound_table_length = 43;

const int Moids::sound_table_on[] =
{
    125/125 , 125/125 , 250/125 , 125/125 , 250/125,
    125/125 , 250/125 , 250/125 , 375/125 , 500/125,
    625/125 , 500/125 , 250/125 , 375/125 , 250/125,
    375/125 , 500/125 , 250/125 , 375/125 , 500/125,
    625/125 , 750/125 , 500/125 , 625/125 , 750/125,
    875/125 , 500/125 , 875/125 , 1000/125, 500/125,
    625/125 , 625/125 , 875/125 , 1000/125, 875/125,
    1000/125, 1250/125, 875/125 , 1000/125, 1125/125,
    1250/125, 1325/125, 1450/125,
};

const int Moids::sound_table_off[] =
{
    125/125 , 250/125 , 250/125 , 375/125 , 375/125 ,
    500/125 , 875/125 , 1125/125, 1250/125, 1250/125,
    1375/125, 500/125 , 625/125 , 625/125 , 750/125 ,
    750/125 , 750/125 , 875/125 , 875/125 , 875/125 ,
    875/125 , 875/125 , 1000/125, 1000/125, 1000/125,
    1000/125, 1125/125, 1125/125, 1125/125, 1250/125,
    1250/125, 1325/125, 1325/125, 1325/125, 1450/125,
    1325/125, 1325/125, 1250/125, 1250/125, 1250/125,
    1325/125, 1450/125, 1450/125,
};

const int Moids::sounding_time_total = 80;

Moids::Moids()
{
}

void Moids::init()
{
	m_dontReadCounter = 0;
	m_numOtherMoids = 0;
	m_firstTimeAfterStateTransition = false;
	m_relayOnTimeCounter = 0;
	m_waitAfterDetectCounter = 0;
	m_timerCounter = 0;
	m_micOffset = 0;
	m_nopWait = (unsigned long) 500;
	m_nopWaitCounter = 0;
	m_needOscillation = false;
	m_relayOffTime = 0;
	m_relayOffCounter = 0;
	m_oscillatorCountMax = 0;
	m_oscillation_high = false;

	for (int i = 0; i < NUM_OTHER_MOIDS; i++)
	{
		m_otherMoids[i] = NULL;
	}

	makeOffset();
	changeState(ReadAnalog);
}

void Moids::makeOffset()
{
	float offset = 0.f;

	for (int i = 0; i < 100; i++)
	{
		offset += 0.01 * analogRead(m_inputMicPin);
	}

	m_micOffset = (int) offset;
}

void Moids::setMicThreshold(const int thres)
{
    m_micThreshold = thres;
}

void Moids::setRelayOnTime(const int time)
{
    m_relayOnTime = time;
}

void Moids::setWaitAfterSoundDetect(const int time)
{
    m_waitAfterDetect = time;
}

void Moids::loop()
{
	if (ReadAnalog == m_state)
	{
		readAnalogInput();
	}
}

void Moids::readAnalogInput()
{
	if (m_dontReadCounter)
	{
		return;
	}

	if (m_firstTimeAfterStateTransition)
	{
		m_firstTimeAfterStateTransition = false;
		makeOffset();

		int read = analogRead(m_inputMicPin) - m_micOffset;
		for (int i = 0; i < MIC_INPUT_ARRAY_LENGTH; i++)
		{
			m_micInput[i] = read;
		}
	}

	// read Input
	m_micInput[0] = analogRead(m_inputMicPin) - m_micOffset;

	// check threshold
	bool changed = checkInput();
	m_micInput[1] = m_micInput[0];

	if (changed)
	{
		changeState(SoundInput);
	}
}

bool Moids::checkInput()
{
	return abs(m_micInput[0] - m_micInput[1]) > m_micThreshold
	    && abs(m_micInput[0] - m_micInput[1]) < m_micThreshold + 4;
}


// tick from MsTimer2, assuming tick cycle is 125 usec 
void Moids::tick()
{
    m_timerCounter++;	
    (this->*m_stateFunction)();
}

void Moids::tickNopState()
{
    m_nopWaitCounter++;

    if (m_nopWait > m_nopWaitCounter)
    {
		return;
    }
	
    m_firstTimeAfterStateTransition = true;
    m_nopWaitCounter = 0;
    analogWrite(m_outputLEDPin, LED_BRIGHTNESS_WAITING);
    changeState(ReadAnalog);
}

void Moids::tickReadAnalogState()
{
    // nop
}

void Moids::tickSoundInputState()
{
    m_waitAfterDetectCounter++;

    if (m_waitAfterDetect > m_waitAfterDetectCounter)
    {
		return;
    }

    m_waitAfterDetectCounter = 0;

    digitalWrite(m_outputRelayPin, HIGH);
    analogWrite(m_outputLEDPin, LED_BRIGHTNESS_SOUND_GENERATING);
    changeState(GenerateSound);
}

void Moids::tickGenerateSoundState()
{
	if (m_needOscillation)
	{
		oscillate();
		return;
	}

	m_relayOnTimeCounter++;

	if (m_relayOnTime > m_relayOnTimeCounter)
	{
		return;
	}

	toNop();
}

void Moids::oscillate()
{
	m_timerCounter++;

	if (m_oscillation_high)
	{
		if (m_timerCounter > m_relayOnTime)
		{
			m_timerCounter = 0;
			digitalWrite(m_outputRelayPin, LOW);
			m_oscillation_high = false;
		}

		return;
	}

	if (m_timerCounter > m_relayOffTime)
	{
		m_oscillationCount++;

		if (m_oscillationCount > m_oscillatorCountMax)
		{
			m_timerCounter = 0;
			toNop();
			return;
		}

		m_timerCounter = 0;
		digitalWrite(m_outputRelayPin, HIGH);
		m_oscillation_high = true;
	}
}

void Moids::toNop()
{
	digitalWrite(m_outputRelayPin, LOW);
	m_relayOnTimeCounter = 0;
	analogWrite(m_outputLEDPin, 0);
	changeState(Nop);
}

void Moids::changeState(const int state)
{
	m_state = state;

	switch (m_state)
	{
	case ReadAnalog:
		m_detect1stTime = false;
		m_firstTimeAfterStateTransition = true;
		m_stateFunction = &Moids::tickReadAnalogState;
		analogWrite(m_outputLEDPin, LED_BRIGHTNESS_WAITING);
		break;
	case SoundInput:
		broadCastGenerateSoundState(true);
		determineSound();
		m_stateFunction = &Moids::tickSoundInputState;
		analogWrite(m_outputLEDPin, LED_BRIGHTNESS_INPUT_DETECTED);
		break;
	case GenerateSound:
		m_stateFunction = &Moids::tickGenerateSoundState;
		break;
	case Nop:
		broadCastGenerateSoundState(false);
		m_stateFunction = &Moids::tickNopState;
		break;
	default:
		break;
	}
}

void Moids::setInputMicPin(const int pin)
{
    m_inputMicPin = pin;
    pinMode(pin, INPUT);
}

void Moids::setOutputLEDPin(const int pin)
{
    m_outputLEDPin = pin;
    pinMode(pin, OUTPUT);
}
	
void Moids::setOutputRelayPin(const int pin)
{
    m_outputRelayPin = pin;
    pinMode(pin, OUTPUT);
}

void Moids::registerOtherMoids(Moids* moids)
{
    m_otherMoids[m_numOtherMoids] = moids;
    m_numOtherMoids++;
}

void Moids::broadCastGenerateSoundState(bool start)
{
	for (int i = 0; i < m_numOtherMoids; i++)
	{
		m_otherMoids[i]->receiveOtherMoidsMessageSoundState(start, m_outputRelayPin);
	}
}

void Moids::receiveOtherMoidsMessageSoundState(bool start, int relayPin)
{
	if (start)
	{
		m_dontReadCounter++;
	}
	else
	{
		m_dontReadCounter--;
	}
}

void Moids::determineSound()
{
	m_timerCounter = 0;

	// randomize sound
	const int currentRelayOnTimeIndex = random(sound_table_length);
	m_needOscillation = (currentRelayOnTimeIndex < 10);

	m_relayOnTime = sound_table_on[currentRelayOnTimeIndex];
	m_relayOffTime = sound_table_off[currentRelayOnTimeIndex];

	if (m_needOscillation)
	{
		m_timerCounter = 0;
		m_oscillation_high = true;
		digitalWrite(m_outputRelayPin, HIGH);
		m_oscillatorCountMax =  sounding_time_total / (m_relayOnTime + m_relayOffTime);
		m_oscillationCount = 0;
	}
}
