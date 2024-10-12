#include <stdio.h>
using namespace std;

unsigned char LEDs = 0x01;
int LED_function1(int input)
{
	return LEDs << input;
}
int LED_function2(int input)
{
	return LEDs >> input;
}

int main(void)
{
	printf("%d ", LEDs);
	for (int i = 0; i < 4; i++)
	{
		while (LEDs < 0x80)
		{
			LEDs = LED_function1(1);
			printf("%d ", LEDs);
		}
		while (LEDs > 0x01)
		{
			LEDs = LED_function2(1);
			printf("%d ", LEDs);
		}
	}
}