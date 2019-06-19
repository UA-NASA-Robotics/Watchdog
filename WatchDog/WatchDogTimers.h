/*
	WatchDogTimers.h
	Timer Include File
	By: Scott Grisso
*/

#include <avr/io.h>				/* Standard I/O Library*/
#include <stdbool.h>			/* Standard Bool library*/
#include <avr/interrupt.h>		/* Standard Timer/Interrupt Library*/

#define WAIT_TIME 1000

int checkTimers(int time)
{
	if(time >= 65000)
	{
		TCNT1 = 0;
		timer1 = TCNT1 + WAIT_TIME;
		timer2 = TCNT1 + WAIT_TIME;
		timer3 = TCNT1 + WAIT_TIME;
		time = TCNT1 + rebootTime;
	}
	return time;
}

int resetTimers(int constantTime)
{
	int returnTime;
	returnTime = TCNT1 + constantTime;
	if(returnTime >= 65000)
	{
		TCNT1 = 0;
	}
	timer1 = TCNT1 + constantTime;
	timer2 = TCNT1 + constantTime;
	timer3 = TCNT1 + constantTime;
	returnTime = TCNT1 + constantTime;
	return returnTime;
}

void initTimers(void)
{
	/* Timer clock = I/O clock / 1024 */
	TCCR0B = (1<<CS02)|(1<<CS00);
	TCCR1B = (1<<CS12)|(1<<CS10);
	/* Clear overflow flag */
	TIFR0  = 1<<TOV0;
	TIFR1 = 1<<TOV0;
	/* Enable Overflow Interrupt */
	TIMSK1 = 1<< OCIE1A;

	/*setup timer 1 OCA for 1 sec given 1024 division */
	OCR1A = 15625;
	/*setup timer 1 OCB for 1 sec given 1024 division */
	OCR1B = 15625;
	
	/*reset TCNT of both timers */
	TCNT0 = 0;
	TCNT1 = 0;
}
//timer1 COMPA
ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
}
