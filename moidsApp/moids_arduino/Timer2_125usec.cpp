/*
  Timer2_125usec.h - Using timer2 with 1ms resolution
  Javier Valencia <javiervalencia80@gmail.com>
  
  History:
  	11/Jun/08 - VM0.3
  		changes to allow working with different CPU frequencies
  		added support for ATMega128 (using timer2)
  		compatible with ATMega48/88/168/8
	10/May/08 - V0.2 added some security tests and volatile keywords
	9/May/08 - V0.1 released working on ATMEGA168 only
	

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Timer2_125usec.h"

unsigned long Timer2_125usec::msecs;
void (*Timer2_125usec::func)();
volatile unsigned long Timer2_125usec::count;
volatile char Timer2_125usec::overflowing;
volatile unsigned int Timer2_125usec::tcnt2;

void Timer2_125usec::set(unsigned long ms, void (*f)()) {
	float prescaler = 0.0;

	TIMSK2 &= ~(1 << TOIE2);
	TCCR2A &= ~((1 << WGM21) | (1 << WGM20));
	TCCR2B &= ~(1 << WGM22);
	ASSR &= ~(1 << AS2);
	TIMSK2 &= ~(1 << OCIE2A);

	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL)) {  // prescaler set to 64
		TCCR2B |= (1 << CS22);
		TCCR2B &= ~((1 << CS21) | (1 << CS20));
		prescaler = 64.f * 8;             // 125uSec resolution // original was 64.f;
	} else if (F_CPU < 1000000UL) {  // prescaler set to 8
		TCCR2B |= (1 << CS21);
		TCCR2B &= ~((1 << CS22) | (1 << CS20));
		prescaler = 8.f;
	} else {              // F_CPU > 16Mhz, prescaler set to 128
		TCCR2B |= ((1 << CS22) | (1 << CS20));
		TCCR2B &= ~(1 << CS21);
		prescaler = 128.f;
	}

	tcnt2 = 256 - (int)((float)F_CPU * 0.001 / prescaler);

	if (ms == 0)
		msecs = 1;
	else
		msecs = ms;

	func = f;
}

void Timer2_125usec::start() {
	count = 0;
	overflowing = 0;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__)
	TCNT2 = tcnt2;
	TIMSK2 |= (1 << TOIE2);
#elif defined(__AVR_ATmega128__)
	TCNT2 = tcnt2;
	TIMSK |= (1 << TOIE2);
#elif defined(__AVR_ATmega8__)
	TCNT2 = tcnt2;
	TIMSK |= (1 << TOIE2);
#endif
}

void Timer2_125usec::stop() {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__)
	TIMSK2 &= ~(1 << TOIE2);
#elif defined(__AVR_ATmega128__)
	TIMSK &= ~(1 << TOIE2);
#elif defined(__AVR_ATmega8__)
	TIMSK &= ~(1 << TOIE2);
#endif
}

void Timer2_125usec::_overflow() {
	count += 1;

	if (count >= msecs && !overflowing) {
		overflowing = 1;
		count = 0;
		(*func)();
		overflowing = 0;
	}
}

ISR(TIMER2_OVF_vect) {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__)
	TCNT2 = Timer2_125usec::tcnt2;
#elif defined(__AVR_ATmega128__)
	TCNT2 = Timer2_125usec::tcnt2;
#elif defined(__AVR_ATmega8__)
	TCNT2 = Timer2_125usec::tcnt2;
#endif
	Timer2_125usec::_overflow();
}
