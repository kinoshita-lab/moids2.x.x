#include <stdint.h>
#include "Moids.h"
#include "Arduino.h"
#include "compensation_after2025.h"

// 反応するときにデカすぎる場合をハネる。値が大きい方が反応しやすい。最小は0。
const int Moids::MOIDS_INPUT_TOO_BIG = 4;

// 状態が変わった時にちょっと待つ（誤動作防止用) 最小は0。最大でも10くらい。
const int Moids::DELAY_FOR_STATE_TRANSITION = 1;

// 音が入ってきたかどうか？のチェックをちょっと厳密にする。
const bool Moids::STRICT_CHECKING = false;

// ここから下は基本データ
const int Moids::sound_table_length = 5;
/**
 *  relay on time.
 *  number of cycles of 125usec timer tick
 */
const int Moids::sound_table_on[] = {
    125 / 125, 125 / 125, 250 / 125, 125 / 125, 375 / 125,
};

/**
 * relay off time.
 * number of cycles of 125usec timer tick
 */
const int Moids::sound_table_off[] = {
    125 / 125, 250 / 125, 250 / 125, 500 / 125, 375 / 125,
};

/**
 * sound drations.
 * number of cycles of 125usec timer tick
 */
const int Moids::sound_durations[] = {
    150, 150, 150, 150, 150,
};

Moids::Moids() {}

void Moids::init() {
  m_dontReadCounter = 0;
  m_numOtherMoids = 0;
  m_firstTimeAfterStateTransition = false;
  m_relayOnTimeCounter = 0;
  m_waitAfterDetectCounter = 0;
  m_timerCounter = 0;
  m_micOffset = 0;
  m_nopWait = (unsigned long)500;
  m_nopWaitCounter = 0;
  m_needOscillation = false;
  m_relayOffTime = 0;
  m_relayOffCounter = 0;
  m_oscillatorCountMax = 0;
  m_oscillation_high = false;

  for (int i = 0; i < NUM_OTHER_MOIDS; i++) {
    m_otherMoids[i] = NULL;
  }

  makeOffset();
  changeState(ReadAnalog);
}

void Moids::makeOffset(const int num_read) {
  float offset = 0.f;

  for (int i = 0; i < num_read; i++) {
    const float mag = 1.0f / (float)num_read;
    offset += mag * analogRead(m_inputMicPin);
  }

  m_micOffset = (int)offset;
}

void Moids::setMicThreshold(const int thres) { m_micThreshold = thres; }

void Moids::setRelayOnTime(const int time) { m_relayOnTime = time; }

void Moids::setWaitAfterSoundDetect(const int time) {
  m_waitAfterDetect = time;
}

void Moids::loop() {
  if (ReadAnalog == m_state) {
    readAnalogInput();
  }

  const uint32_t now = micros();
  const uint32_t delta = now - m_lastMicros;

  uint32_t count = delta / MOIDS_TIMER_TICK_MICROS;

  if (count) {
    for (uint32_t i = 0; i < count; ++i) {
      tick();
    }
    m_lastMicros = now;
  }
}

void Moids::readAnalogInput() {
  if (m_dontReadCounter) {
    return;
  }

  if (m_firstTimeAfterStateTransition) {
    m_firstTimeAfterStateTransition = false;
    makeOffset();

    int read = analogRead(m_inputMicPin) - m_micOffset;
    for (int i = 0; i < MIC_INPUT_ARRAY_LENGTH; i++) {
      m_micInput[i] = read;
    }
  }

  // read Input
  m_micInput[0] = analogRead(m_inputMicPin) - m_micOffset;

  // check threshold
  bool changed = checkInput();
  m_micInput[1] = m_micInput[0];

  if (changed) {
    changeState(SoundInput);
  }
}

bool Moids::checkInput() {

  return abs(m_micInput[0] - m_micInput[1]) > m_micThreshold &&
         abs(m_micInput[0] - m_micInput[1]) <
             m_micThreshold + MOIDS_INPUT_TOO_BIG;
}

// tick from MsTimer2, assuming tick cycle is 125 usec
void Moids::tick() {
  m_timerCounter++;
  (this->*m_stateFunction)();
}

void Moids::tickNopState() {
  m_nopWaitCounter++;

  if (m_nopWait > m_nopWaitCounter) {
    return;
  }

  m_firstTimeAfterStateTransition = true;
  m_nopWaitCounter = 0;
  analogWrite(m_outputLEDPin, LED_BRIGHTNESS_WAITING);
  changeState(ReadAnalog);
}

void Moids::tickReadAnalogState() {
  // nop
}

void Moids::tickSoundInputState() {
  m_waitAfterDetectCounter++;

  if (m_waitAfterDetect > m_waitAfterDetectCounter) {
    return;
  }

  m_waitAfterDetectCounter = 0;

  digitalWrite_with_delay_compensation(m_outputRelayPin, HIGH);
  analogWrite(m_outputLEDPin, LED_BRIGHTNESS_SOUND_GENERATING);
  changeState(GenerateSound);
}

void Moids::tickGenerateSoundState() {
  m_relayOnTimeCounter++;

  if (m_relayOnTime > m_relayOnTimeCounter) {
    return;
  }

  toNop();
}

void Moids::toNop() {
  digitalWrite_with_delay_compensation(m_outputRelayPin, LOW);
  m_relayOnTimeCounter = 0;
  analogWrite(m_outputLEDPin, 0);
  changeState(Nop);
}

void Moids::changeState(const int state) {
  cli();
  m_state = state;

  switch (m_state) {
  case ReadAnalog:
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
  sei();

  if (DELAY_FOR_STATE_TRANSITION) {
    delay(DELAY_FOR_STATE_TRANSITION);
  }
}

void Moids::setInputMicPin(const int pin) {
  m_inputMicPin = pin;
  pinMode(pin, INPUT);
}

void Moids::setOutputLEDPin(const int pin) {
  m_outputLEDPin = pin;
  pinMode(pin, OUTPUT);
}

void Moids::setOutputRelayPin(const int pin) {
  m_outputRelayPin = pin;
  pinMode(pin, OUTPUT);
}

void Moids::registerOtherMoids(Moids *moids) {
  m_otherMoids[m_numOtherMoids] = moids;
  m_numOtherMoids++;
}

void Moids::broadCastGenerateSoundState(bool start) {
  for (int i = 0; i < m_numOtherMoids; i++) {
    m_otherMoids[i]->receiveOtherMoidsMessageSoundState(start,
                                                        m_outputRelayPin);
  }
}

void Moids::receiveOtherMoidsMessageSoundState(bool start, int relayPin) {
  if (start) {
    m_dontReadCounter++;
  } else {
    m_dontReadCounter--;
  }
}

void Moids::determineSound() {
  m_timerCounter = 0;

  // randomize sound
  const int soundIndex = random(sound_table_length);

  m_relayOnTime = sound_table_on[soundIndex];
  m_relayOffTime = sound_table_off[soundIndex];

  return;
}
