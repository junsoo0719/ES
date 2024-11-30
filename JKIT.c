#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CDS_VALUE 871
#define ON 1
#define OFF 0

int state = OFF; //소리를 끄고 킬 변수
void init_adc();
unsigned short read_adc();
void show_adc(unsigned short value);
void show_fnd(unsigned short value);
void show_buzzer(unsigned short value);

char fnd_value[4] = {};
const unsigned char digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f };
const unsigned char fnd_sel[4] = { 0x08,0x04,0x02,0x01 };

ISR(TIMER0_OVF_vect) {
	if (state == ON) {
		PORTB ^= 0x10; // 버저 ON/OFF 반복
	}
	else {
		PORTB = 0x00; // 버저 OFF 유지
	}
	TCNT0 = 0; // 타이머 초기화
}

int main() {
	unsigned short value;
	DDRA = 0xff;
	DDRB = 0x10;
	DDRC = 0xff;
	DDRG = 0x0f;
	TCCR0 = 0x03;
	TIMSK = 0x01;
	sei();
	TCNT0 = 0;

	init_adc();
	while (1) {
		value = read_adc();
		show_adc(value);
		show_fnd(value);
		show_buzzer(value);
	}
}

void init_adc() {
	ADMUX = 0x00;
	// 00000000
	// REFS(1:0) = "00" AREF(+5V) 기준전압 사용
	// ADLAR = '0' 디폴트 오른쪽 정렬
	// MUX(4:0) = "00000" ADC0 사용, 단극 입력
	ADCSRA = 0x87;
	// 10000111
	// ADEN = '1' ADC를 Enable
	// ADFR = '0' single conversion 모드
	// ADPS(2:0) = "111" 프리스케일러 128분주
}

unsigned short read_adc() {
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= 0x40; // ADC start conversion, ADSC = '1'
	while ((ADCSRA & (0x10)) != 0x10); // ADC 변환 완료 검사
	adc_low = ADCL;
	adc_high = ADCH;
	value = (adc_high << 8) | adc_low;

	return value;
}

void show_adc(unsigned short value) {
	if (value < CDS_VALUE) {
		//LED 켜기
		PORTA = 0xff;
	}
	else
	{
		//LED 끄기
		PORTA = 0x00;
	}
}

void show_fnd(unsigned short value)
{
	int i;
	if (value < CDS_VALUE) {
		//FND에 ADC 값 출력하기
		fnd_value[3] = (value / 1000) % 10;
		fnd_value[2] = (value / 100) % 10;
		fnd_value[1] = (value / 10) % 10;
		fnd_value[0] = value % 10;
		for (i = 0; i < 4; i++)
		{
			PORTC = digit[fnd_value[i]];
			PORTG = fnd_sel[i];
			_delay_ms(2);
		}
	}
	else
	{
		//FND에 '0000' 출력하기
		for (i = 0; i < 4; i++)
		{
			fnd_value[i] = 0;
			PORTC = digit[fnd_value[i]];
			PORTG = fnd_sel[i];
			_delay_ms(2);
		}
	}
}

void show_buzzer(unsigned short value)
{
	if (value < CDS_VALUE) {
		//버저 켜기
		state = ON;
	}
	else
	{
		//버저 끄기
		state = OFF;
		PORTB = 0x00;
	}
}