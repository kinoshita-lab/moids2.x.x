#pragma once
#ifndef COMPENSATION_AFTER2025_H
#define COMPENSATION_AFTER2025_H

#include <Arduino.h>

constexpr int extra_delay_us = 142; // Adjusted delay to compensate for timing changes for new version of Arduino (faster digitalWrite)

inline void digitalWrite_with_delay_compensation(int pin, int value)
{
    digitalWrite(pin, value);
    delayMicroseconds(extra_delay_us);
}


#endif // COMPENSATION_AFTER2025_H
