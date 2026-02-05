#pragma once

enum InputMicPin
{
    MIC_0 = 1,
    MIC_1 = 0,
    MIC_2 = 2
};

enum OutputLedPin
{
    LED_0 = 6,
    LED_1 = 10,
    LED_2 = 9
};

enum OutputRelayPin
{
    RELAY_0 = 8,
    RELAY_1 = 7,
    RELAY_2 = 4
};

static const int INPUT_MIC_PINS[3]    = {MIC_0, MIC_1, MIC_2};
static const int OUTPUT_LED_PINS[3]   = {LED_0, LED_1, LED_2};
static const int OUTPUT_RELAY_PINS[3] = {RELAY_0, RELAY_1, RELAY_2};
