#include <Arduino.h>
#include <stdint.h>
#include "DmxSimple.h"
#include "TimerOne.h"

struct BrightnessInfoTableItem
{
	uint8_t initialBrightness; // 最初の明るさ
	uint8_t lastBrightness; // 最後の明るさ
	uint32_t duration; // その間を移動する時間(msec)
};

// #define DEBUG

// このへんでいろんな設定ができる
static const uint32_t MOIDS_ON_TIME_LENGTH = 15UL * 60UL; // 電源On時間の長さを secで！
static const uint32_t POWER_OFF_TIME_LENGTH = 15UL; /// Offになっている時間の長さ sec。
static const uint32_t MINIMUM_ON_OFF_ASSERT_DELAY_MSEC = 3000UL; // 確実に全部がOffしてResetできるのに必要な時間(最低これ以上必要)
static const uint32_t NUMBER_OF_ON_OFF_TIMES = 2UL;

const int NUMBER_OF_BRIGHTNESS_TABLE_ITEM = 10; // 下の表の数
BrightnessInfoTableItem brightnessTable[NUMBER_OF_BRIGHTNESS_TABLE_ITEM] =
{
	{ 32, 64, 2UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 64, 160, 1UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 160, 140, 1UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 140, 64, 60UL * 14UL * 1000UL }, // 定常状態 14分
	{ 64, 32, 50UL * 1000UL }, // 終わるときだんだん暗くなる

	{ 32, 64, 2UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 64, 1, 1UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 1, 128, 1UL * 1000UL }, // Offの時にだんだん明るくなる
	{ 128, 140, 60UL * 14UL * 1000UL }, // 定常状態 14分
	{ 140, 64, 50UL * 1000UL }, // 終わるときだんだん暗くなる
};

// このへんは基本的なPINの設定
static const int DMX_OUT_PIN = 8;
static const int DMX_CHANNEL = 1;
static const int POWER_CIRCUIT_MASTER_PIN = 5;
static const int NUMBER_OF_POWER_CIRCUITS = 4;
static const int LED_PIN = 13;
static const uint8_t POWER_CIRCUIT_PINS[NUMBER_OF_POWER_CIRCUITS] = { 1, 2, 3, 4 };

// その他
static const int DMX_MAX = 0xFF;

struct SystemState
{
	volatile uint32_t msecCounter;
	volatile bool needInitialize;

	volatile uint8_t brightNess;
	volatile bool updateBrightness;

	volatile uint8_t currentBrightnessTableItemIndex;
	volatile uint32_t brightnessDurationCounter;

	volatile uint32_t totalSecCounter;
	volatile bool secTicked;

	volatile bool brightnessChangefinished;
	volatile bool moidsPowerFinished;

	volatile bool brightnessEndMessageShown;
	volatile bool moidsPowerEndMessageShown;

	volatile uint32_t moidsOffTimeMillisec;
	volatile uint32_t moidsOffDurationCounter;
	volatile bool needToTurnOnMoids;
	volatile bool alreadyTurnOnMoids;

	volatile uint32_t onOffTimes;
	volatile uint32_t onOffSecCounter;

	SystemState() :
			msecCounter(0), needInitialize(false), brightNess(0), updateBrightness(
					false), currentBrightnessTableItemIndex(0), brightnessDurationCounter(
					0), totalSecCounter(0), secTicked(false), brightnessChangefinished(
					false), moidsPowerFinished(false), brightnessEndMessageShown(
					false), moidsPowerEndMessageShown(false), moidsOffTimeMillisec(
					0), moidsOffDurationCounter(0), needToTurnOnMoids(false), alreadyTurnOnMoids(
					false), onOffTimes(0), onOffSecCounter(0)
	{

	}

	void initialize()
	{
		msecCounter = 0;
		needInitialize = false;
		updateBrightness = true;
		currentBrightnessTableItemIndex = 0;
		brightnessDurationCounter = 0;
		totalSecCounter = 0;
		secTicked = false;
		brightNess =
				getHumanBrightNess(
						brightnessTable[currentBrightnessTableItemIndex].initialBrightness);
		brightnessChangefinished = false;
		moidsPowerFinished = false;
		brightnessEndMessageShown = false;
		moidsPowerEndMessageShown = false;
		needToTurnOnMoids = false;
		alreadyTurnOnMoids = false;
		moidsOffDurationCounter = 0;
		onOffTimes = 0;
		onOffSecCounter = 0;

		uint32_t delayTime = (POWER_OFF_TIME_LENGTH * 1000UL);
		if (delayTime < MINIMUM_ON_OFF_ASSERT_DELAY_MSEC) {
			delayTime = MINIMUM_ON_OFF_ASSERT_DELAY_MSEC;
		}
#ifdef DEBUG
		Serial.print("init: moids delayTime =");
		Serial.println(delayTime);
#endif
		moidsOffTimeMillisec = delayTime;
		moidsOffDurationCounter = 0;
	}

	void nextBrightness()
	{
		if (brightnessChangefinished) {
			return;
		}

		currentBrightnessTableItemIndex++;

		if (currentBrightnessTableItemIndex >= NUMBER_OF_BRIGHTNESS_TABLE_ITEM) {
			brightnessChangefinished = true;
			currentBrightnessTableItemIndex--;
			return;
		}

		if (false == brightnessChangefinished) {
			brightnessDurationCounter = 0;
		}
	}
};
SystemState systemState;

