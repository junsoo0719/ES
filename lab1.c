#include "includes.h"

#define F_CPU	16000000UL	// CPU frequency = 16 Mhz
#include <avr/io.h>
#include <util/delay.h>

#define UCHAR unsigned char // UCHAR 정의
#define USHORT unsigned short // USHORT 정의
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
#define ATS75_ADDR 0x98 // 0b10011000, 7비트를 1비트 left shift
#define CDS_VALUE 871


#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE
#define  N_TASKS        5


OS_STK		TaskStk[N_TASKS][TASK_STK_SIZE];
OS_EVENT* Mbox;
OS_FLAG_GRP* EventFlag;

UCHAR send[4];
USHORT light_value;

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data);
void write_twi_0byte_nopreset(UCHAR reg);
void  TemperatureTask(void* data);
void  ADCTask(void* data);
void  FndTask(void* data);
void  FndDisplayTask(void* data);
void  LEDTask(void* data);

int main(void)
{
	OSInit();
	OS_ENTER_CRITICAL();
	TCCR0 = 0x07;
	TIMSK = _BV(TOIE0);
	TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
	OS_EXIT_CRITICAL();

	DDRA = 0xff;

	INT8U	err;
	EventFlag = OSFlagCreate(0x00, &err);
	Mbox = OSMboxCreate((void*)0);

	OSTaskCreate(TemperatureTask, (void*)0, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 0);
	OSTaskCreate(ADCTask, (void*)0, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 1);
	OSTaskCreate(FndTask, (void*)0, (void*)&TaskStk[2][TASK_STK_SIZE - 1], 2);
	OSTaskCreate(LEDTask, (void*)0, (void*)&TaskStk[3][TASK_STK_SIZE - 1], 3);
	OSTaskCreate(FndDisplayTask, (void*)0, (void*)&TaskStk[4][TASK_STK_SIZE - 1], 4);
	OSStart();

	return 0;
}

void InitI2C()
{
	PORTD = 3; 						// For Pull-up override value
	SFIOR &= ~(1 << PUD); 			// PUD
	TWSR = 0; 						// TWPS0 = 0, TWPS1 = 0
	TWBR = 32;						// for 100  K Hz bus clock
	TWCR = _BV(TWEA) | _BV(TWEN);	// TWEA = Ack pulse is generated
	// TWEN = TWI 동작을 가능하게 한다
}

int ReadTemperature(void)
{
	int value;

	TWCR = _BV(TWSTA) | _BV(TWINT) | _BV(TWEN);
	while (!(TWCR & _BV(TWINT)));

	TWDR = 0x98 + 1; //TEMP_I2C_ADDR + 1
	TWCR = _BV(TWINT) | _BV(TWEN);
	while (!(TWCR & _BV(TWINT)));

	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	while (!(TWCR & _BV(TWINT)));

	//온도센서는 16bit 기준으로 값을 가져오므로
	//8비트씩 2번을 받아야 한다.
	value = TWDR;
	TWCR = _BV(TWINT) | _BV(TWEN);
	while (!(TWCR & _BV(TWINT)));

	value = ((value << 8) | TWDR);
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);

	//value >>= 8;
	TIMSK = (value >= 33) ? TIMSK | _BV(TOIE2) : TIMSK & ~_BV(TOIE2);

	return value;
}

void InitADC() 
{
	ADMUX = 0x00;
	ADCSRA = 0x87;
}

USHORT ReadADC()
{
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= 0x40; // ADC start conversion, ADSC = '1'
	while ((ADCSRA & (0x10)) != 0x10); // ADC 변환 완료 검사
	adc_low = ADCL;
	adc_high = ADCH;
	value = (adc_high << 8) | adc_low;

	return value;
}

void TemperatureTask(void* data)
{
	data = data;
	USHORT value;
	InitI2C();

	write_twi_1byte_nopreset(ATS75_CONFIG_REG, 0x00);
	write_twi_0byte_nopreset(ATS75_TEMP_REG);
	while (1) {
		value = ReadTemperature();
		OSMboxPost(Mbox, (void*)&value);
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void ADCTask(void* data)
{
	INT8U	err;
	InitADC();

	while (1) {
		light_value = ReadADC();
		if (light_value < CDS_VALUE) {
			OSFlagPost(EventFlag, 0x02, OS_FLAG_SET, &err);
			OSFlagPost(EventFlag, 0x04, OS_FLAG_CLR, &err);
		}
		else {
			OSFlagPost(EventFlag, 0x04, OS_FLAG_SET, &err);
			OSFlagPost(EventFlag, 0x02, OS_FLAG_CLR, &err);
		}
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void FndTask(void* data)
{
	INT8U	err;
	data = data;
	int i;
	UCHAR value_int, value_deci, num[4];
	USHORT now_value;

	while (1) {
		now_value = *(USHORT*)OSMboxPend(Mbox, 0, &err);

		if ((now_value & 0x8000) != 0x8000)
			num[3] = 11;
		else
		{
			num[3] = 10;
			now_value = (~now_value) - 1;
		}

		value_int = (UCHAR)((now_value & 0x7f00) >> 8);
		value_deci = (UCHAR)(now_value & 0x00ff);

		num[2] = (value_int / 10) % 10;
		num[1] = value_int % 10;
		num[0] = ((value_deci & 0x80) == 0x80) * 5;

		for (i = 0; i < 4; i++)
			send[i] = num[i];
		OSFlagPost(EventFlag, 0x01, OS_FLAG_SET, &err);

		OSTimeDlyHMSM(0, 0, 2, 0);
	}
}

void LEDTask(void* data)
{
	INT8U	err;

	while (1) {
		OSFlagPend(EventFlag, 0x02, OS_FLAG_WAIT_SET_ANY, 0, &err);
		PORTA = 0xff;

		OSFlagPend(EventFlag, 0x04, OS_FLAG_WAIT_SET_ANY, 0, &err);
		PORTA = 0x00;

		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}


void FndDisplayTask(void* data)
{
	UCHAR FND_DATA[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00 };
	UCHAR fnd_sel[4] = { 0x01, 0x02, 0x04, 0x08 };
	INT8U	err;
	int i;

	DDRC = 0xff;
	DDRG = 0x0f;

	while (1)
	{
		OSFlagPend(EventFlag, 0x01, OS_FLAG_WAIT_SET_ANY, 0, &err);
		for (i = 0; i < 4; i++)
		{
			PORTC = FND_DATA[send[i]];
			PORTG = fnd_sel[i];
			if (i == 1) PORTC |= 0x80;
			OSTimeDlyHMSM(0, 0, 0, 1);
		}
	}
}


void write_twi_1byte_nopreset(UCHAR reg, UCHAR data)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10)); // ACK를 기다림
	TWDR = ATS75_ADDR | 0;  // SLA+W 준비, W=0
	TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg;    // aTS75 Reg 값 준비
	TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg 값 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWDR = data;    // DATA 준비
	TWCR = (1 << TWINT) | (1 << TWEN);  // DATA 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP 전송
}
void write_twi_0byte_nopreset(UCHAR reg)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10));  // ACK를 기다림
	TWDR = ATS75_ADDR | 0; // SLA+W 준비, W=0
	TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg;    // aTS75 Reg 값 준비
	TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg 값 전송
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP 전송
}
