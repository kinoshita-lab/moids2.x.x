/*
  UsTimer2.h - Using timer2 with ???us resolution
  Based On MsTimer2
  Improve resolution to 125 usec (8 times higher than original)

  This library is free software; you can redistribute it and/org
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

#include <UsTimer2.h>

unsigned long USTimer::usecs;
void (*USTimer::func)();
volatile unsigned long USTimer::count;
volatile char USTimer::overflowing;
volatile unsigned int USTimer::tcnt2;


void USTimer::set(unsigned long us, void (*f)())
{
	originalSet(us / 125, f);
}

void USTimer::originalSet(unsigned long us, void (*f)())
{

// for mega168 only!
	float prescaler = 0.0;
	
	TIMSK2 &= ~(1<<TOIE2);
	TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
	TCCR2B &= ~(1<<WGM22);
	ASSR &= ~(1<<AS2);
	TIMSK2 &= ~(1<<OCIE2A);
	
	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL))
	{// prescaler set to 8
		TCCR2B |= (1<<CS22);
		TCCR2B &= ~((1<<CS21) | (1<<CS20));
		prescaler = 8.0;
	}
	else if (F_CPU < 1000000UL)
	{	// prescaler set to 1
		TCCR2B |= (1<<CS21);
		TCCR2B &= ~((1<<CS22) | (1<<CS20));
		prescaler = 8.0;
	}
	else
	{ // F_CPU > 16Mhz, prescaler set to 128
		TCCR2B |= ((1<<CS22) | (1<<CS20));
		TCCR2B &= ~(1<<CS21);
		prescaler = 16.0;
	}
	
	tcnt2 = 256 - (int)((float)F_CPU * 0.001 / prescaler);

	usecs = us == 0 ? 1 : us;
	func = f;
}

void USTimer::start()
{
	count = 0;
	overflowing = 0;
	TCNT2 = tcnt2;
	TIMSK2 |= (1<<TOIE2);
}

void USTimer::stop()
{
	TIMSK2 &= ~(1<<TOIE2);
}

void USTimer::_overflow()
{
	count += 1;
	
	if (count >= usecs && !overflowing)
	{
		overflowing = 1;
		count = 0;
		(*func)();
		overflowing = 0;
	}
}

ISR(TIMER2_OVF_vect)
{
	TCNT2 = USTimer::tcnt2;
	USTimer::_overflow();
}

