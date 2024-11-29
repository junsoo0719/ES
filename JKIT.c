//Switch + Stop watch
#include <avr/io.h>  //ATmage128 �������� ����
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

const int melody[25] = { MI,RE,DO,RE,MI,MI,MI,RE,RE,RE,MI,SOL,SOL,MI,RE,DO,RE,MI,MI,MI,RE,RE,MI,RE,DO };
const int interval[25] = { 3,1,2,2,2,2,4,2,2,4,2,2,4,3,1,2,2,2,2,4,2,2,2,2,8 };
int state = ON; //���ļ� �߻���Ű�� ���� �Ҹ��� ���� ų ����
int index = 0; // ���° ���� �߻���ų�� ������ ����
int flag = 0; //����ġ�� �ߺ����� ������ ���� �����ϱ� ���� ����

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
	TCNT0 = melody[index];
}

void play(int delay) {
	TCNT0 = melody[index];
	while (delay > 0)
	{
		_delay_ms(2500);
		delay--;
	}
}

int main() {
	int i;
	DDRB = 0x10;
	TCCR0 = 0x03;
	TIMSK = 0x01;
	sei();
	while (1)
	{
		for (index = 0; index < 25; index++) {
			play(interval[index]);
			_delay_ms(50);
		}
	}
}
