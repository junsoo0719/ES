#include <stdio.h>
using namespace std;

unsigned char LEDs = 0x01;
int LED_function(int input)
{
	return LEDs << input;
}

int main(void)
{
	int i;
	scanf("%d", &i);
	int return_value = LED_function(i);
	printf("return_value: %d", return_value);
}