#include <stdio.h>
using namespace std;

int main(void)
{
	unsigned char LEDs;
	unsigned char return_value = 0x00;
	scanf("%uc", &LEDs);
	int cnt = 0;
	for (int i = 0; i < 8; i++)
	{
		if (LEDs & (1 << i))
		{
			cnt++;
			return_value = (return_value >> 1);
			return_value |= 0x80;
		}
	}
	printf("Number of ones: %d\n", cnt);
	printf("Output in binary: %X\n", return_value);
	printf("Output in decimal: %d\n", return_value);
	printf("Output in hexadecimal: %#X\n", return_value);
}