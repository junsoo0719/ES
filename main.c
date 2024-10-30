#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000UL

void main()
{
	unsigned char value = 128;
	DDRA = 0xff;
	for (;;)
	{
		PORTA = value;
		_delay_ms(200);
		value >>= 1;
		if (value == 0) {
			value = 128;
		}
	}
}
