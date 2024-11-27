//Switch + Stop watch
#include <avr/io.h>  //ATmage128 레지스터 정의
#include <util/delay.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define START 1
#define STOP 0

const unsigned char digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f };
const unsigned char fnd_sel[4] = { 0x08,0x04,0x02,0x01 };
const unsigned char dot = 0x80;

int signal;
int count = 0;
unsigned char num[4]; //FND에 숫자 출력용 배열

ISR(INT4_vect) { // INT4_vect에 해당하는 button 누르면 stopwatch 시작, 다시 누르면 stopwatch 멈춤, signal 활용

	// your code
	if (signal == START)
	{
		signal = STOP;
	}
	else
	{
		signal = START;
	}
	_delay_ms(10);
}


ISR(INT5_vect) { // 만약 stopwatch가 멈춰져 있고 INT5_vect button이 눌린다면 시간 초기화, signal 활용

	// your code
	if (signal == STOP)
	{
		count = 0;
	}
	_delay_ms(10);
}




void display_fnd(int c) {

	// FND 화면에 출력을 위한 코드
	int i;
	num[0] = digit[(c / 1000) % 10];
	num[1] = digit[(c / 100) % 10] + dot;
	num[2] = digit[(c / 10) % 10];
	num[3] = digit[c % 10];
	for (int i = 0; i < 4; i++) {
		//1. PORTC 와 PORTG 사용
		//2. delay_ms 또는 delay_us 를 사용하여 1/100 초 구현
		PORTC = num[i];
		PORTG = fnd_sel[i];
		_delay_ms(2);
	}

}



int main() {
	DDRA = 0xff;
	// DDRE : switch 1 and 2 set
	// set EICRB
	// set EIMSK
	// set SREG
	DDRC = 0xff;
	DDRG = 0x0f;
	DDRE = 0xcf;
	EICRB = 0x0a;
	EIMSK = 0x30;
	sei();

	while (1) {
		display_fnd(count);
		if (signal == START) {
			count = (count + 1) % 10000;
		}
	}
}
