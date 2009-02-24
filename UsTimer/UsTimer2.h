#ifndef USTIMER_H
#define USTIMER_H

#include <avr/interrupt.h>

namespace USTimer {
	extern unsigned long usecs;
	extern void (*func)();
	extern volatile unsigned long count;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;

	// syntax suger for interfacing
	void set(unsigned long us, void (*f)());

	// original MsTimer2's set function
	void originalSet(unsigned long us, void (*f)());
	void start();
	void stop();
	void _overflow();
}

#endif