void setup()
{
#ifdef DEBUG
	Serial.begin(38400);
#endif

	pinMode(POWER_CIRCUIT_MASTER_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);

	for (int i = 0; i < NUMBER_OF_POWER_CIRCUITS; ++i) {
		pinMode(POWER_CIRCUIT_PINS[i], OUTPUT);
	}

	DmxSimple.usePin(DMX_OUT_PIN);
	DmxSimple.maxChannel(DMX_CHANNEL);

	initializeWholeSystem();

	Timer1.initialize(1000); // 1msec
	Timer1.attachInterrupt(timerInterrupt);
}

void loop()
{
	if (systemState.needInitialize) {
		initializeWholeSystem();
		systemState.needInitialize = false;
	}

	if (systemState.needToTurnOnMoids && !systemState.alreadyTurnOnMoids) {
		systemState.alreadyTurnOnMoids = true;
		moidsPowerOnOff(true);
	}

	if (systemState.brightnessChangefinished
			&& systemState.moidsPowerFinished) {
		systemState.needInitialize = true;
	}

	if (systemState.updateBrightness) {
		DmxSimple.write(DMX_CHANNEL, systemState.brightNess);
		systemState.updateBrightness = false;
#if 0
//		Serial.print("[b] ");
//		Serial.println(systemState.brightNess);
#endif
	}

	if (systemState.secTicked) {
		systemState.secTicked = false;
#if DEBUG
		Serial.println(systemState.totalSecCounter);
#endif
	}

	if (systemState.brightnessChangefinished) {
		if (false == systemState.brightnessEndMessageShown) {
			systemState.brightnessEndMessageShown = true;
#if DEBUG
			Serial.println("[st]brightness end");
#endif
		}
	}

	if (systemState.moidsPowerFinished) {
		if (false == systemState.moidsPowerEndMessageShown) {
			systemState.moidsPowerEndMessageShown = true;
#if DEBUG
			Serial.println("[st]moids power end");
#endif
		}
	}
}

void initializeWholeSystem()
{
#if DEBUG
	Serial.println("[st]init whole");
#endif
	systemState.initialize();
	const uint8_t initialBrightness = getHumanBrightNess(
			brightnessTable[0].initialBrightness);
	DmxSimple.write(DMX_CHANNEL, initialBrightness);

	moidsPowerOnOff(false);
}

void moidsPowerOnOff(bool onOff)
{
	if (systemState.moidsPowerFinished && onOff) {
		return;
	}
	const int onOffValue = onOff ? HIGH : LOW;

	for (int i = 0; i < NUMBER_OF_POWER_CIRCUITS; ++i) {
		digitalWrite(POWER_CIRCUIT_PINS[i], onOffValue);
	}

	digitalWrite(POWER_CIRCUIT_MASTER_PIN, onOff);
	digitalWrite(LED_PIN, onOff);

#if DEBUG
	Serial.print("[st]Moids: ");
	Serial.println(onOff);
#endif

}

void timerInterrupt()
{
	systemState.msecCounter++;
	systemState.brightnessDurationCounter++;
	systemState.moidsOffDurationCounter++;

	if (systemState.msecCounter >= 1000) {
		systemState.totalSecCounter++;
		systemState.onOffSecCounter++;
		systemState.secTicked = true;
		systemState.msecCounter = 0;

		if (systemState.onOffSecCounter > MOIDS_ON_TIME_LENGTH) {
			systemState.onOffSecCounter = 0;
			systemState.onOffTimes++;

			if (false == systemState.moidsPowerFinished) {
				if (NUMBER_OF_ON_OFF_TIMES <= systemState.onOffTimes) {
					systemState.moidsPowerFinished = true;
					systemState.moidsOffDurationCounter = 0;
					systemState.alreadyTurnOnMoids = false;
					systemState.needToTurnOnMoids = false;
					systemState.moidsPowerFinished = true;
					moidsPowerOnOff(false);
					return;
				} else {
					systemState.moidsOffDurationCounter = 0;
					systemState.alreadyTurnOnMoids = false;
					systemState.needToTurnOnMoids = false;
					moidsPowerOnOff(false);
					return;
				}
			}
		}
	}

	if (systemState.moidsOffDurationCounter
			>= systemState.moidsOffTimeMillisec) {
		if (false == systemState.alreadyTurnOnMoids) {
			systemState.needToTurnOnMoids = true;
		}
	}

	// 100msecごとに明るさを変える
	if (systemState.msecCounter % 100 == 0) {
		if (false == systemState.brightnessChangefinished) {
			const uint8_t brightness =
					getHumanBrightNess(
							interpolate(systemState.brightnessDurationCounter,
									brightnessTable[systemState.currentBrightnessTableItemIndex].duration,
									brightnessTable[systemState.currentBrightnessTableItemIndex].initialBrightness,
									brightnessTable[systemState.currentBrightnessTableItemIndex].lastBrightness));

			if (systemState.brightNess != brightness) {
				systemState.brightNess = brightness;
				systemState.updateBrightness = true;
			}
		}
	}

	if (systemState.brightnessDurationCounter
			> brightnessTable[systemState.currentBrightnessTableItemIndex].duration) {
		systemState.nextBrightness();
	}
}

uint8_t getHumanBrightNess(const int& brightness)
{
	if (brightness == 0) {
		return 0;
	}

	float tmp = 0.5
			+ ((float) brightness * (float) brightness) / ((float) DMX_MAX);
	if (tmp < 1.0f) {
		tmp = 1.f;
	}

	return (uint8_t)(tmp);
}

//線形補間の公式 y = y0 + x(  (y1 - y0) / x1 )
uint8_t interpolate(uint32_t currentTime, uint32_t totalDuration,
		uint8_t startBrightness, uint8_t finalBrightness)
{
	float y0 = startBrightness;
	float y1 = finalBrightness;
	float x = currentTime;
	float x1 = totalDuration;

	float tmp = y0 + x * ((y1 - y0) / x1);

	return (uint8_t) tmp;
}
