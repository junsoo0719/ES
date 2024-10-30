#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000UL

void main()
{
	unsigned char value = 32;
	DDRA = 0xff;
	for (;;)
	{
		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(5000);
		value >>= 1;
		PORTA = value;
		_delay_ms(5000);
		value >>= 1;
		PORTA = value;
		_delay_ms(15000);
		value <<= 1;
		PORTA = value;
		_delay_ms(15000);
		value >>= 1;
		PORTA = value;
		_delay_ms(5000);
		value >>= 2;
		PORTA = value;
		_delay_ms(10000);
		value <<= 2;

		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(5000);
		value >>= 1;
		PORTA = value;
		_delay_ms(5000);
		value >>= 1;
		PORTA = value;
		_delay_ms(15000);
		value <<= 1;
		PORTA = value;
		_delay_ms(10000);
		value >>= 1;
		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(5000);
		value <<= 1;
		PORTA = value;
		_delay_ms(15000);
		value >>= 2;
	}
}
