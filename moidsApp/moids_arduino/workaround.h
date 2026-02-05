#pragma once
#ifndef WORKAROUND_H
#define WORKAROUND_H

#define TIME_FACTOR 8 // modified timer0 prescaler factor
// 補正版delay関数マクロ
#define delay_comp(ms) delay((unsigned long)(ms) * TIME_FACTOR)

#define EXTRA_DELAY_US (145)
#endif // WORKAROUND_H
