#ifndef MsTimer2_h
#define MsTimer2_h

#include <avr/interrupt.h>

namespace Timer2_125usec {
	extern unsigned long msecs;
	extern void (*func)();
	extern volatile unsigned long count;
	extern volatile char overflowing;
	extern volatile unsigned int tcnt2;
	
	void set(unsigned long ms, void (*f)());
	void start();
	void stop();
	void _overflow();
}

#endif
