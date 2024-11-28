//Switch + Stop watch
#include <avr/io.h>  //ATmage128 레지스터 정의
#include <util/delay.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#define DO 17
#define RE 43
#define MI 66
#define FA 77
#define SOL 97
#define LA 114
#define TI 129
#define UDO 137

#define ON 1
#define OFF 0

const int scale[8] = { DO, RE, MI, FA, SOL, LA, TI, UDO };
int state = ON; //주파수 발생시키기 위해 소리를 끄고 킬 변수
int index = 0; // 몇번째 음을 발생시킬지 결정할 변수
int flag = 0; //스위치가 중복으로 눌리는 것을 보정하기 위한 변수

ISR(TIMER0_OVF_vect) {

	if (state == ON)
	{
		PORTB = 0x00;
		state = OFF;
	}
	else
	{
		PORTB = 0x10;
		state = ON;
	}
	TCNT0 = scale[index];
	flag = 1;
}


ISR(INT5_vect) {
	if (flag == 1)
	{
		index = (index + 1) % 8;
		flag = 0;
		_delay_ms(100);
	}
}

int main() {

	DDRB = 0x10;
	DDRE = 0xdf;
	TCCR0 = 0x03;
	TIMSK = 0x01;
	EICRB = 0x0c;
	EIMSK = 0x20;
	sei();
	while (1)
	{
	}
}
