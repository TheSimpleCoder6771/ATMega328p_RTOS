/*
 * led.c
 *
 */

#include <avr/io.h>
#include "led.h"

void vLEDInit(void)
{
	DDRC |= _BV(PINB5);
}

void vLEDToggle(void)
{
	PORTC ^= _BV(PINB5);
}

void vLEDOff(void)
{
	PORTC &= ~_BV(PINB5);
}
