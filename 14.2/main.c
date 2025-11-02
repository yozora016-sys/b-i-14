#include "stm32f10x.h"

void Delay_ms(uint32_t ms);

int main(void)
{

    while (1)
    {
				//__WFI();
        Delay_ms(1000);
    }
}
void Delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 8000; i++)
    {
        __NOP(); // CPU v?n ch?y liên t?c ? tiêu th? cao
    }
}


